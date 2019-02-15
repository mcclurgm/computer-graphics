

/*** 2 x 2 Matrices
Michael McClurg and Vermilion Villarreal
***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const GLdouble m[2][2]) {
	int i, j;
	for (i = 0; i < 2; i += 1) {
		for (j = 0; j < 2; j += 1)
			printf("%f    ", m[i][j]);
		printf("\n");
	}
}

GLdouble determinant2by2(const GLdouble m[2][2]) {
	GLdouble det = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
	return det;
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0,
then the matrix is invertible, and its inverse is placed into mInv. The output
CANNOT safely alias the input. */
GLdouble mat22Invert(const GLdouble m[2][2], GLdouble mInv[2][2]) {
	GLdouble det = determinant2by2(m);

	mInv[0][0] = m[1][1] / det;
	mInv[0][1] = -1 * m[0][1] / det;
	mInv[1][0] = -1 * m[1][0] / det;
	mInv[1][1] = m[0][0] / det;

	return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV.
The output CANNOT safely alias the input. */
void mat221Multiply(const GLdouble m[2][2], const GLdouble v[2],
		GLdouble mTimesV[2]) {
	mTimesV[0] = (m[0][0] * v[0]) + (m[0][1] * v[1]);
	mTimesV[1] = (m[1][0] * v[0]) + (m[1][1] * v[1]);
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the
input. */
void mat22Columns(const GLdouble col0[2], const GLdouble col1[2], GLdouble m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the
rotation matrix corresponding to counterclockwise rotation of the plane through
the angle theta. */
void mat22Rotation(GLdouble theta, GLdouble m[2][2]) {
	GLdouble cosTh, sinTh;
	cosTh = cos(theta);
	sinTh = sin(theta);

	m[0][0] = cosTh;
	m[0][1] = -sinTh;
	m[1][0] = sinTh;
	m[1][1] = cosTh;
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely
alias the input. */
void mat333Multiply(const GLdouble m[3][3], const GLdouble n[3][3],
		GLdouble mTimesN[3][3]) {
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
void mat331Multiply(const GLdouble m[3][3], const GLdouble v[3],
		GLdouble mTimesV[3]) {
    mTimesV[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
    mTimesV[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2];
    mTimesV[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2];
}

/* Multiplies the transpose of the 3x3 matrix m by the 3x1 matrix v. To
 clarify, in math notation it computes M^T v. The output CANNOT safely alias the
 input. */
void mat331TransposeMultiply(const GLdouble m[3][3], const GLdouble v[3],
                             GLdouble mTTimesV[3]) {
    mTTimesV[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2];
    mTTimesV[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2];
    mTTimesV[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2];
}

/* Fills the matrix m from its three columns. The output CANNOT safely alias the
input. */
void mat33Columns(const GLdouble col0[3], const GLdouble col1[3], const GLdouble col2[3],
        GLdouble m[3][3]) {
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

void mat33Add(const GLdouble m[3][3], const GLdouble n[3][3], GLdouble mPlusN[3][3]) {
    int i, j;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            mPlusN[i][j] = m[i][j] + n[i][j];
        }
    }
}

void mat33Identity(GLdouble identity[3][3]) {
    int i, j;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            if(i == j) {
                identity[i][j] = 1;
            } else {
                identity[i][j] = 0;
            }
        }
    }
}

void mat33Scale(GLdouble scalar, const GLdouble m[3][3], GLdouble scaledM[3][3]) {
    int i, j;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            scaledM[i][j] = scalar * m[i][j];
        }
    }
}

void mat33Transpose(const GLdouble m[3][3], GLdouble transpose[3][3]) {
    int i, j;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            transpose[i][j] = m[j][i];
        }
    }
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous
coordinates. More precisely, the transformation first rotates through the angle
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(GLdouble theta, const GLdouble t[2], GLdouble isom[3][3]) {
    GLdouble col0[3] = {cos(theta), sin(theta), 0};
    GLdouble col1[3] = {-sin(theta), cos(theta), 0};
    GLdouble col2[3] = {t[0], t[1], 1};
    mat33Columns(col0, col1, col2, isom);
}

