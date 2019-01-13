

/*** In general dimensions ***/

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. */
void vecCopy(int dim, const double v[], double copy[]) {
	int i;
	for(i = 0; i < dim; i++) {
		copy[i] = v[i];
	}
}

/* Adds the dim-dimensional vectors v and w. */
void vecAdd(int dim, const double v[], const double w[], double vPlusW[]) {
	int i;
	for(i = 0; i < dim; i++) {
		vPlusW[i] = v[i] + w[i];
	}
}

/* Subtracts the dim-dimensional vectors v and w. */
void vecSubtract(int dim, const double v[], const double w[],
		double vMinusW[]) {
	int i;
	for(i = 0; i < dim; i++) {
		vMinusW[i] = v[i] - w[i];
	}
}

/* Scales the dim-dimensional vector w by the number c. */
void vecScale(int dim, double c, const double w[], double cTimesW[]) {
	int i;
	for(i = 0; i < dim; i++) {
		cTimesW[i] = c * w[i];
	}
}


// int main(void) {
// 	double thing[2] = {1,2};
// 	double c[2];

// 	vecCopy(2, thing, c);

// 	for(int i = 0; i < 2; i++) {
// 		printf("c    : %f\n", thing[i]);
// 		printf("thing: %f\n", c[i]);
// 	}

// 	printf("original vector thing:\n");
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", thing[i]);
// 	}
	
// 	printf("original vector c:\n");
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", c[i]);
// 	}

// 	printf("Adding thing and c\n");
// 	double added[2];
// 	vecAdd(2, thing, c, added);
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", added[i]);
// 	}

// 	printf("Subtracting thing and c\n");
// 	double subbed[2];
// 	vecSubtract(2, thing, c, subbed);
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", subbed[i]);
// 	}

// 	printf("Scaling thing by 2\n");
// 	double scaled[2];
// 	vecScale(2, 2, thing, scaled);
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", scaled[i]);
// 	}

// 	printf("final vector thing:\n");
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", thing[i]);
// 	}
	
// 	printf("final vector c:\n");
// 	for(int i = 0; i < 2; i++) {
// 		printf("%f\n", c[i]);
// 	}
// }
