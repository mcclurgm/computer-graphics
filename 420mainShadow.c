/* Michael McClurg, Vermilion Villarreal */

/* On macOS, compile with...
    clang 300openGL20b.c -lglfw -framework OpenGL -Wno-deprecated
	On Linux:
	clang 360mainTexturing.c /usr/local/gl3w/src/gl3w.o -lGL -lglfw -lm -ldl
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "320shading.c"
#include "360texture.c"
#include "310vector.c"
#include "320matrix.c"
#include "320isometry.c"
#include "320camera.c"
#include "310mesh.c"
#include "410meshgl.c"
#include "370body.c"
#include "140landscape.c"
#include "400shadow.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

shaShading sha;

GLdouble angle = 0.0;
GLuint buffers[2];
GLuint vao;
GLuint vao1;
/* These are new. */
isoIsometry modeling;
camCamera cam;
texTexture texture0;
texTexture texture1;
texTexture textureWater;

double pSpot[3] = {0.0, 0.0, 1.0};
double spotPhi = M_PI / 6.0;
double spotTheta = -M_PI / 4.0;
GLdouble angleSpot = M_PI / 3;

shadowMap map;
camCamera camSpot;
shaShading shaShadow;

/* Camera setup, so we can be interactive */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 6.0, cameraTheta = -M_PI / 4.0;
double screenWidth = 768, screenHeight = 512;

bodyBody capsuleBody;
bodyBody landscapeBody;
meshglMesh glCapsule;
meshglMesh glLandscape;

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFPSPOT 4
#define UNIFCSPOT 5
#define UNIFDSPOT 6
#define UNIFTHSPOT 7
#define UNIFCAMBIENT 8
#define UNIFPCAMERA 9
#define UNIFTEXTURE 10
#define UNIFVIEWINGS 11
#define UNIFTEXSHADOW 12
#define UNIFSHADOWRGB 2
#define ATTRPOSITION 0
#define ATTRST 1
#define ATTRNORMAL 2

double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    /* The documentation for camSetFrustum says that we must re-call it here. */
	camSetFrustum(&cam, M_PI / 6.0, 10.0, 10.0, width, height);
	screenWidth = width;
	screenHeight = height;
}

