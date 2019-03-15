


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
		const rayResponse *response, int bodyNum, const void *bodies[], 
		int lightNum, const void *lights[], const double cAmbient[3], 
		double rgb[3]) {
    const plaPlane *plane = (const plaPlane *)body;
    
    rgb[0] = 0.0;
    rgb[1] = 0.0;
    rgb[2] = 0.0;

	/* x = e + t d. */
	double xWorld[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, xWorld);
	vecAdd(3, query->e, xWorld, xWorld);
	isoUntransformPoint(&(plane->isometry), xWorld, xLocal);

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
        
	rayQuery shadowQuery;
    for (int i = 0; i < lightNum; i++) {
        double rgbResult[3], dLightLocal[3];

		lightClass **class;
		class = (lightClass **)(lights[i]);
		lightResponse response = (*class)->lighting(lights[i], xWorld);

		shadowQuery.tStart = rayEPSILON;
		shadowQuery.tEnd = rayINFINITY;
		int index;
		rayResponse intersection = rayIntersection(bodyNum, bodies, &shadowQuery, &index);

        double distance = rayINFINITY + 1;
//             printf("distance %f lightResponse %f\n", distance, response.distance);
        if (intersection.intersected) {
            printf("Intersected!\n");
            double scaledD[3];
            vecScale(3, intersection.t, shadowQuery.d, scaledD);
            distance = vecLength(3, scaledD);
        }
        
        /* If body isn't closer, then the point isn't in shadow and is lit */
        if (distance >= response.distance) {
            isoUnrotateVector(&(plane->isometry), response.dLight, dLightLocal);
            
		    rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff, 
		    	cSpec, shininess, response.cLight, rgbResult);
            
            vecAdd(3, rgbResult, rgb, rgb);
        }
    }

	/* Ambient light. */
	rgb[0] += cDiff[0] * cAmbient[0];
	rgb[1] += cDiff[1] * cAmbient[1];
	rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};