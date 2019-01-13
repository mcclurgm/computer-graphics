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
double findyBound(double a0, double a1, double b0, double b1, int x0, int upper){
    if(b0 == a0) {
        if(upper == 1)
            if(a1 > b1)
                return a1;
            else
                return b1;
        else
            if(a1 > b1)
                return b1;
            else
                return a1;
    }

    double m = (b1 - a1) / (b0 - a0);
    double y = m * ((double)x0 - a0) + a1;
    return y;
}
/*triRender takes 3 points on the initialized graphics and creates a triangle with the given RGB
  value. All triangles must be counterclockwise and are then formed into one of two base cases before
  being drawn. */
void triRender(const double a0, const double a1, const double b0,
               const double b1, const double c0, const double c1, const double r,
               const double g, const double b) {
    //Check if the location in position "a" is the left most point, if not recall triRender in a
    //different order.
    if(b0 < a0 || c0 < a0) {
        triRender(b0, b1, c0, c1, a0, a1, r, g, b);
    } else {
        int x0;
        int lowery;
        int uppery;
        
        //1st case where the location "b" is to the right of "c"
        if(b0 > c0) {
            //1st loop from the leftmost point, "a", until we get to c0
            for(x0 = (int)ceil(a0); x0 <= (int)floor(c0); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a0, a1, b0, b1, x0, 0));
                uppery = (int)floor(findyBound(a0, a1, c0, c1, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1){
                    pixSetRGB(x0, y, r, g, b);
                }
            }
            
            //2nd loop from floor(c0) + 1 until we get to b0
            for(x0 = (int)floor(c0) + 1; x0 <= (int)floor(b0); x0 = x0 + 1) {
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a0, a1, b0, b1, x0, 0));
                uppery = (int)floor(findyBound(c0, c1, b0, b1, x0, 1));
                
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    pixSetRGB(x0, y, r, g, b);
                }
            }
        }
        
        //2nd case where the location "b" is to the left of "c"
        else {
            //1st loop from the leftmost point, a0, to b0
            for(x0 = (int)ceil(a0); x0 <= (int)floor(b0); x0 = x0 + 1){
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(a0, a1, b0, b1, x0, 0));
                uppery = (int)floor(findyBound(a0, a1, c0, c1, x0, 1));
                int y;
                for(y = lowery; y <= uppery; y = y + 1){
                    //            printf("x0: %i, y: %i\n", x0, y);
                    pixSetRGB(x0, y, r, g, b);
                }
            }
            
            //2nd loop from floor(b0) + 1 to c0
            for(x0 = (int)floor(b0) + 1; x0 <= (int)floor(c0); x0 = x0 + 1) {
                //find upper and lower values of y and then fill in all pixels between them
                lowery = (int)ceil(findyBound(b0, b1, c0, c1, x0, 0));
                uppery = (int)floor(findyBound(a0, a1, c0, c1, x0, 1));
                
                int y;
                for(y = lowery; y <= uppery; y = y + 1) {
                    //            printf("x0: %i, y: %i\n", x0, y);
                    pixSetRGB(x0, y, r, g, b);
                }
            }
        }
    }
}