int initializeMesh(void) {
	meshMesh capsule;
	if (meshInitializeCapsule(&capsule, .5, 2, 20, 20) != 0)
		return 1;
	meshglInitialize(&glCapsule, &capsule);
	meshDestroy(&capsule);

	// Add attribute arrays VAO
	glBindBuffer(GL_ARRAY_BUFFER, glCapsule.buffers[0]);
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		glCapsule.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		glCapsule.attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
	glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
		glCapsule.attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));

    /* Setup shadow VAO */
    meshglContinueInitialization(&glCapsule);
	glEnableVertexAttribArray(shaShadow.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(shaShadow.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		glCapsule.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));

	meshglFinishInitialization(&glCapsule);

	/* Setup landscape */
	int landNum = 50;
	double landData[landNum][landNum];
	double landMin, landMean, landMax;
	time_t t;
	int i;
	srand((unsigned)time(&t));
	landFlat(landNum, landNum, (double *)landData, 0.0);
	for (i = 0; i < 32; i += 1)
		landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
	for (i = 0; i < 4; i += 1)
		landBlur(landNum, landNum, (double *)landData);
	landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean, 
		&landMax);
	meshMesh landscape;
	GLdouble zs[3][4] = {
	{5.0, 4.5, 3.5, 3.0}, 
	{3.0, 2.5, 1.5, 0.5}, 
	{2.0, 1.5, -0.5, -1.0}};
	// if (meshInitializeBox(&landscape, -3.0, 3.0, -3.0, 3.0, -3.0, 0.0) != 0)
	if (meshInitializeLandscape(&landscape, landNum, landNum, 1.0, (GLdouble *)landData) != 0)
		return 2;
	
	//Setting up glLandscape VAO and mesh
	meshglInitialize(&glLandscape, &landscape);
	meshDestroy(&landscape);

	glBindBuffer(GL_ARRAY_BUFFER, glLandscape.buffers[0]);
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		glLandscape.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		glLandscape.attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
	glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
		glLandscape.attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));

    /* Setup shadow VAO */
    meshglContinueInitialization(&glLandscape);
	glEnableVertexAttribArray(shaShadow.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(shaShadow.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		glCapsule.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));

	meshglFinishInitialization(&glLandscape);

	return 0;
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	/* The two matrices will be sent to the shaders as uniforms. */
	GLchar vertexCode[] = ""
		"#version 140\n"
		"uniform mat4 viewing;"
		"uniform mat4 modeling;"
        "uniform mat4 viewingS;"
		"in vec3 position;"
		"in vec2 texCoords;"
		"in vec3 normal;"
		"out vec4 rgba;"
		"out vec2 st;"
		"out vec3 nop;"
		"out vec3 pFragment;"
        "out vec4 pFragmentS;"
		"void main() {"
		"   vec4 world = modeling * vec4(position, 1.0);"
		"   pFragment = vec3(world);"
        "   "
        "   mat4 scaleBias = mat4("
        "       0.5, 0.0, 0.0, 0.0,"
        "       0.0, 0.5, 0.0, 0.0,"
        "       0.0, 0.0, 0.5, 0.0,"
        "       0.5, 0.5, 0.5, 1.0);"
        "   vec4 pWorld = modeling * vec4(position, 1.0);"
		"   gl_Position = viewing * pWorld;"
        "   pFragmentS = scaleBias * viewingS * pWorld;"
        "   "
		"   st = texCoords;"
		"   nop = vec3(modeling * vec4(normal, 0));"
		"}";
	// Diffuse: need dLight, dNormal, cLight, cDiff
	// dNormal is the same as position (but we have to normalize it)
	// Ambient: need cAmbient
	// Specular: cSpec, pCam,
	GLchar fragmentCode[] = ""
		"#version 140\n"
	    "uniform vec3 dLight;" // Must be normalized (unit)
	    "uniform vec3 cLight;"
	    "uniform vec3 pSpot;" // Must be normalized (unit)
	    "uniform vec3 cSpot;"
		"uniform vec3 dSpot;"
		"uniform float thSpot;"
	    "uniform vec3 cAmbient;"
		"uniform vec3 pCam;"
		"uniform sampler2D texture0;"
        "uniform sampler2DShadow texture1;"
 	    "in vec3 nop;"
		"in vec2 st;"
		"in vec3 pFragment;"
        "in vec4 pFragmentS;"
		"out vec4 fragColor;"
		"void main() {"
		"	vec4 rgba = vec4(vec3(texture(texture0, st)), 1.0);"
		"	"
        "   float shadow = textureProj(texture1, pFragmentS);"
		/* 	First (directional) light source */
		"   vec3 dNormal = normalize(nop);" // diffuse
		"   float iDiff = dot(dLight, dNormal);"
		"   vec4 cLight = vec4(cLight, 1.0);"
		"	"
		"	vec3 dRefl = (2.0 * iDiff * dNormal) - dLight;"
		"	vec3 dCam = normalize(pCam - pFragment);" // calculate dCam
		"	float iSpec = dot(dRefl, dCam);"
		"	if (iSpec < 0.0)"
		"		iSpec = 0.0;"
		"	"
		"   if (iDiff < 0.0) {"
	    "       iDiff = 0.0;"
		"		iSpec = 0.0;"
		"	}"
        "	"
		"   vec4 diffuse = iDiff * rgba * cLight;"
		"	"
		"	vec4 cSpec = vec4(0.5, 0.5, 0.5, 1.0);"
		"	iSpec = pow(iSpec, 100.0);"
		"	vec4 specular = iSpec * cSpec * cLight;"
		"	"
		"   vec4 ambient = rgba * vec4(cAmbient, 1.0);"
		"	"
		"	fragColor = diffuse + ambient + specular;"
		"	"
		/* 	Light 2 (spotlight) */
		"	vec3 dSpotLight = normalize(pSpot - pFragment);"
		"	if(dot(dSpotLight, dSpot) >= thSpot) {"
		"   	iDiff = dot(dSpotLight, dNormal);"
		"   	vec4 cSpot = vec4(cSpot, 1.0);"
		"		"
		"		dRefl = (2.0 * iDiff * dNormal) - dSpotLight;"
		"		iSpec = dot(dRefl, dCam);"
		"		if (iSpec < 0.0)"
		"			iSpec = 0.0;"
		"		"
		"   	if (iDiff < 0.0) {"
	    "       	iDiff = 0.0;"
		"			iSpec = 0.0;"
		"		}"
        "	    "
        /*      Shadow mapping */
        "       iDiff *= shadow;"
        "       iSpec *= shadow;"
        "       "
		"   	diffuse = iDiff * rgba * cSpot;"
		"		"
		"		iSpec = pow(iSpec, 100.0);"
		"		specular = iSpec * cSpec * cSpot;"
		"		"
		"		fragColor = fragColor + diffuse + specular;"
		"	}"
		"}";

    const int unifNum = 13;
	const GLchar *uniformNames[unifNum] = {"viewing", "modeling", "dLight", "cLight", "pSpot", "cSpot", "dSpot", "thSpot", "cAmbient", "pCam", "texture0", "viewingS", "texture1"};
	const GLchar **unifNames = uniformNames;
	const int attrNum = 3;
	const GLchar *attributeNames[attrNum] = {"position", "texCoords", "normal"};
	const GLchar **attrNames = attributeNames;

	return(shaInitialize(&sha, vertexCode, fragmentCode, unifNum, unifNames, attrNum, attrNames));
}

