#pragma once
#include "polyhedron.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <stdexcept>
#include <string>
#include "GL/freeglut.h"
#include <iostream>
#include <vector>
#include <algorithm>

// Image Based Flow Visualization (Van Wijk 2002)
//https://www.win.tue.nl/~vanwijk/ibfv/
class IBFV
{
private:

	Polyhedron* poly;
	int width, height;
	static const int npn = 64;
	float scale = 4.0;
	float alpha = 8.0;
	float tmax, dmax;
	unsigned char* pixels;
	GLuint noiseTexId;
	GLuint feedbackTexId;

	bool useSobel = false; // default to no
	int sobel_img_width, sobel_img_height;
	std::vector<icVector2> edgeFlow;

	float getXYVal(int x, int y, int width, std::vector<float>& vec);

public:

	IBFV(Polyhedron* p, int w, int h, char*);
	~IBFV();
	void freepixels();
	void initIBFV(int w, int h);
	void initIBFV(int w, int h, char*);
	void drawIBFV();

	void readSobelImage(char*);

};