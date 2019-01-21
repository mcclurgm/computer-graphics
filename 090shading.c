typedef struct shaShading shaShading;

struct shaShading {
    int unifDim;
    int texNum;
    int attrDim;
    int varyDim;
    void (*colorPixel)(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[3]);
	void (*transformVertex)(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]);
};