


/* The sphere is centered on its local z-axis and has 
the prescribed radius. Its class should be set to sphereClass. */
typedef struct sphereSphere sphereSphere;
struct sphereSphere {
	rayClass *class;
	isoIsometry isometry;
	double radius;
	texTexture *texture;
};

rayResponse sphereIntersection(const void *body, const rayQuery *query) {
    const sphereSphere *sphere = (const sphereSphere *)body;
	rayResponse result;

	/* Note: center vector (called c) is the same as iso->translation. */
	/* Get quadform values */
	double a = pow(vecLength(3, query->d), 2);
	double eMinusC[3];
	vecSubtract(3, query->e, (sphere->isometry).translation, eMinusC);
	double b = 2 * vecDot(3, query->d, eMinusC);
	double c = pow(vecLength(3, eMinusC), 2) - (sphere->radius * sphere->radius);

	double descriminant = pow(b, 2) - 4 * a * c;
	if (descriminant <= 0) {
		result.intersected = 0;
		result.t = rayINFINITY;
	} else {
		double tMinus = (-b - sqrt(descriminant)) / (2 * a);
		if (tMinus >= query->tStart && tMinus <= query->tEnd) {
			result.intersected = -1;
			result.t = tMinus;
		} else {
			double tPlus = (-b + sqrt(descriminant)) / (2 * a);
			if (tPlus >= query->tStart && tPlus <= query->tEnd) {
				result.intersected = 1;
				result.t = tPlus;
			} else {
				result.intersected = 0;
				result.t = rayINFINITY;
			}
		}
	}

	return result;
}

void sphereTexCoords(const double xLocal[3], double st[2]) {
	/* Simply use cylindrical coordinates as texture coordinates. */
	double rho, phi, theta;
    vec3Rectangular(xLocal, &rho, &phi, &theta);
    st[0] = theta / (2 * M_PI);
    st[1] = phi / M_PI;
}

void sphereColor(const void *body, const rayQuery *query, 
		const rayResponse *response, int bodyNum, const void *bodies[], 
		int lightNum, const void *lights[], const double cAmbient[3], 
		int recursionNum, double rgb[3]) {
    const sphereSphere *sphere = (const sphereSphere *)body;
    
    rgb[0] = 0.0;
    rgb[1] = 0.0;
    rgb[2] = 0.0;

	/* x = e + t d. */
	double xWorld[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, xWorld);
	vecAdd(3, query->e, xWorld, xWorld);
	isoUntransformPoint(&(sphere->isometry), xWorld, xLocal);

    double texCoords[2];
    sphereTexCoords(xLocal, texCoords);
	double cDiff[sphere->texture->texelDim];
    texSample(sphere->texture, texCoords[0], texCoords[1], cDiff);

    double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
    
    /* Do lighting calculations in local coordinates. */
    double dNormalLocal[3];
    vecUnit(3, xLocal, dNormalLocal);

    double pCameraLocal[3], dCameraLocal[3];
    isoUntransformPoint(&(sphere->isometry), query->e, pCameraLocal);
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
		vecCopy(3, xWorld, shadowQuery.e);
		vecCopy(3, response.dLight, shadowQuery.d);
		int index;
		rayResponse intersection = rayIntersection(bodyNum, bodies, &shadowQuery, &index);

        double distance = rayINFINITY + 1;
        if (intersection.intersected) {
            double scaledD[3];
            vecScale(3, intersection.t, shadowQuery.d, scaledD);
            distance = vecLength(3, scaledD);
        }
        
        /* If body isn't closer, then the point isn't in shadow and is lit */
        if (distance >= response.distance) {
            isoUnrotateVector(&(sphere->isometry), response.dLight, dLightLocal);
            
            rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff, 
                cSpec, shininess, response.cLight, rgbResult);
            
            vecAdd(3, rgbResult, rgb, rgb);
        }
    }

	/* Mirror contribution */
	if (recursionNum > 0) {
		double rgbResult[3];

		rayQuery mirrorQuery;
		mirrorQuery.tStart = rayEPSILON;
		mirrorQuery.tEnd = rayINFINITY;
		vecCopy(3, xWorld, mirrorQuery.e);
		double twiceDot, dRefl[3];
		twiceDot = 2.0 * vecDot(3, dNormalLocal, dCameraLocal);
		vecScale(3, twiceDot, dNormalLocal, dRefl);
		vecSubtract(3, dRefl, dCameraLocal, dRefl);
		isoRotateVector(&(sphere)->isometry, dRefl, mirrorQuery.d);

		rayColor(bodyNum, bodies, lightNum, lights, cAmbient, &mirrorQuery, recursionNum - 1, rgbResult);
		rgb[0] += rgbResult[0] * cSpec[0];
		rgb[1] += rgbResult[1] * cSpec[1];
		rgb[2] += rgbResult[2] * cSpec[2];
	}
	
	/* Ambient light. */
	rgb[0] += cDiff[0] * cAmbient[0];
	rgb[1] += cDiff[1] * cAmbient[1];
	rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass sphereClass = {sphereIntersection, sphereColor};