/* Given a length-1 3D vector axis (u) and an angle theta (in radians), builds the
rotation matrix for the rotation about that axis through that angle. */
void mat33AngleAxisRotation(GLdouble theta, const GLdouble axis[3],
		GLdouble rot[3][3]) {
	GLdouble u[3][3], u2[3][3], identity[3][3], scaledU[3][3], scaledU2[3][3], intermediate[3][3];
	GLdouble col0[3] = {0, axis[2], -axis[1]};
	GLdouble col1[3] = {-axis[2], 0, axis[0]};
	GLdouble col2[3] = {axis[1], -axis[0], 0};
	mat33Columns(col0, col1, col2, u);

	mat333Multiply(u, u, u2);

	mat33Identity(identity);
	mat33Scale(sin(theta), u, scaledU);
	mat33Scale(1 - cos(theta), u2, scaledU2);

	mat33Add(scaledU, scaledU2, intermediate);
	mat33Add(identity, intermediate, rot);
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other.
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(const GLdouble u[3], const GLdouble v[3],
		const GLdouble a[3], const GLdouble b[3], GLdouble rot[3][3]) {
	GLdouble w[3], r[3][3], rt[3][3], c[3], s[3][3];

	vec3Cross(u, v, w);
	mat33Columns(u, v, w, r);

	vec3Cross(a, b, c);
    mat33Columns(a, b, c, s);

    mat33Transpose(r, rt);
    mat333Multiply(s, rt, rot);
}

/* Multiplies m by n, placing the answer in mTimesN. The output CANNOT safely
alias the input. */
void mat444Multiply(const GLdouble m[4][4], const GLdouble n[4][4],
		GLdouble mTimesN[4][4]) {
	int i, j, k;
	for(i = 0; i < 4; i++){
	    for(j = 0; j < 4; j++){
	        GLdouble sum = 0;
	        for(k = 0; k < 4; k++){
	            sum += m[i][k] * n[k][j];
	        }
	        mTimesN[i][j] = sum;
	    }
	}
}

/* Multiplies m by v, placing the answer in mTimesV. The output CANNOT safely
alias the input. */
void mat441Multiply(const GLdouble m[4][4], const GLdouble v[4],
		GLdouble mTimesV[4]) {
	mTimesV[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3];
	mTimesV[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3];
	mTimesV[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3];
	mTimesV[3] = m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3];
}

/* Fills the matrix m from its four columns. The output CANNOT safely alias the
input. */
void mat44Columns(const GLdouble col0[4], const GLdouble col1[4], const GLdouble col2[4],
        const GLdouble col3[4], GLdouble m[4][4]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[2][0] = col0[2];
    m[3][0] = col0[3];

	m[0][1] = col1[0];
	m[1][1] = col1[1];
	m[2][1] = col1[2];
	m[3][1] = col1[3];

	m[0][2] = col2[0];
	m[1][2] = col2[1];
	m[2][2] = col2[2];
	m[3][2] = col2[3];

	m[0][3] = col3[0];
	m[1][3] = col3[1];
	m[2][3] = col3[2];
	m[3][3] = col3[3];
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix
representing the rotation followed in time by the translation. */
void mat44Isometry(const GLdouble rot[3][3], const GLdouble trans[3],
		GLdouble isom[4][4]) {
	GLdouble col0[4] = {rot[0][0], rot[1][0], rot[2][0], 0};
	GLdouble col1[4] = {rot[0][1], rot[1][1], rot[2][1], 0};
	GLdouble col2[4] = {rot[0][2], rot[1][2], rot[2][2], 0};
	GLdouble col3[4] = {trans[0], trans[1], trans[2], 1};

	mat44Columns(col0, col1, col2, col3, isom);
}

/* Sets its argument to the 4x4 zero matrix (which consists entirely of 0s). */
void mat44Zero(GLdouble m[4][4]) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            m[i][j] = 0;
        }
    }
}

/* Builds a 4x4 matrix for a viewport with lower left (0, 0) and upper right
(width, height). This matrix maps a projected viewing volume
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w] x [0, h] x [0, 1] (each interval
in that order). */
void mat44Viewport(GLdouble width, GLdouble height, GLdouble view[4][4]) {
  mat44Zero(view);

  view[0][0] = width / 2;
  view[1][1] = height / 2;
  view[2][2] = 0.5;
  view[0][3] = width / 2;
  view[1][3] = height / 2;
  view[2][3] = 0.5;
  view[3][3] = 1.0;
}

/* Inverse to mat44Viewport. */
void mat44InverseViewport(GLdouble width, GLdouble height, GLdouble view[4][4]) {
  mat44Zero(view);

  view[0][0] = 2 / width;
  view[1][1] = 2 / height;
  view[2][2] = 2;
  view[0][3] = -1;
  view[1][3] = -1;
  view[2][3] = -1;
  view[3][3] = 1.0;
}
