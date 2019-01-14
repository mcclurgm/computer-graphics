
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
#include "030triangle.c"

int main(void){
    if (pixInitialize(512, 512, "Triangle things!") != 0) {
        return 1;
    } else {
        pixClearRGB(0.0, 0.0, 0.0);
        double A[2] = {10.0, 10.0};
        double B[2] = {400.0, 244.0};
        double C[2] = {250.0, 400.0};
        double rgb[3] = {.6, .3, .9};
        double alpha[3] = {1.0, 0.0, 0.0};
        double beta[3] = {0.0, 1.0, 0.0};
        double gamma[3] = {0.0, 0.0, 1.0};
        //Triangles are created from bottom left to right then up in rows.
        triRender(A, B, C, rgb, alpha, beta, gamma);
        
        pixRun();
        return 0;
    }
}
