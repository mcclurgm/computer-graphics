//
//  020triangle.c
//  triRender and helper functions.
//  This renders triangles input to triRender as point pairs.
//  They must all be in counterclockwise point orientation.
//  It must be called on after pixInitialize and before pixRun.
//
//  Created by Michael McClurg and Vermilion Villarreal on 1/9/19.
//

#include <stdio.h>
#include <math.h>
#include "000pixel.h"
#include <GLFW/glfw3.h>

/*findyBound takes two locations and a x value and creates a line between the two locations
  and finds the corresponding y value at the given x value using that line.
  The upper parameter defines whether this function is finding an upper or lower bound.
  1 means it upper bound, anything else (preferably 0) means lower. */
double findyBound(const double a[2], const double b[2], int x0, int upper){
    if(b[0] == a[0]) {
        if(upper == 1)
            if(a[1] > b[1])
                return a[1];
            else
                return b[1];
        else
            if(a[1] > b[1])
                return b[1];
            else
                return a[1];
    }

    double m = (b[1] - a[1]) / (b[0] - a[0]);
    double y = m * ((double)x0 - a[0]) + a[1];
    return y;
}

/* Finds the values p and q that are used in the interpolation algorithm.
Returns in vector form, [p q].
*/
void findPQ(const double x[2], const double a[2], double mInv[2][2], 
        double pq[2]) {

    double xMinusA[2];
    vecSubtract(2, x, a, xMinusA);

    mat221Multiply(mInv, xMinusA, pq);
}

void interpolateVary(const int varyDim, const double a[], const double b[], const double c[],
        const double pq[2], const double x[2], double vary[]) {
    
//  Declare all the vectors we need to do the linear equation
    double bMinusA[varyDim], cMinusA[varyDim], scaledP[varyDim], scaledQ[varyDim], pPlusQ[varyDim], st[varyDim];
    
    vecSubtract(varyDim, b, a, bMinusA);
    vecSubtract(varyDim, c, a, cMinusA);
    
    vecScale(varyDim, pq[0], bMinusA, scaledP);
    vecScale(varyDim, pq[1], cMinusA, scaledQ);
    
    vecAdd(varyDim, scaledP, scaledQ, pPlusQ);
    vecAdd(varyDim, a, pPlusQ, vary);
    
//  Set the first two elements of vary, which are the current rasterizing coordinates
    vary[0] = x[0];
    vary[1] = x[1];
}

/*triRender takes 3 points on the initialized graphics and creates a triangle with the given RGB
  value. All triangles must be counterclockwise and are then formed into one of two base cases before
  being drawn. */
void triRender(const shaShading *sha,
        const double unif[], const texTexture *tex[],
        const double a[], const double b[], const double c[]) {
    //Check if the location in position "a" is the left most point, if not recall triRender in a
    //different order.
    if(b[0] < a[0]) {
        triRender(sha, unif, tex, b, c, a);
    } else if(c[0] < a[0]) {
        triRender(sha, unif, tex, c, a, b);
    } else {
        int x0;
        int lowery;
        int uppery;
        
//      Generate m from its columns
        double bMinusA[2], cMinusA[2], m[2][2], mInv[2][2], det;
        double aCoord[2] = {a[0], a[1]};
        double bCoord[2] = {b[0], b[1]};
        double cCoord[2] = {c[0], c[1]};
        
        vecSubtract(2, bCoord, aCoord, bMinusA);
        vecSubtract(2, cCoord, aCoord, cMinusA);
        mat22Columns(bMinusA, cMinusA, m);

//      Check if m is an invertible matrix; break if not.
        det = mat22Invert(m, mInv);
        if(det == 0) {
            return;
        }
    
        double pq[2], pointRGB[3], vary[sha->varyDim];
        
        //1st case where the location "b" is to the right of "c"
        if(b[0] > c[0]) {
            //1st loop from the leftmost point, "a", until we get to c0
            for(x0 = (int)ceil(a[0]); x0 <= (int)floor(c[0]); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a, b, x0, 0));
                uppery = (int)floor(findyBound(a, c, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, mInv, pq);
                    interpolateVary(sha->varyDim, a, b, c, pq, currentX, vary);
                    
                    sha->colorPixel(sha->unifDim, unif, sha->texNum, tex, sha->varyDim, vary, pointRGB);
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
            
            //2nd loop from floor(c0) + 1 until we get to b0
            for(x0 = (int)floor(c[0]) + 1; x0 <= (int)floor(b[0]); x0 = x0 + 1) {
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a, b, x0, 0));
                uppery = (int)floor(findyBound(c, b, x0, 1));
                
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, mInv, pq);
                    interpolateVary(sha->varyDim, a, b, c, pq, currentX, vary);
                    
                    sha->colorPixel(sha->unifDim, unif, sha->texNum, tex, sha->varyDim, vary, pointRGB);
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
        }
        
        //2nd case where the location "b" is to the left of "c"
        else {
            //1st loop from the leftmost point, a0, to b0
            for(x0 = (int)ceil(a[0]); x0 <= (int)floor(b[0]); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a, b, x0, 0));
                uppery = (int)floor(findyBound(a, c, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1){
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, mInv, pq);
                    interpolateVary(sha->varyDim, a, b, c, pq, currentX, vary);
                    
                    sha->colorPixel(sha->unifDim, unif, sha->texNum, tex, sha->varyDim, vary, pointRGB);
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
            
            //2nd loop from floor(b0) + 1 to c0
            for(x0 = (int)floor(b[0]) + 1; x0 <= (int)floor(c[0]); x0 = x0 + 1) {
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(b, c, x0, 0));
                uppery = (int)floor(findyBound(a, c, x0, 1));
                
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, mInv, pq);
                    interpolateVary(sha->varyDim, a, b, c, pq, currentX, vary);
                    
                    sha->colorPixel(sha->unifDim, unif, sha->texNum, tex, sha->varyDim, vary, pointRGB);
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
        }
    }
}
