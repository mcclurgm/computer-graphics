


/* On macOS, compile with...
    clang 600mainSphere.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include "000pixel.h"
#include <GLFW/glfw3.h>

#include "610vector.c"
#include "140matrix.c"
#include "610isometry.c"
#include "600camera.c"
#include "040texture.c"

#define SCREENWIDTH 512
#define SCREENHEIGHT 512

#define EPSILON 0.00000001
// #define INFINITY 100000000

camCamera camera;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

texTexture tex;

/* These are our two sphere bodies. */
isoIsometry isomA, isomB;
double radiusA = 0.7, radiusB = 1.3;
double colorA[3] = {1.0, 0.0, 1.0}, colorB[3] = {1.0, 1.0, 0.0};

/* Lighting globals */
double cLight[3] = {1.0, 1.0, 1.0};
double dLight[3] = {0.0, 0.0, -1.0};
double cSpec[3] = {1.0, 1.0, 1.0};
const double shininess = 70.0;

/* Rendering ******************************************************************/

/* The intersected member contains a code for how a ray (first) intersects the 
surface of a body: 0 if it doesn't intersect (or is tangent), 1 if it 
intersects leaving the body, or -1 if it intersects entering the body. If the 
intersected code is not 0, then time records the time of the intersection. */
typedef struct rayRecord rayRecord;
struct rayRecord {
	int intersected;
	double t;
};

/* Given the isometry and radius of a spherical body. Given a starting time and 
an ending time. Returns the ray record describing how the ray first intersects 
the sphere. */
rayRecord sphereIntersection(const isoIsometry *iso, double radius, 
		const double e[3], const double d[3], double tStart, double tEnd) {
	rayRecord result;

	/* Note: center vector (called c) is the same as iso->translation. */
	/* Get quadform values */
	double a = pow(vecLength(3, d), 2);
	double eMinusC[3];
	vecSubtract(3, e, iso->translation, eMinusC);
	double b = 2 * vecDot(3, d, eMinusC);
	double c = pow(vecLength(3, eMinusC), 2) - (radius * radius);

	double descriminant = pow(b, 2) - 4 * a * c;
	if (descriminant <= 0) {
		result.intersected = 0;
		result.t = INFINITY;
	} else {
		double tMinus = (-b - sqrt(descriminant)) / (2 * a);
		if (tMinus >= tStart && tMinus <= tEnd) {
			result.intersected = -1;
			result.t = tMinus;
		} else {
			double tPlus = (-b + sqrt(descriminant)) / (2 * a);
			if (tPlus >= tStart && tPlus <= tEnd) {
				result.intersected = 1;
				result.t = tPlus;
			} else {
				result.intersected = 0;
				result.t = INFINITY;
			}
		}
	}

	return result;
}

/* Fills the RGB color with the color sampled from the specified texture. */
void sphereColor(const isoIsometry *isom, double radius, const double e[3], 
		const double d[3], double tEnd, const texTexture *tex, double rgb[3]) {

    double dScaled[3], x[3], xLocal[3];
    vecScale(3, tEnd, d, dScaled);
    vecAdd(3, e, dScaled, x);
    isoUntransformPoint(isom, x, xLocal);

    double rho, phi, theta;
    vec3Rectangular(xLocal, &rho, &phi, &theta);
    texSample(tex, phi / M_PI, theta / (2 * M_PI), rgb);

    double dNormal[3], dCam[3], diffuse[3];
    vecSubtract(3, x, isom->translation, dNormal);
    vecUnit(3, dNormal, dNormal);
    double iDiff = vecDot(3, dNormal, dLight);
    
    /* Find dCam */
    vecScale(3, -1, dScaled, dCam);
    vecUnit(3, dCam, dCam);

    /* Specular */
    double dRefl[3], specular[3];
    vecScale(3, 2 * vecDot(3, dCam, dNormal), dNormal, dRefl);
    vecSubtract(3, dRefl, dCam, dRefl);
    double iSpec = pow(vecDot(3, dRefl, dCam), shininess);
    if (iSpec < 0)
        iSpec = 0;

    /* Check if light hits back face */
    if (iDiff < 0) {
        iDiff = 0;
        iSpec = 0;
    }

    /* Ambient */
    double cAmbient[3] = {0.1, 0.1, 0.1};

    /* Shadows: a new ray casting */
    double newTStart = EPSILON, newTEnd = INFINITY;
    rayRecord recA = sphereIntersection(&isomA, radiusA, x, dLight, newTStart, newTEnd);
    rayRecord recB = sphereIntersection(&isomB, radiusB, x, dLight, newTStart, newTEnd);
    if (recA.intersected || recB.intersected) {
        iSpec = 0.0;
        iDiff = 0.0;
    }

    rgb[0] = (iDiff * rgb[0] * cLight[0]) + (iSpec * cSpec[0] * cLight[0]) + (rgb[0] * cAmbient[0]);
    rgb[1] = (iDiff * rgb[1] * cLight[1]) + (iSpec * cSpec[1] * cLight[1]) + (rgb[1] * cAmbient[1]);
    rgb[2] = (iDiff * rgb[2] * cLight[2]) + (iSpec * cSpec[2] * cLight[2]) + (rgb[2] * cAmbient[2]);
}

