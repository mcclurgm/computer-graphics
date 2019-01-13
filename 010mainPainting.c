/* 010mainPainting.c
Michael McClurg
For homework due 1/8/2018
*/

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

/* Define global variables.
These represent the state of the program: which color to draw,
whether the mouse button is down.
They should be modified by the key press or mouse click handlers.
mouseDown controls whether to paint or not.
If it is <= 0, nothing will be drawn.
Otherwise, the program will draw the color represented by the RGB combo.
*/
double red, green, blue;
int mouseDown;

/* Draw pixels.
This function includes some logic to check whether it should draw at all.
It also takes the double mouse location and converts it to an int for pixSetRGB.
*/
void paintPixel(double x, double y) {
	printf("paintPixel: mouseDown %i; x %f, y %f; red %f, green %f, blue %f\n",
			mouseDown, x, y, red, green, blue);
	if (mouseDown > 0) {
		int intX, intY;
		intX = floor(x);
		intY = floor(y);
		pixSetRGB(intX, intY, red, green, blue);
	}
}

// When mouse is clicked, store this information in the mouseDown variable.
// This also draws at the current pixel.
void handleMouseDown(double x, double y, int button, int shiftIsDown,
		int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
	printf("handleMouseDown: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		button, shiftIsDown, controlIsDown, altOptionIsDown,
		superCommandIsDown);

	mouseDown = 1;
	paintPixel(x, y);
}

// When mouse is released, reset mouseDown variable to stop drawing
void handleMouseUp(double x, double y, int button, int shiftIsDown,
		int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
	printf("handleMouseUp: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		button, shiftIsDown, controlIsDown, altOptionIsDown,
		superCommandIsDown);

	mouseDown = 0;
}

// If mouse is down, draw at its current location
void handleMouseMove(double x, double y) {
	printf("handleMouseMove: x %f, y %f\n", x, y);
	paintPixel(x, y);
}

// Set the color or account for special keyboard-based features
void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	printf("handleKeyDown: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		key, shiftIsDown, controlIsDown, altOptionIsDown, superCommandIsDown);

	// Ignore keystrokes if modifier keys are pressed
	if(shiftIsDown == 1 || controlIsDown == 2 || altOptionIsDown == 4 || superCommandIsDown == 8) {
		return;
	}

	switch(key) {
		case 82: // r, red
			red = 1.0;
			green = 0.0;
			blue = 0.0;
			break;
		case 71: // g, green
			red = 0.0;
			green = 1.0;
			blue = 0.0;
			break;
		case 66: // b, blue
			red = 0.0;
			green = 0.0;
			blue = 1.0;
			break;
		case 77: // m, magenta
			red = 1.0;
			green = 0.0;
			blue = 1.0;
			break;
		case 67: // c, cyan
			red = 0.0;
			green = 1.0;
			blue = 1.0;
			break;
		case 89: // y, yellow
			red = 1.0;
			green = 1.0;
			blue = 0.0;
			break;
		case 69: // e, erase
		case 75: // k, black (key)
			red = 0.0;
			green = 0.0;
			blue = 0.0;
			break;
		case 87: // w, white
			red = 1.0;
			green = 1.0;
			blue = 1.0;
			break;
		case 261: // del, clear screen
			pixClearRGB(0.0, 0.0, 0.0);
	}
}

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		// Initialize global variables
		red = 1.0;
		blue = 1.0;
		green = 1.0;
		mouseDown = 0;

		/* Register the callbacks (defined above) with the user interface, so
		that they are called as needed during pixRun (invoked below). */
		pixSetMouseDownHandler(handleMouseDown);
		pixSetMouseUpHandler(handleMouseUp);
		pixSetMouseMoveHandler(handleMouseMove);
		pixSetKeyDownHandler(handleKeyDown);

		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);

		/* Run the event loop. The callbacks that were registered above are
		invoked as needed. At the end, the resources supporting the window are
		deallocated. */
		pixRun();
		return 0;
	}
}