int initializeShadowShadingProgram(void) {
	/* The two matrices will be sent to the shaders as uniforms. */
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		void main() {\
			vec4 world = modeling * vec4(position, 1.0);\
			gl_Position = viewing * world;\
		}";
	// Diffuse: need dLight, dNormal, cLight, cDiff
	// dNormal is the same as position (but we have to normalize it)
	// Ambient: need cAmbient
	// Specular: cSpec, pCam,
	GLchar fragmentCode[] = ""
		"#version 140\n"
        "uniform vec3 rgb;"
        "out vec4 rgba;"
		"void main() {"
        "   rgba = vec4(rgb, 1.0);"
		"}";

    const int unifNum = 3;
	const GLchar *uniformNames[unifNum] = {"viewing", "modeling", "rgb"};
	const GLchar **unifNames = uniformNames;
	const int attrNum = 1;
	const GLchar *attributeNames[attrNum] = {"position"};
	const GLchar **attrNames = attributeNames;

	return(shaInitialize(&shaShadow, vertexCode, fragmentCode, unifNum, unifNames, attrNum, attrNames));
}

/* We want to pass 4x4 matrices into uniforms in OpenGL shaders, but there are
two obstacles. First, our matrix library uses double matrices, but OpenGL
shaders expect GLfloat matrices. Second, C matrices are implicitly stored one-
row-after-another, while OpenGL shaders expect matrices to be stored one-column-
after-another. This function plows through both of those obstacles. */
void uniformMatrix44(GLdouble m[4][4], GLint uniformLocation) {
	GLfloat mTFloat[4][4];
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			mTFloat[i][j] = m[j][i];
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}

/* Here is a similar function for vectors. */
void uniformVector3(GLdouble v[3], GLint uniformLocation) {
	GLfloat vFloat[3];
	for (int i = 0; i < 3; i += 1)
		vFloat[i] = v[i];
	glUniform3fv(uniformLocation, 1, vFloat);
}

