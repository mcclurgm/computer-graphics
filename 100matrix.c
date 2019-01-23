

/*** 2 x 2 Matrices 
Michael McClurg and Vermilion Villarreal
***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const double m[2][2]) {
	int i, j;
	for (i = 0; i < 2; i += 1) {
		for (j = 0; j < 2; j += 1)
			printf("%f    ", m[i][j]);
		printf("\n");
	}
}

double determinant2by2(const double m[2][2]) {
	double det = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
	return det;
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. The output 
CANNOT safely alias the input. */
double mat22Invert(const double m[2][2], double mInv[2][2]) {
	double det = determinant2by2(m);

	mInv[0][0] = m[1][1] / det;
	mInv[0][1] = -1 * m[0][1] / det;
	mInv[1][0] = -1 * m[1][0] / det;
	mInv[1][1] = m[0][0] / det;
	
	return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. 
The output CANNOT safely alias the input. */
void mat221Multiply(const double m[2][2], const double v[2], 
		double mTimesV[2]) {
	mTimesV[0] = (m[0][0] * v[0]) + (m[0][1] * v[1]);
	mTimesV[1] = (m[1][0] * v[0]) + (m[1][1] * v[1]);
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the 
input. */
void mat22Columns(const double col0[2], const double col1[2], double m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the 
rotation matrix corresponding to counterclockwise rotation of the plane through 
the angle theta. */
void mat22Rotation(double theta, double m[2][2]) {
	double cosTh, sinTh;
	cosTh = cos(theta);
	sinTh = sin(theta);

	m[0][0] = cosTh;
	m[0][1] = -sinTh;
	m[1][0] = sinTh;
	m[1][1] = cosTh;
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely 
alias the input. */
void mat333Multiply(const double m[3][3], const double n[3][3], 
		double mTimesN[3][3]) {
    mTimesN[0][0] = m[0][0] * n[0][0] + m[0][1] * n[1][0] + m[0][2] * n[2][0];
    mTimesN[0][1] = m[0][0] * n[0][1] + m[0][1] * n[1][1] + m[0][2] * n[2][1];
    mTimesN[0][2] = m[0][0] * n[0][2] + m[0][1] * n[1][2] + m[0][2] * n[2][2];
    
    mTimesN[1][0] = m[1][0] * n[0][0] + m[1][1] * n[1][0] + m[1][2] * n[2][0];
    mTimesN[1][1] = m[1][0] * n[0][1] + m[1][1] * n[1][1] + m[1][2] * n[2][1];
    mTimesN[1][2] = m[1][0] * n[0][2] + m[1][1] * n[1][2] + m[1][2] * n[2][2];
    
    mTimesN[2][0] = m[2][0] * n[0][0] + m[2][1] * n[1][0] + m[2][2] * n[2][0];
    mTimesN[2][1] = m[2][0] * n[0][1] + m[2][1] * n[1][1] + m[2][2] * n[2][1];
    mTimesN[2][2] = m[2][0] * n[0][2] + m[2][1] * n[1][2] + m[2][2] * n[2][2];

}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. The output CANNOT safely 
alias the input. */
void mat331Multiply(const double m[3][3], const double v[3], 
		double mTimesV[3]) {
    mTimesV[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
    mTimesV[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2];
    mTimesV[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2];

}


/* Fills the matrix m from its three columns. The output CANNOT safely alias the 
input. */
void mat33Columns(const double col0[3], const double col1[3], const double col2[3],
        double m[3][3]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[2][0] = col0[2];
	
	m[0][1] = col1[0];
	m[1][1] = col1[1];
	m[2][1] = col1[2];
	
	m[0][2] = col2[0];
	m[1][2] = col2[1];
	m[2][2] = col2[2];
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(double theta, const double t[2], double isom[3][3]) {
    double col0[3] = {cos(theta), sin(theta), 0}; 
    double col1[3] = {-sin(theta), cos(theta), 0};
    double col2[3] = {t[0], t[1], 1};
    mat33Columns(col0, col1, col2, isom);
}