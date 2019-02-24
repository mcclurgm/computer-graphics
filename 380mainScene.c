/* Michael McClurg, Vermilion Villarreal */

/* On macOS, compile with...
    clang 300openGL20b.c -lglfw -framework OpenGL -Wno-deprecated
	On Linux:
	clang 360mainTexturing.c /usr/local/gl3w/src/gl3w.o -lGL -lglfw -lm -ldl
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include "350meshgl.c"
#include "370body.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

shaShading sha;

GLdouble angle = 0.0;
GLdouble transSphere[3] = {1.0, 1.0, 0.0};
GLuint buffers[2];
GLuint vao;
GLuint vao1;
/* These are new. */
isoIsometry modeling;
camCamera cam;
texTexture texture0;
texTexture texture1;
texTexture textureWater;

bodyBody capsuleBody;
bodyBody landscapeBody;
bodyBody sphereBody;
meshglMesh glCapsule;
meshglMesh glLandscape;
meshglMesh glSphere;

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFCAMBIENT 4
#define UNIFPCAMERA 5
#define UNIFTEXTURE 6
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
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, width, height);
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

	meshglFinishInitialization(&glCapsule);

	/* Setup landscape */
	meshMesh landscape;
	GLdouble zs[3][4] = {
	{5.0, 4.5, 3.5, 3.0}, 
	{3.0, 2.5, 1.5, 0.5}, 
	{2.0, 1.5, -0.5, -1.0}};
	// if (meshInitializeLandscape(&landscape, 3, 4, 2.0, (GLdouble *)zs) != 0)
	if (meshInitializeBox(&landscape, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5) != 0)
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

	meshglFinishInitialization(&glLandscape);

	
	/* Setup sphere */
	meshMesh sphere;
	if (meshInitializeSphere(&sphere, 1.0, 20, 20) != 0)
		return 3;
	
	//Setting up glSphere VAO and mesh
	meshglInitialize(&glSphere, &sphere);
	meshDestroy(&sphere);

	glBindBuffer(GL_ARRAY_BUFFER, glSphere.buffers[0]);
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		glSphere.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		glSphere.attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
	glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
		glSphere.attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));

	meshglFinishInitialization(&glSphere);

	return 0;
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	/* The two matrices will be sent to the shaders as uniforms. */
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec2 texCoords;\
		in vec3 normal;\
		out vec4 rgba;\
		out vec2 st;\
		out vec3 nop;\
		out vec3 pFragment;\
		void main() {\
			vec4 world = modeling * vec4(position, 1.0);\
			pFragment = vec3(world);\
			gl_Position = viewing * world;\
			st = texCoords;\
			nop = vec3(modeling * vec4(normal, 0));\
		}";
	// Diffuse: need dLight, dNormal, cLight, cDiff
	// dNormal is the same as position (but we have to normalize it)
	// Ambient: need cAmbient
	// Specular: cSpec, pCam,
	GLchar fragmentCode[] = ""
		"#version 140\n"
	    "uniform vec3 dLight;" // Must be normalized (unit)
	    "uniform vec3 cLight;"
	    "uniform vec3 cAmbient;"
		"uniform vec3 pCam;"
		"uniform sampler2D texture0;"
 	    "in vec3 nop;"
		"in vec2 st;"
		"in vec3 pFragment;"
		"out vec4 fragColor;"
		"void main() {"
		"	vec4 rgba = vec4(vec3(texture(texture0, st)), 1.0);"
		""
		"   vec3 dNormal = normalize(nop);" // diffuse
		"   float iDiff = dot(dLight, dNormal);"
		"   vec4 cLight = vec4(cLight, 1.0);"
		""
		"	vec3 dRefl = (2.0 * iDiff * dNormal) - dLight;"
		"	vec3 dCam = normalize(pCam - pFragment);" // calculate dCam
		"	float iSpec = dot(dRefl, dCam);"
		"	if (iSpec < 0.0)"
		"		iSpec = 0.0;"
		""
		"   if (iDiff < 0.0) {"
	    "       iDiff = 0.0;"
		"		iSpec = 0.0;"
		"	}"
        ""
		"   vec4 diffuse = iDiff * rgba * cLight;"
		""
		"	vec4 cSpec = vec4(0.5, 0.5, 0.5, 1.0);"
		"	iSpec = pow(iSpec, 100.0);"
		"	vec4 specular = iSpec * cSpec * cLight;"
		""
		"   vec4 ambient = rgba * vec4(cAmbient, 1.0);"
		""
		"	fragColor = diffuse + ambient + specular;"
		"}";

    const int unifNum = 7;
	const GLchar *uniformNames[unifNum] = {"viewing", "modeling", "dLight", "cLight", "cAmbient", "pCam", "texture0"};
	const GLchar **unifNames = uniformNames;
	const int attrNum = 3;
	const GLchar *attributeNames[attrNum] = {"position", "texCoords", "normal"};
	const GLchar **attrNames = attributeNames;

	return(shaInitialize(&sha, vertexCode, fragmentCode, unifNum, unifNames, attrNum, attrNames));
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

