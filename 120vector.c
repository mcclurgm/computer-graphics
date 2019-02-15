

/*** In general dimensions
Michael McClurg and Vermilion Villarreal
***/

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


/*** In specific dimensions ***/

/* By the way, there is a way to write a single vecSet function that works in
all dimensions. The module stdarg.h, which is part of the C standard library,
lets you write variable-arity functions. The general vecSet would look like
	void vecSet(int dim, double a[], ...)
where the '...' represents dim numbers to be loaded into a. We're not going to
take this approach for two reasons. First, I try not to burden you with
learning a lot of C that isn't strictly necessary. Second, the variable-arity
feature is a bit dangerous, in that it provides no type checking. */

/* Copies three numbers into a three-dimensional vector. */
void vec3Set(double a0, double a1, double a2, double a[3]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
}

/* Copies four numbers into a four-dimensional vector. */
void vec4Set(double a0, double a1, double a2, double a3, double a[4]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	a[3] = a3;
}

/* Copies eight numbers into a eight-dimensional vector. */
void vec8Set(double a0, double a1, double a2, double a3, double a4, double a5,
		double a6, double a7, double a[8]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	a[3] = a3;
	a[4] = a4;
	a[5] = a5;
	a[6] = a6;
	a[7] = a7;
}

/* Returns the dot product of the vectors v and w. */
double vecDot(int dim, const double v[], const double w[]) {
    int i;
	double total = 0;
    for(i = 0; i < dim; i++) {
        total += v[i] * w[i];
    }

    return total;
}

/* Returns the length of the vector v. */
double vecLength(int dim, const double v[]) {
    double mag2 = vecDot(dim, v, v);
    return sqrt(mag2);
}

/* Returns the length of the vector v. If the length is non-zero, then also
places a normalized (length-1) version of v into unit. The output can safely
alias the input. */
double vecUnit(int dim, const double v[], double unit[]) {
    double mag = vecLength(dim, v);

    if(mag != 0.0) {
        int i;
        for(i = 0; i < dim; i++) {
            unit[i] = v[i] / mag;
        }
    }

    return mag;
}

/* Computes the cross product of v and w, and places it into vCrossW. The
output CANNOT safely alias the input. */
void vec3Cross(const double v[3], const double w[3], double vCrossW[3]) {
    vCrossW[0] = (v[1] * w[2]) - (v[2] * w[1]);
    vCrossW[1] = (v[2] * w[0]) - (v[0] * w[2]);
    vCrossW[2] = (v[0] * w[1]) - (v[1] * w[0]);
}

/* Computes the vector v from its spherical coordinates. rho >= 0.0 is the
radius. 0 <= phi <= pi is the co-latitude. -pi <= theta <= pi is the longitude
or azimuth. */
void vec3Spherical(double rho, double phi, double theta, double v[3]) {
    v[0] = rho * sin(phi) * cos(theta);
    v[1] = rho * sin(phi) * sin(theta);
    v[2] = rho * cos(phi);
}