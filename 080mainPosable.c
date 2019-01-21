


/* On macOS, compile with...
    clang 050mainAbstracted.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "080shading.c"

/* It is important that colorPixel correctly parse the data that we give it. To 
help avoid errors in communication, we set up some compile-time constants. 
Notice that the documentation for the new triRender requires mainATTRX to be 0 
and mainATTRY to be 1. */
#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainATTRR 4
#define mainATTRG 5
#define mainATTRB 6
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYS 2
#define mainVARYT 3
#define mainVARYR 4
#define mainVARYG 5
#define mainVARYB 6
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFTH 3
#define mainUNIFX 4
#define mainUNIFY 5
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* attr has already been interpolated from the vertex attributes. tex is an 
array of texNum elements, each of which is of type texTexture *. rgb is the 
output parameter. The other parameters should be self-explanatory. For reasons 
that become clear later in the course, colorPixel is allowed to use (A) compile-
time constants such as mainATTRX, (B) functions such as sin() and texSample(), 
(C) its parameters such as unifDim, and (D) any variables that it declares 
locally. It is not allowed to use any other variables, such as global variables 
that aren't parameters. */
void colorPixel(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[3]) {
	double sample[3];
	texSample(tex[0], vary[2], vary[3], sample);
	
	rgb[0] = sample[0] * unif[0];
	rgb[1] = sample[1] * unif[1];
	rgb[2] = sample[2] * unif[2];
}

/* Outputs vary, based on the other parameters, which are unaltered. Like 
colorPixel, this function should not access any variables other than its 
parameters and any local variables that it declares. */
void transformVertex(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {
    /* Copy texture coordinates to vary, since they are unchanged. */
    vary[mainVARYX] = attr[mainATTRX];
    vary[mainVARYY] = attr[mainATTRY];
    vary[mainVARYS] = attr[mainATTRS];
    vary[mainVARYT] = attr[mainATTRT];
    
    double col1[2] = {cos(unif[mainUNIFTH]), sin(unif[mainUNIFTH])};
    double col2[2] = {-sin(unif[mainUNIFTH]), cos(unif[mainUNIFTH])};
    double rotation[2][2];
    mat22Columns(col1, col2, rotation);
    
    double rotated[2];
    mat221Multiply(rotation, vary, rotated);
    
    double translation[2] = {unif[mainUNIFX], unif[mainUNIFY]};
    vecAdd(2, rotated, translation, vary);
}

/* We have to include 050triangle.c after defining colorPixel, because it 
refers to colorPixel. (Later in the course we handle this issue better.) */
#include "080triangle.c"
#include "080mesh.c"

/* This struct is initialized in main() below. */
shaShading sha;
/* Here we make an array of one texTexture pointer, in such a way that the 
const qualifier can be enforced throughout the surrounding code. C is confusing 
for stuff like this. Don't worry about mastering C at this level. It doesn't 
come up much in our course. */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;

double unif[6] = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0};

meshMesh mesh;

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	meshRender(&mesh, &sha, unif, tex);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown, 
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		if (texture.filtering == texLINEAR)
			texSetFiltering(&texture, texNEAREST);
		else
			texSetFiltering(&texture, texLINEAR);
		draw();
	}
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
		
		double timeStep = newTime - oldTime;
		
		unif[mainUNIFTH] += 0.1 * timeStep;
		unif[mainUNIFX] += 3 * timeStep;
		unif[mainUNIFY] += 10 * timeStep;
		draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if (texInitializeFile(&texture, "zuck.jpg") != 0)
			return 2;
		else {
		    if(meshInitializeEllipse(&mesh, 250, 250, 100, 100, 5) != 0) {
		        return 3;
		    } else {
			    texSetFiltering(&texture, texNEAREST);
			    texSetLeftRight(&texture, texREPEAT);
			    texSetTopBottom(&texture, texREPEAT);
			    sha.unifDim = 3;
			    sha.attrDim = 2 + 2;
			    sha.varyDim = sha.attrDim;
			    sha.texNum = 1;
			    draw();
			    pixSetKeyUpHandler(handleKeyUp);
			    pixSetTimeStepHandler(handleTimeStep);
			    pixRun();
			    texDestroy(&texture);
			    return 0;
			}
		}
	}
}

