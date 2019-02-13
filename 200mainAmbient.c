


/* On macOS, compile with...
    clang 140mainLandscape.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "170engine.h"

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
#define mainUNIFDLIGHT 23
#define mainUNIFCLIGHT 26
#define mainUNIFDCAMERA 29
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgbd[4]) {

	// Sample texture color
	double sample[tex[0]->texelDim];
	double s = vary[mainVARYS] / vary[mainVARYW];
	double t = vary[mainVARYT] / vary[mainVARYW];
	texSample(tex[0], s, t, sample);

	// Diffuse reflection initial math
	double normal[3], dLight[3], diffuse[3];
	vecCopy(3, &vary[mainVARYN], normal);
	vecCopy(3, &unif[mainUNIFDLIGHT], dLight);
	vecUnit(3, normal, normal);
	vecUnit(3, dLight, dLight);
	double iDiff = vecDot(3, dLight, normal);

	// Specular reflection initial math
	double iSpec, scaledDNormal[3], dRefl[3], dCam[3], specular[3];
	vecCopy(3, &unif[mainUNIFDCAMERA], dCam);
	vecUnit(3, dCam, dCam);
	vecScale(3, 2 * iDiff, normal, scaledDNormal);
	vecSubtract(3, scaledDNormal, dLight, dRefl);
	iSpec = pow(vecDot(3, dRefl, dCam), 10);
	if(iSpec < 0)
		iSpec = 0;

	// Implement max for intensities
	if(iDiff < 0) {
		iDiff = 0;
		iSpec = 0;
	}

	// Calculate final results
	diffuse[0] = iDiff * sample[0] * unif[mainUNIFCLIGHT];
	diffuse[1] = iDiff * sample[1] * unif[mainUNIFCLIGHT + 1];
	diffuse[2] = iDiff * sample[2] * unif[mainUNIFCLIGHT + 2];

	specular[0] = iSpec * 0.5 * unif[mainUNIFCLIGHT];
	specular[1] = iSpec * 0.5 * unif[mainUNIFCLIGHT + 1];
	specular[2] = iSpec * 0.5 * unif[mainUNIFCLIGHT + 2];

    // Ambient lighting: complete math
    double cAmbient[3] = {0.1, 0.1, 0.1}, ambient[3];
    ambient[0] = cAmbient[0] * sample[0];
    ambient[1] = cAmbient[1] * sample[1];
    ambient[2] = cAmbient[2] * sample[2];

    double specPlusDiff[3];
	vecAdd(3, diffuse, specular, specPlusDiff);
    vecAdd(3, specPlusDiff, ambient, rgbd);
	rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
	double attrHom[4] = {attr[0], attr[1], attr[2], 1.0};
	double worldHom[4], varyHom[4], world0[4];
	/* The modeling transformation is just Z-translation. So this code is much
	simpler than the usual matrix multiplication. */
	vecCopy(4, attrHom, worldHom);
	vecCopy(3, &attr[mainATTRN], world0);
	world0[3] = 0;
	worldHom[2] += unif[mainUNIFMODELING];
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
	vecCopy(4, varyHom, vary);

	vecUnit(3, &attr[mainATTRN], varyHom);

	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
	vary[mainVARYN] = varyHom[0];
	vary[mainVARYO] = varyHom[1];
	vary[mainVARYP] = varyHom[2];
	vary[mainVARYWORLDZ] = worldHom[2];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
camCamera cam;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 256.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
texTexture waterTex;
const texTexture *wat[1] = {&waterTex};
const texTexture **waters = wat;
texTexture rockTex;
const texTexture *roc[1] = {&rockTex};
const texTexture **rocks = roc;
/* Meshes to be rendered. */
meshMesh grass;
double unifGrass[3 + 1 + 3 + 16 + 3 + 3 + 3] = {
	0.0, 1.0, 0.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,
  	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0};
meshMesh rock;
double unifRock[3 + 1 + 3 + 16 + 3 + 3 + 3] = {
	1.0, 1.0, 1.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0};
meshMesh water;
double unifWater[3 + 1 + 3 + 16 + 3 + 3 + 3] = {
	0.0, 0.0, 1.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0};

/*** User interface ***/

void render(void) {
	double view[4][4], projInvIsom[4][4], viewProjInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	vecCopy(16, (double *)projInvIsom, &unifGrass[mainUNIFCAMERA]);
	meshRender(&grass, &buf, view, &sha, unifGrass, tex);
	vecCopy(16, (double *)projInvIsom, &unifRock[mainUNIFCAMERA]);
	meshRender(&rock, &buf, view, &sha, unifRock, rocks);
	vecCopy(16, (double *)projInvIsom, &unifWater[mainUNIFCAMERA]);
	meshRender(&water, &buf, view, &sha, unifWater, waters);

    double zVec[3] = {0, 0, 1}, dCam[3];
    isoRotateVector(&(cam.isometry), zVec, dCam);
    vecCopy(3, dCam, &unifGrass[mainUNIFDCAMERA]);
    vecCopy(3, dCam, &unifWater[mainUNIFDCAMERA]);
    vecCopy(3, dCam, &unifRock[mainUNIFDCAMERA]);
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
	else if (key == GLFW_KEY_J)
		unifWater[mainUNIFMODELING] -= 0.1;
	else if (key == GLFW_KEY_U)
		unifWater[mainUNIFMODELING] += 0.1;
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
	/* Design landscape and water. */
	int landNum = 100;
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
	double waterData[4] = {landMin, landMin, landMin, landMin};
	unifGrass[mainUNIFMIN] = landMin;
	unifGrass[mainUNIFMEAN] = landMean;
	unifGrass[mainUNIFMAX] = landMax;
	unifRock[mainUNIFMIN] = landMin;
	unifRock[mainUNIFMEAN] = landMean;
	unifRock[mainUNIFMAX] = landMax;
	unifWater[mainUNIFMIN] = landMin;
	unifWater[mainUNIFMEAN] = landMean;
	unifWater[mainUNIFMAX] = landMax;
	meshMesh land;
	/* Begin configuring scene. */
	if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
		return 1;
	else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
		return 2;
	else if (meshInitializeLandscape(&land, landNum, landNum, 1.0,
			(double *)landData) != 0)
		return 3;
	else if (meshInitializeDissectedLandscape(&grass, &land, M_PI / 4.0,
			1) != 0)
		return 4;
	else if (meshInitializeDissectedLandscape(&rock, &land, M_PI / 4.0,
			0) != 0)
		return 5;
	else if (meshInitializeLandscape(&water, 2, 2, landNum - 1.0,
			(double *)waterData) != 0)
		return 6;
	else if (texInitializeFile(&texture, "grass.jpg") != 0)
		return 7;
	else if (texInitializeFile(&waterTex, "water.jpg") != 0)
		return 7;
	else if (texInitializeFile(&rockTex, "rock.jpg") != 0)
		return 7;
	else {
		meshDestroy(&land);
		/* Continue configuring scene. */
		sha.unifDim = 3 + 1 + 3 + 16 + 3 + 3 + 3;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 4 + 2 + 3;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 0;
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
			mainSCREENSIZE);
		vec3Set(landNum / 2.0, landNum / 2.0,
			landData[landNum / 2][landNum / 2], cameraTarget);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		/* User interface. */
		pixSetKeyDownHandler(handleKeyAny);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		meshDestroy(&grass);
		meshDestroy(&rock);
		meshDestroy(&water);
		depthDestroy(&buf);
		texDestroy(&texture);
		texDestroy(&waterTex);
		texDestroy(&rockTex);
		return 0;
	}
}