void render(double oldTime, double newTime) {
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
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	isoSetTranslation(&modeling, trans);
	angle += 0.2 * (newTime - oldTime);
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	GLdouble model[4][4];
	bodySetIsometry(&capsuleBody, modeling);

	/* Calculate sphere modeling isometry. */
	transSphere[0] += 0.25 * (newTime - oldTime);
	transSphere[1] += 0.25 * (newTime - oldTime);
	isoSetTranslation(&modeling, transSphere);
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	bodySetIsometry(&sphereBody, modeling);

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
	GLdouble cLight[3] = {1.0, 1.0, 1.0};
	uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);
	/* Create cAmbient uniform */
	GLdouble cAmbient[3] = {0.1, 0.1, 0.1};
	uniformVector3(cAmbient, sha.unifLocs[UNIFCAMBIENT]);
	/* Create pCam uniform */
	uniformVector3(cam.isometry.translation, sha.unifLocs[UNIFPCAMERA]);

	/* Render capsule */
	isoGetHomogeneous(&(capsuleBody.isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(capsuleBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(capsuleBody.mesh);
	/* Clean up texture */
	texUnrender(capsuleBody.tex[0], GL_TEXTURE0);

	/* Render landscape */
	isoGetHomogeneous(&(landscapeBody.isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(landscapeBody.tex[0], GL_TEXTURE1, 1, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(landscapeBody.mesh);
	/* Clean up texture */
	texUnrender(landscapeBody.tex[0], GL_TEXTURE1);

	/* Render sphere */
	isoGetHomogeneous(&(sphereBody.isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Setup texture uniform */
	texRender(sphereBody.tex[0], GL_TEXTURE2, 2, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(sphereBody.mesh);
	/* Clean up texture */
	texUnrender(sphereBody.tex[0], GL_TEXTURE2);
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
	GLdouble trans[3] = {-1.0, -1.0, -1.0};
	isoSetTranslation(&landscapeIsom, trans);
	bodySetIsometry(&landscapeBody, landscapeIsom);
	bodySetMesh(&landscapeBody, &glLandscape);
	bodySetTexture(&landscapeBody, 0, &texture1);

	if (bodyInitialize(&sphereBody, 0, 1) != 0)
	return 11;
	bodySetMesh(&sphereBody, &glSphere);
	bodySetTexture(&sphereBody, 0, &textureWater);

	return 0;
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
    window = glfwCreateWindow(768, 512, "Learning OpenGL 2.0", NULL, NULL);
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

    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	double target[3] = {0.0, 0.0, 0.0};
	camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 3.0, 5.0, 10.0, 768, 512);

	if (texInitializeFile(&texture0, "bliss.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 4;
	if (texInitializeFile(&texture1, "grass.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 5;
	if (texInitializeFile(&textureWater, "water.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 6;
    if (initializeShaderProgram() != 0)
    	return 7;
    if (initializeMesh() != 0)
		return 8;
	if (initializeBodies() != 0)
		return 9;
    
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
    bodyDestroy(&sphereBody);
	glDeleteBuffers(2, buffers);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