/* Fills the RGB color with the color sampled from the specified texture. */
void sphereReflection(const isoIsometry *isom, double radius, const double e[3], 
		const double d[3], double tEnd, const texTexture *tex, double rgb[3]) {

    double dScaled[3], x[3], xLocal[3];
    vecScale(3, tEnd, d, dScaled);
    vecAdd(3, e, dScaled, x);
    isoUntransformPoint(isom, x, xLocal);

    double rho, phi, theta;
    vec3Rectangular(xLocal, &rho, &phi, &theta);
    texSample(tex, phi / M_PI, theta / (2 * M_PI), rgb);

    double dNormal[3], dCam[3], dRefl[3];
    vecSubtract(3, x, isom->translation, dNormal);
    vecUnit(3, dNormal, dNormal);
    
    vecScale(3, -1, dScaled, dCam);
    vecUnit(3, dCam, dCam);

    vecScale(3, 2 * vecDot(3, dCam, dNormal), dNormal, dRefl);
    vecSubtract(3, dRefl, dCam, dRefl);

    /* Set default color */
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
    
    double newTStart = EPSILON, newTEnd = INFINITY;
    rayRecord recA = sphereIntersection(&isomA, radiusA, x, dRefl, newTStart, newTEnd);
    if (recA.intersected) {
        newTEnd = recA.t;
        sphereColor(&isomA, radiusA, x, dRefl, newTEnd, tex, rgb);
    }

    /* Test the second sphere. */
    rayRecord recB = sphereIntersection(&isomB, radiusB, x, dRefl, newTStart, newTEnd);
    if (recB.intersected) { // sphere B wins
        newTEnd = recB.t;
        sphereColor(&isomB, radiusB, x, dRefl, newTEnd, tex, rgb);  
    }
}

void render(void) {
	double homog[4][4], screen[4], world[4], e[3], d[3], rgb[3];
	double tStart, tEnd;
	rayRecord recA, recB;
	int i, j;
	pixClearRGB(0.0, 0.0, 0.0);
	
	/* Get camera world position e and transformation from screen to world. */
	vecCopy(3, camera.isometry.translation, e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);

	/* Each screen point is arbitrarily chosen on the near plane. */
	screen[2] = 0.0;
	screen[3] = 1.0;
	
	for (i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;

			/* Compute the direction d from the camera to the pixel. */
			mat441Multiply(homog, screen, world);
			vecScale(4, 1 / world[3], world, world);
			vecSubtract(3, world, e, d);

			/* Prepare to loop over all bodies. */
			tStart = EPSILON;
			tEnd = INFINITY;

			/* Set default color */
			rgb[0] = 0;
			rgb[1] = 0;
			rgb[2] = 0;
			
			/* Test the first sphere. */
			recA = sphereIntersection(&isomA, radiusA, e, d, tStart, tEnd);
			if (recA.intersected) {
				tEnd = recA.t;
                sphereReflection(&isomA, radiusA, e, d, tEnd, &tex, rgb);
			}

			/* Test the second sphere. */
			recB = sphereIntersection(&isomB, radiusB, e, d, tStart, tEnd);
			if (recB.intersected) { // sphere B wins
                tEnd = recB.t;
                sphereColor(&isomB, radiusB, e, d, tEnd, &tex, rgb);
			}

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

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_C)
		handleKeyCamera(key);
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

int main(void) {
	if (pixInitialize(SCREENWIDTH, SCREENHEIGHT, "Ray Tracing") != 0)
		return 1;
	else {
        vecUnit(3, dLight, dLight);

		/* Initialize the scene. */
		camSetProjectionType(&camera, camPERSPECTIVE);
		camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
			SCREENHEIGHT);
		camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);

		double center[3] = {0.0, 0.0, 0.0}, axis[3] = {1.0, 0.0, 0.0}, r[3][3];
		mat33AngleAxisRotation(0.0, axis, r);
		isoSetTranslation(&isomA, center);
		isoSetRotation(&isomA, r);
		vec3Set(1.0, 0.0, 2.0, center);
		isoSetTranslation(&isomB, center);
		isoSetRotation(&isomB, r);

        texInitializeFile(&tex, "bliss.jpg");
        
		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		return 0;
	}
}


