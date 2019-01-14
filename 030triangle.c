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
void findPQ(const double x[2], const double a[2], const double b[2], 
        const double c[2], double pq[2]) {
    double bMinusA[2], cMinusA[2], m[2][2], det;
    vecSubtract(2, b, a, bMinusA);
    vecSubtract(2, c, a, cMinusA);
    
    det = determinant2by2(m);
    if(det > 0) {
        //  Create matrix m from its columns
        double xMinusA[2];
        vecSubtract(2, x, a, xMinusA);

        double mInv[2][2];
        mat22Columns(bMinusA, cMinusA, m);
        mat22Invert(m, mInv);
        
        mat221Multiply(mInv, xMinusA, pq);
    }
    
}

/* Calculates the interpolated and modulated color to use at a point.
It takes its position element from the pq vector.
*/
void findRGB(const double alpha[3], const double beta[3], 
        const double gamma[3], const double pq[2], const double rgb[3],
        double finalRGB[3]) {
    double betaMinusAlpha[3], gammaMinusAlpha[3], scaledP[3], scaledQ[3], pPlusQ[3];
    
    vecSubtract(3, beta, alpha, betaMinusAlpha);
    vecSubtract(3, gamma, alpha, gammaMinusAlpha);
    
    vecScale(3, pq[0], betaMinusAlpha, scaledP);
    vecScale(3, pq[1], gammaMinusAlpha, scaledQ);
    
    vecAdd(3, scaledP, scaledQ, pPlusQ);
    vecAdd(3, alpha, pPlusQ, finalRGB);
    
    finalRGB[0] = finalRGB[0] * rgb[0];
    finalRGB[1] = finalRGB[1] * rgb[1];
    finalRGB[2] = finalRGB[2] * rgb[2];
}

/*triRender takes 3 points on the initialized graphics and creates a triangle with the given RGB
  value. All triangles must be counterclockwise and are then formed into one of two base cases before
  being drawn. */
void triRender(const double a[2], const double b[2], const double c[2],
               const double rgb[3], const double alpha[3], const double beta[3],
               const double gamma[3]) {
    //Check if the location in position "a" is the left most point, if not recall triRender in a
    //different order.
    if(b[0] < a[0] || c[0] < a[0]) {
        triRender(b, c, a, rgb, beta, gamma, alpha);
    } else {
        int x0;
        int lowery;
        int uppery;

        double pq[2], pointRGB[3];
        
        //1st case where the location "b" is to the right of "c"
        if(b[0] > c[0]) {
            printf("Case 1\n");
            //1st loop from the leftmost point, "a", until we get to c0
            for(x0 = (int)ceil(a[0]); x0 <= (int)floor(c[0]); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a, b, x0, 0));
                uppery = (int)floor(findyBound(a, c, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, b, c, pq);
                    findRGB(alpha, beta, gamma, pq, rgb, pointRGB);
                    
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
                    findPQ(currentX, a, b, c, pq);
                    findRGB(alpha, beta, gamma, pq, rgb, pointRGB);
                    
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
        }
        
        //2nd case where the location "b" is to the left of "c"
        else {
            printf("Case 2\n");

            //1st loop from the leftmost point, a0, to b0
            for(x0 = (int)ceil(a[0]); x0 <= (int)floor(b[0]); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a, b, x0, 0));
                uppery = (int)floor(findyBound(a, c, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1){
                    double currentX[2] = {(double)x0, (double)y};
                    findPQ(currentX, a, b, c, pq);
                    findRGB(alpha, beta, gamma, pq, rgb, pointRGB);
                    
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
                    findPQ(currentX, a, b, c, pq);
                    findRGB(alpha, beta, gamma, pq, rgb, pointRGB);
                    
                    pixSetRGB(x0, y, pointRGB[0], pointRGB[1], pointRGB[2]);
                }
            }
        }
    }
}
