


/* On macOS, compile with...
    clang 650mainBody.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "000pixel.h"

#include "610vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "610isometry.c"
#include "600camera.c"
#include "680ray.c"
#include "680light.c"
#include "680omnidirectional.c"
#include "690directional.c"

#define SCREENWIDTH 512
#define SCREENHEIGHT 512
#define BODYNUM 5
#define LIGHTNUM 1

double dLightRaw[3] = {-1.0, -1.0, 1.0}, dLight[3];
double pLight[3] = {1.0, 1.0, 10.0};
// double cLight[3] = {1.0, 1.0, 1.0};
double cLight[3] = {1.0, 1.0, 1.0};
double cTest[3] = {1.0, 1.0, 0.0};
double cAmbient[3] = {0.3, 0.3, 0.3};

#include "680cylinder.c"
#include "680sphere.c"
#include "680plane.c"

camCamera camera;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

texTexture textureRed, textureGreen, textureBlue, zuck;
cylCylinder cylRed, cylGreen, cylBlue;
sphereSphere sphere;
plaPlane plane;
const void *bodies[BODYNUM] = {&cylRed, &cylGreen, &cylBlue, &sphere, &plane};
omniLight omni;
dirLight dir;
const void *lights[LIGHTNUM] = {&omni, &dir};

/* Rendering ******************************************************************/

/* If the ray does not intersect the scene, then returns -1. If the ray 
intersects the scene, then updates query->tEnd and response and returns the 
index of the body that the ray intersects (first). */
int queryScene(rayQuery *query, rayResponse *response) {
	rayResponse candidate;
	int bestK = -1;
	rayClass **class;
	for (int k = 0; k < BODYNUM; k += 1) {
		class = (rayClass **)(bodies[k]);
		candidate = (*class)->intersection(bodies[k], query);
		if (candidate.intersected) {
			query->tEnd = candidate.t;
			*response = candidate;
			bestK = k;
		}
	}
	return bestK;
}

void render(void) {
	double homog[4][4], world[4], rgb[3];
	double screen[4] = {0.0, 0.0, 0.0, 1.0};
	rayQuery query;
	rayResponse response;
	vecUnit(3, dLightRaw, dLight);
	/* Compute the position e of the camera. */
	vecCopy(3, camera.isometry.translation, query.e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);
	for (int i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (int j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;
			/* Compute the direction d from the camera to the pixel. */
			mat441Multiply(homog, screen, worrld);
			vecScale(4, 1.0 / world[3], world, world);
			vecSubtract(3, world, query.e, query.d);
			/* Query the scene to find the intersection, if any. */
			query.tStart = rayEPSILON;
			query.tEnd = rayINFINITY;
			int index = queryScene(&query, &response);
			/* Color the pixel. */
			rayClass **class;
			if (index >= 0) {
				class = (rayClass **)(bodies[index]);
				(*class)->color(bodies[index], &query, &response, BODYNUM, 
                    bodies, LIGHTNUM, lights, cAmbient, rgb);
			} else
				vec3Set(0.0, 0.0, 0.0, rgb);
			pixSetRGB(i, j, rgb[0], rgb[1], rgb[2]);
		}
	}
}

/* User interface *************************************************************/

void handleKeyCamera(int key) {
	if (key == GLFW_KEY_W)
		cameraPhi -= 0.1;
	else if (key == GLFW_KEY_A)
		cameraTheta -= 0.1;
	else if (key == GLFW_KEY_S)
		cameraPhi += 0.1;
	else if (key == GLFW_KEY_D)
		cameraTheta += 0.1;
	else if (key == GLFW_KEY_E)
		cameraRho *= 0.9;
	else if (key == GLFW_KEY_C)
		cameraRho *= 1.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyLight(int key) {
	if (key == GLFW_KEY_O)
		dLightRaw[1] += 0.1;
	else if (key == GLFW_KEY_K)
		dLightRaw[0] -= 0.1;
	else if (key == GLFW_KEY_L)
		dLightRaw[1] -= 0.1;
	else if (key == GLFW_KEY_SEMICOLON)
		dLightRaw[0] += 0.1;
	else if (key == GLFW_KEY_P)
		dLightRaw[2] += 0.1;
	else if (key == GLFW_KEY_SLASH)
		dLightRaw[2] -= 0.1;
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_C)
		handleKeyCamera(key);
	else if (key == GLFW_KEY_O || key == GLFW_KEY_K || key == GLFW_KEY_L ||
			key == GLFW_KEY_SEMICOLON || key == GLFW_KEY_P || 
			key == GLFW_KEY_SLASH)
		handleKeyLight(key);
}

