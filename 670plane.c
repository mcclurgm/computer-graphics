


typedef struct plaPlane plaPlane;
struct plaPlane {
	rayClass *class;
	isoIsometry isometry;
	texTexture *texture;
};

rayResponse planeIntersection(const void *body, const rayQuery *query) {
    const plaPlane *plane = (const plaPlane *)body;
	rayResponse result;

	double d[3], e[3];
	isoUnrotateVector(&(plane->isometry), query->d, d);
	isoUntransformPoint(&(plane->isometry), query->e, e);

	if (d[2] == 0) {
		result.intersected = 0;
		result.t = rayINFINITY;
		return result;
	}

	double t = (-1.0 * e[2]) / d[2];
	/* Compare this result with valid t interval */
	if (t <= query->tStart || t >= query->tEnd) {
		result.intersected = 0;
		result.t = t;
		return result;
	}
	
	/* Otherwise: it intersects, now determine which direction */
	if (d[2] > 0) {
		result.intersected = 1;
		result.t = t;
		return result;
	} else {
		result.intersected = -1;
		result.t = t;
		return result;
	}

	result.intersected = 0;
	return result;
}

void planeTexCoords(const double xLocal[3], double st[2]) {
    st[0] = xLocal[0];
    st[1] = xLocal[1];
}

void planeColor(const void *body, const rayQuery *query, 
		const rayResponse *response, double rgb[3]) {
    const plaPlane *plane = (const plaPlane *)body;

	/* x = e + t d. */
	double x[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, x);
	vecAdd(3, query->e, x, x);
	isoUntransformPoint(&(plane->isometry), x, xLocal);

    double texCoords[2];
    planeTexCoords(xLocal, texCoords);
	double cDiff[plane->texture->texelDim];
    texSample(plane->texture, texCoords[0], texCoords[1], cDiff);

    double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
    /* Do lighting calculations in local coordinates. */
    double dNormalLocal[3] = {0.0, 0.0, 1.0}, dLightLocal[3];
    vecUnit(3, dNormalLocal, dNormalLocal);
    isoUnrotateVector(&(plane->isometry), dLight, dLightLocal);
    double pCameraLocal[3], dCameraLocal[3];
    isoUntransformPoint(&(plane->isometry), query->e, pCameraLocal);
    vecSubtract(3, pCameraLocal, xLocal, dCameraLocal);
    vecUnit(3, dCameraLocal, dCameraLocal);
    diffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff, 
        cSpec, shininess, cLight, rgb);

	/* Ambient light. */
	rgb[0] += cDiff[0] * cAmbient[0];
	rgb[1] += cDiff[1] * cAmbient[1];
	rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};