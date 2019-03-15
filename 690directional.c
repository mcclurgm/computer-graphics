


/* A simple directional light. No attenuation. */
typedef struct dirLight dirLight;
struct dirLight {
	lightClass *class;
	double dLight[3];
	double cLight[3];
};

lightResponse dirLighting(const void *light, const double world[3]) {
	const dirLight *dir = (const dirLight *)light;
	lightResponse result;
	vecCopy(3, dir->cLight, result.cLight);
	vecCopy(3, dir->dLight, result.dLight);
	result.distance = rayINFINITY;
	return result;
}

lightClass dirClass = {dirLighting};