void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_Q)
		cameraMode = 1 - cameraMode;
	else
		handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown, 
			superCommandIsDown);
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/s\n", 1.0 / (newTime - oldTime));
	render();
}

void initializeCylinder(cylCylinder *cyl, double radius, 
		const double rot[3][3], const double transl[3], texTexture *texture) {
	cyl->class = &cylClass;
	isoSetRotation(&(cyl->isometry), rot);
	isoSetTranslation(&(cyl->isometry), transl);
	cyl->radius = radius;
	cyl->texture = texture;
}

void initializeSphere(sphereSphere *sph, double radius, const double rot[3][3],
        const double transl[3], texTexture *texture) {
    sph->class = &sphereClass;
    isoSetRotation(&(sph->isometry), rot);
    isoSetTranslation(&(sph->isometry), transl);
    sph->radius = radius;
    sph->texture = texture;
}

void initializePlane(plaPlane *pl, const double rot[3][3], 
		const double transl[3], texTexture *texture) {
	pl->class = &planeClass;
    isoSetRotation(&(pl->isometry), rot);
    isoSetTranslation(&(pl->isometry), transl);
	pl->texture = texture;
}

void initializeOmni(omniLight *light, const double pLight[3], const double cLight[3]) {
    light->class = &omniClass;
    printf("dLight %f\n", pLight[0]);
    vecUnit(3, pLight, light->pLight);
    vecCopy(3, cLight, light->cLight);
}

void initializeDir(dirLight *light, const double dLight[3], const double cLight[3]) {
    light->class = &dirClass;
    vecUnit(3, dLight, light->dLight);
    vecCopy(3, cLight, light->cLight);
}

int main(void) {
	if (pixInitialize(SCREENWIDTH, SCREENHEIGHT, "Ray Tracing") != 0)
		return 1;
	else {
		/* Initialize the scene. */
        vecUnit(3, dLightRaw, dLight);
		camSetProjectionType(&camera, camPERSPECTIVE);
		camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
			SCREENHEIGHT);
		camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		double rgb[3] = {1.0, 0.0, 0.0};
		if (texInitializeSolid(&textureRed, 4, 4, 3, rgb) != 0)
			return 2;
		vec3Set(0.0, 1.0, 0.0, rgb);
		if (texInitializeSolid(&textureGreen, 4, 4, 3, rgb) != 0)
			return 3;
		vec3Set(0.0, 0.0, 1.0, rgb);
		if (texInitializeSolid(&textureBlue, 4, 4, 3, rgb) != 0)
			return 4;
        if (texInitializeFile(&zuck, "bliss.jpg") != 0)
            return 5;

		texSetTopBottom(&zuck, texREPEAT);
		texSetLeftRight(&zuck, texREPEAT);
		double center[3] = {0.0, 0.0, 0.0}, rot[3][3];
		double axis[3] = {-1.0 / sqrt(3.0), -1.0 / sqrt(3.0), -1.0 / sqrt(3.0)};
		mat33AngleAxisRotation(-2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylRed, 0.1, rot, center, &textureRed);
		mat33AngleAxisRotation(2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylGreen, 0.1, rot, center, &textureGreen);
		mat33AngleAxisRotation(0.0, axis, rot);
		initializeCylinder(&cylBlue, 0.1, rot, center, &textureBlue);
        
        double sphereTransl[3] = {1.0, -1.0, 0.0};
        initializeSphere(&sphere, 1.0, rot, sphereTransl, &zuck);
		// mat33AngleAxisRotation(0.0, axis, rot);
		mat33AngleAxisRotation(0.0 * M_PI / 3.0, axis, rot);
		initializePlane(&plane, rot, center, &zuck);

        /* Initialize lights */
        initializeOmni(&omni, pLight, cLight);
        initializeDir(&dir, dLight, cTest);
        
		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		texDestroy(&textureRed);
		texDestroy(&textureGreen);
		texDestroy(&textureBlue);
        texDestroy(&zuck);
		return 0;
	}
}


