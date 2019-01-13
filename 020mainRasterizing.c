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
#include "020triangle.c"

int main(void){
    if (pixInitialize(512, 512, "Triangle things!") != 0) {
        return 1;
    } else {
        pixClearRGB(0.0, 0.0, 0.0);
        
        //Triangles are created from bottom left to right then up in rows.
        triRender(10, 10, 50, 10, 30, 30, 1.0, 1.0, 0.0);
        triRender(60.4, 10.5, 80.8, 10.2, 100.1, 30, 0.0, 1.0, 0.0);
        triRender(100, 30, 100, 10, 140, 10, 1.0, 0.0, 1.0);
        triRender(160.1, 10.4, 200, 10.9, 200, 30, 1.0, 0.0, 1.0);
        triRender(220.1, 30, 260, 10, 240, 30, 1.0, 0.0, 0.0);
        triRender(280.8, 30.4, 300.5, 10, 320, 30, 0.0, 1.0, 1.0);
        triRender(320.7, 30.7, 360.4, 10.4, 340.5, 50, 1.0, 1.0, 1.0);
        triRender(380.4, 30.5, 400.9, 10.9, 420.2, 50.2, 1.0, 0.0, 0.0);
        triRender(440.4, 10.5, 480.5, 30.8, 460.3, 50.1, 0.0, 0.0, 1.0);
        triRender(700, 0, 800, 80, 730, 100, 1.0, 1.0, 0.0);
        triRender(20.4, 70.5, 40.5, 90.1, 60.2, 130.9, 0.0, 1.0, 1.0);
        triRender(80.4, 70.8, 100.4, 90.6, 120.2, 130.4, 1.0, 0.0, 1.0);
        triRender(150, 70, 150, 110, 150, 110, 1.0, 0.0, 1.0);
        triRender(155.2, 70, 155.2, 110, 155.2, 130.4, 1.0, 0.0, 1.0);
        triRender(160, 70, 180, 70, 180, 70, 1.0, 0.0, 1.0);
        
        //These last 3 "triangles" are interesting cases for horizontal lines,
        //vertical lines, and a single point.
        triRender(160, 70.8, 180, 70.8, 180, 70.8, 1.0, 0.0, 1.0);
        triRender(160, 70, 180, 90, 180, 90, 1.0, 0.0, 1.0);
        triRender(200, 70.2, 200, 70, 200, 70, 1.0, 0.0, 1.0);
//        triRender(80.4, 70.8, 100.4, 90.6, 120.2, 130.4, 1.0, 0.0, 1.0);
        //triRender(10, 10, 30, 10, 20, 20, 1.0, 1.0, 0.0);
        pixRun();
        return 0;
    }
}
