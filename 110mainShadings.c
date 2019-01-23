/* Thoughts on effects:
    Put Mark in a circle (a real circle this time)
    Make him roll around (with user input? Right/left[/up/down])
    Move texture behind him in opposite direction of his movement
        We can implement this by simply changing texture coordinates so it repeats
        Texture only goes side to side
*/


/* On macOS, compile with...
    clang 090mainAbstracted.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "100matrix.c"
#include "040texture.c"
#include "090shading.c"
/* New. We no longer need to include these files after colorPixel and 
transformVertex. So instead we include them up here. It's good C style to have 
all #includes in one section near the top of the file. */
#include "090triangle.c"
#include "090mesh.c"

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYS 2
#define mainVARYT 3
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFTRANSL0 3
#define mainUNIFTRANSL1 4
#define mainUNIFROT 5
#define mainUNIFMODELING 0
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

double translationStep = 0;

void colorPixelMark(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFB];
}

void transformVertexMark(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {

	double varyHomog[3], attrHomog[3] = {attr[0], attr[1], 1};
	mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
	
	vecCopy(2, varyHomog, vary);
	
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
}

void colorPixelBliss(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[1], vary[mainVARYS] + translationStep, vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * sin(translationStep);
	rgb[1] = sample[mainTEXG] * cos(translationStep);
	rgb[2] = sample[mainTEXB] * sin(translationStep / 2);
}

void transformVertexBliss(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {

	double varyHomog[3], attrHomog[3] = {attr[0], attr[1], 1};
	mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
	
	vecCopy(2, varyHomog, vary);
	
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
}

double rotationAngle = 0.1;
double translationVectorMark[2] = {1.0, 0.0};
double zeroVector[2] = {0.0, 0.0};
shaShading shaMark;
shaShading shaBliss;
texTexture textureMark;
texTexture textureBliss;
const texTexture *textures[2] = {&textureMark, &textureBliss};
const texTexture **tex = textures;
meshMesh meshMark;
meshMesh meshBliss;
double unifMark[3 + 3] = {1.0, 1.0, 1.0, -128.0, -128.0, 0.0};
double unifBliss[3 + 3] = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0};

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	meshRender(&meshBliss, &shaBliss, unifBliss, tex);
	meshRender(&meshMark, &shaMark, unifMark, tex);
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));

    translationStep += 0.01;

	unifBliss[mainUNIFR] = sin(newTime);
	unifBliss[mainUNIFG] = cos(oldTime);
	rotationAngle += (newTime - oldTime);
	translationVectorMark[0] += 1;
    
    double isomMark[3][3]; // Mark
    mat33Isometry(0.0, translationVectorMark, isomMark);
    vecCopy(9, (double *)isomMark, &unifMark[mainUNIFMODELING]);
    double isomBliss[3][3]; // Bliss
    mat33Isometry(0.0, zeroVector, isomBliss);
    vecCopy(9, (double *)isomBliss, &unifBliss[mainUNIFMODELING]);

	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&textureMark, "zuck.jpg") != 0)
		return 2;
	else if (meshInitializeEllipse(&meshMark, 66.0, 66.0, 64.0, 64.0, 40) != 0)
		return 3;
	else if (texInitializeFile(&textureBliss, "bliss.jpg") != 0)
	    return 4;
	else if (meshInitializeRectangle(&meshBliss, 0.0, 512.0, 0.0, 512.0) != 0)
		return 3;
	else {
// 		Setup Mark
		texSetFiltering(&textureMark, texNEAREST);
		texSetLeftRight(&textureMark, texREPEAT);
		texSetTopBottom(&textureMark, texREPEAT);
		shaMark.unifDim = 3 + 3;
		shaMark.attrDim = 2 + 2;
		shaMark.varyDim = 2 + 2;
		shaMark.texNum = 1;
		shaMark.colorPixel = colorPixelMark;
		shaMark.transformVertex = transformVertexMark;

// 		Setup Bliss
		texSetFiltering(&textureBliss, texNEAREST);
		texSetLeftRight(&textureBliss, texREPEAT);
		texSetTopBottom(&textureBliss, texREPEAT);
		shaBliss.unifDim = 3 + 3;
		shaBliss.attrDim = 2 + 2;
		shaBliss.varyDim = 2 + 2;
		shaBliss.texNum = 1;
		shaBliss.colorPixel = colorPixelBliss;
		shaBliss.transformVertex = transformVertexBliss;

		draw();
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		meshDestroy(&meshMark);
		texDestroy(&textureMark);
		meshDestroy(&meshBliss);
		texDestroy(&textureBliss);
		return 0;
	}
}

