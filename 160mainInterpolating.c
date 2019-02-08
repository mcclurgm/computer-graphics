


/* On macOS, compile with...
    clang 140mainLandscape.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "120vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "130shading.c"
#include "130depth.c"
#include "130triangle.c"
#include "160mesh.c"
#include "140isometry.c"
#include "150camera.c"
#include "140landscape.c"

#define mainSCREENSIZE 512

/*** Shaders ***/

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRZ 2
#define mainATTRS 3
#define mainATTRT 4
#define mainATTRN 5
#define mainATTRO 6
#define mainATTRP 7
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYZ 2
#define mainVARYW 3
#define mainVARYS 4
#define mainVARYT 5
#define mainVARYN 6
#define mainVARYO 7
#define mainVARYP 8
#define mainVARYWORLDZ 9
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFMIN 4
#define mainUNIFMEAN 5
#define mainUNIFMAX 6
#define mainUNIFCAMERA 7
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgbd[4]) {

	double sample[tex[0]->texelDim];
	double s = vary[mainVARYS] / vary[mainVARYW];
	double t = vary[mainVARYT] / vary[mainVARYW];
	texSample(tex[0], s, t, sample);

	rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
	rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
	double attrHom[4] = {attr[0], attr[1], attr[2], 1.0};
	double worldHom[4], varyHom[4], varyHomDivided[4];
	/* The modeling transformation is just Z-translation. So this code is much
	simpler than the usual matrix multiplication. */
	vecCopy(4, attrHom, worldHom);
	worldHom[2] += unif[mainUNIFMODELING];
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
	vecCopy(4, varyHom, vary);
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
	vary[mainVARYWORLDZ] = worldHom[2];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
camCamera cam;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 400.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Meshes to be rendered. */
meshMesh bliss;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
double unif[3 + 1 + 3 + 16] = {
	1.0, 1.0, 1.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};

/*** User interface ***/

void render(void) {
	double view[4][4], projInvIsom[4][4], viewProjInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	vecCopy(16, (double *)projInvIsom, &unif[mainUNIFCAMERA]);
	meshRender(&bliss, &buf, view, &sha, unif, tex);
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
	else if (key == GLFW_KEY_B)
		camSetProjectionType(&cam, camPERSPECTIVE);
	else if (key == GLFW_KEY_N)
		camSetProjectionType(&cam, camORTHOGRAPHIC);

	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
		mainSCREENSIZE);
	camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
	render();
}

int main(void) {
	/* Begin configuring scene. */
	if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
		return 1;
	else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
		return 2;
	else if (meshInitializeBox(&bliss, 100, 350, 100, 350, 100, 350) != 0)
		return 3;
	else if (texInitializeFile(&texture, "bliss.jpg") != 0) {
		return 4;
	}
	else {
		/* Continue configuring scene. */
		sha.unifDim = 3 + 1 + 3 + 16;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 5 + 2 + 3;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 1;
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
			mainSCREENSIZE);
		vec3Set(0, 0,
			0, cameraTarget);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		/* User interface. */
		pixSetKeyDownHandler(handleKeyAny);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		meshDestroy(&bliss);
		texDestroy(&texture);
		depthDestroy(&buf);
		return 0;
	}
}