void renderRegularly(double oldTime, double newTime) {
	/* 
	Step 1: setup data in bodies for this time
	Update animated properties

	Step 2: deploy data
	send isometry
	send auxiliaries
	send textures (texRender)
	render mesh (meshglRender)
	texUnrender

	^ this should be the same for each body, and we loop over the bodies to do this
	*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);

	/* Calculate capsule modeling isometry. */
	GLdouble trans[3] = {-1.0, -1.0, -1.0};
	isoSetTranslation(&modeling, trans);
	angle += 0.2 * (newTime - oldTime);
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	GLdouble model[4][4];
	bodySetIsometry(&capsuleBody, modeling);

	/* Send our own viewing transformation P C^-1 to the shaders. */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);

	/* Create lighting uniforms, send to shaders */
	/* Create dLight uniform */
	GLdouble dLight[3] = {-1.0, -1.0, 1.0};
	vecUnit(3, dLight, dLight);
	uniformVector3(dLight, sha.unifLocs[UNIFDLIGHT]);
	/* Create cLight uniform */
	GLdouble cLight[3] = {0.5, 0.5, 0.5};
	uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);
	/* Create cAmbient uniform */
	GLdouble cAmbient[3] = {0.1, 0.1, 0.1};
	uniformVector3(cAmbient, sha.unifLocs[UNIFCAMBIENT]);
	/* Create pSpot uniform */
	uniformVector3(pSpot, sha.unifLocs[UNIFPSPOT]);
	/* Create cSpot uniform */
	GLdouble cSpot[3] = {1.0, 1.0, 1.0};
	uniformVector3(cSpot, sha.unifLocs[UNIFCSPOT]);
	/* Create dSpot uniform */
	GLdouble dSpot[3];
    vec3Spherical(1.0, spotPhi, spotTheta, dSpot);
	uniformVector3(dSpot, sha.unifLocs[UNIFDSPOT]);
	/* Create thSpot uniform */
	glUniform1f(sha.unifLocs[UNIFTHSPOT], cos(angleSpot / 2));

	/* Create pCam uniform */
	uniformVector3(cam.isometry.translation, sha.unifLocs[UNIFPCAMERA]);

    /* Create viewingS uniform: P C-1 for the spotlight camera camSpot */
    GLdouble viewingS[4][4];
	camGetProjectionInverseIsometry(&camSpot, viewingS);
	uniformMatrix44(viewingS, sha.unifLocs[UNIFVIEWINGS]);

	/* Render capsule */
	isoGetHomogeneous(&(capsuleBody.isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(capsuleBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(capsuleBody.mesh, 0);
	/* Clean up texture */
	texUnrender(capsuleBody.tex[0], GL_TEXTURE0);

	/* Render landscape */
	isoGetHomogeneous(&(landscapeBody.isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(landscapeBody.tex[0], GL_TEXTURE1, 1, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(landscapeBody.mesh, 0);
	/* Clean up texture */
	texUnrender(landscapeBody.tex[0], GL_TEXTURE1);
}

void renderShadowly(double oldTime, double newTime) {

	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaShadow.program);

	/* Calculate capsule modeling isometry. */
	GLdouble trans[3] = {-1.0, -1.0, -1.0};
	isoSetTranslation(&modeling, trans);
	angle += 0.2 * (newTime - oldTime);
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	GLdouble model[4][4];
	bodySetIsometry(&capsuleBody, modeling);


	/* Send our own viewing transformation P C^-1 to the shaders. */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&camSpot, viewing);
	uniformMatrix44(viewing, shaShadow.unifLocs[UNIFVIEWING]);

    /* Send shadow map color uniform */
    GLdouble shadowRGB[3] = {1.0, 1.0, 0.0};
    uniformVector3(shadowRGB, shaShadow.unifLocs[UNIFSHADOWRGB]);

	/* Render capsule */
	isoGetHomogeneous(&(capsuleBody.isometry), model);
	uniformMatrix44(model, shaShadow.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(capsuleBody.tex[0], GL_TEXTURE0, 0, shaShadow.unifLocs[UNIFTEXTURE]);
	meshglRender(capsuleBody.mesh, 1);
	/* Clean up texture */
	texUnrender(capsuleBody.tex[0], GL_TEXTURE0);

	/* Render landscape */
	isoGetHomogeneous(&(landscapeBody.isometry), model);
	uniformMatrix44(model, shaShadow.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(landscapeBody.tex[0], GL_TEXTURE1, 1, shaShadow.unifLocs[UNIFTEXTURE]);
	meshglRender(landscapeBody.mesh, 1);
	/* Clean up texture */
	texUnrender(landscapeBody.tex[0], GL_TEXTURE1);
}

void render(double oldTime, double newTime) {
    shadowRenderFirst(&map);
    renderShadowly(oldTime, newTime);
    shadowUnrenderFirst(&map);
    glUseProgram(sha.program);
    shadowRenderSecond(&map, GL_TEXTURE2, 2, sha.unifLocs[UNIFTEXSHADOW]);
    renderRegularly(oldTime, newTime);
    shadowUnrenderSecond(GL_TEXTURE2);
}

int initializeBodies(void) {
    if (bodyInitialize(&capsuleBody, 0, 1) != 0)
        return 8;
    bodySetMesh(&capsuleBody, &glCapsule);
    bodySetTexture(&capsuleBody, 0, &texture0);

	if (bodyInitialize(&landscapeBody, 0, 1) != 0)
		return 10;
	/* Setup landscape isometry, which is constant */
	isoIsometry landscapeIsom;
	GLdouble rot[3][3];
	GLdouble axis[3] = {0.0, 0.0, 1.0};
	mat33AngleAxisRotation(0, axis, rot);
	isoSetRotation(&landscapeIsom, rot);
	GLdouble trans[3] = {-15.0, -30.0, -10.5};
	isoSetTranslation(&landscapeIsom, trans);
	bodySetIsometry(&landscapeBody, landscapeIsom);
	bodySetMesh(&landscapeBody, &glLandscape);
	bodySetTexture(&landscapeBody, 0, &textureWater);

	return 0;
}

int initializeShadows(void) {
    shadowInitialize(&map, 512, 512);

    vecUnit(3, pSpot, pSpot);
    camSetProjectionType(&camSpot, 1);
    camLookAt(&camSpot, pSpot, 0.0, spotPhi, spotTheta);
    camSetFrustum(&camSpot, angleSpot, 10.0, 10.0, 512, 512);
	// camSetFrustum(&cam, M_PI / 6.0, 10.0, 10.0, screenWidth, screenHeight);
    return 0;
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_A)
		cameraTheta -= M_PI / 100;
	else if (key == GLFW_KEY_D)
		cameraTheta += M_PI / 100;
	else if (key == GLFW_KEY_W)
		cameraPhi -= M_PI / 100;
	else if (key == GLFW_KEY_S)
		cameraPhi += M_PI / 100;
	else if (key == GLFW_KEY_Q)
		cameraRho *= 0.9;
	else if (key == GLFW_KEY_E)
		cameraRho *= 1.1;
	else if (key == GLFW_KEY_K)
		cameraTarget[0] -= 0.5;
	else if (key == GLFW_KEY_SEMICOLON)
		cameraTarget[0] += 0.5;
	else if (key == GLFW_KEY_L)
		cameraTarget[1] -= 0.5;
	else if (key == GLFW_KEY_O)
		cameraTarget[1] += 0.5;
	else if (key == GLFW_KEY_I)
		cameraTarget[2] -= 0.5;
	else if (key == GLFW_KEY_P)
		cameraTarget[2] += 0.5;
	camSetFrustum(&cam, M_PI / 6.0, 10.0, 10.0, screenWidth, screenHeight);
	camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKey(GLFWwindow *window, int key, int scancode, int action,
        int mods) {
    int shiftIsDown, controlIsDown, altOptionIsDown, superCommandIsDown;
    shiftIsDown = mods & GLFW_MOD_SHIFT;
    controlIsDown = mods & GLFW_MOD_CONTROL;
    altOptionIsDown = mods & GLFW_MOD_ALT;
    superCommandIsDown = mods & GLFW_MOD_SUPER;
    if (action == GLFW_PRESS)
        handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown,
            superCommandIsDown);
    else if (action == GLFW_RELEASE)
        handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown,
            superCommandIsDown);
    else if (action == GLFW_REPEAT)
        handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown,
            superCommandIsDown);
}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0) {
    	fprintf(stderr, "main: glfwInit failed.\n");
        return 1;
    }

    /* Ask GLFW to supply an OpenGL 3.2 context. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window;
    window = glfwCreateWindow(screenWidth, screenHeight, "Learning OpenGL 2.0", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);

	if (gl3wInit() != 0) {
    	fprintf(stderr, "main: gl3wInit failed.\n");
    	glfwDestroyWindow(window);
    	glfwTerminate();
    	return 3;
    }

	/* Setup keybindings */
	glfwSetKeyCallback(window, handleKey);

    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);

	if (texInitializeFile(&texture0, "bliss.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 4;
	if (texInitializeFile(&texture1, "grass.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 5;
	if (texInitializeFile(&textureWater, "water.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 6;
    if (initializeShaderProgram() != 0)
    	return 7;
    if (initializeShadowShadingProgram() != 0)
        return 8;
    if (initializeMesh() != 0)
		return 9;
	if (initializeBodies() != 0)
		return 10;
    if (initializeShadows() != 0)
        return 11;
    
    while (glfwWindowShouldClose(window) == 0) {
        oldTime = newTime;
    	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    shaDestroy(&sha);
	texDestroy(&texture0);
	texDestroy(&texture1);
	texDestroy(&textureWater);
    bodyDestroy(&capsuleBody);
    bodyDestroy(&landscapeBody);
    shadowDestroy(&map);
	glDeleteBuffers(2, buffers);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
