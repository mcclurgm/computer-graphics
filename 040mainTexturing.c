
//
//  020mainRasterizing.c
//  Testing 020triangle.c and triRender.
//
//  Created by Michael McClurg and Vermilion Villarreal on 1/9/19.
//

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

#include <GLFW/glfw3.h>
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "040triangle.c"

int texFilter = 0;
texTexture tex;
double A[2] = {10.0, 10.0};
double B[2] = {500.0, 44.0};
double C[2] = {250.0, 400.0};
double rgb[3] = {.6, .3, .9};
double alpha[2] = {1.0, 0.0};
double beta[2] = {0.0, 0.0};
double gamma2[2] = {1.0, 1.0};

void switchFilter(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {

	if(key == GLFW_KEY_ENTER) {
	    if(texFilter == 1) {
	        texSetFiltering(&tex, 0);
	        texFilter = 0;
	    } else {
	        texSetFiltering(&tex, 1);
	        texFilter = 1;
	    }
	}
	
    triRender(A, B, C, rgb, &tex, alpha, beta, gamma2);
}

int main(void){
    if (pixInitialize(512, 512, "Triangle things!") != 0) {
        return 1;
    } else {
        pixClearRGB(0.0, 0.0, 0.0);
        
        if(texInitializeFile(&tex, "zucksmall.jpg") != 0) {
            printf("Failed to initialize texture.\n");
            return 1;
        }
        
        pixSetKeyDownHandler(switchFilter);

        //Triangles are created from bottom left to right then up in rows.
        triRender(A, B, C, rgb, &tex, alpha, beta, gamma2);
        
        pixRun();
        texDestroy(&tex);
        return 0;
    }
}
