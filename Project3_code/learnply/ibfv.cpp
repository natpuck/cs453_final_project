#include "ibfv.h"
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Image Based Flow Visualization (Van Wijk 2002)
//https://www.win.tue.nl/~vanwijk/ibfv/

IBFV::IBFV(Polyhedron* p, int w, int h, char* image_path)
{

	poly = p;

	// call without image - random black/white noise
	if (image_path == NULL) {
		initIBFV(w, h);
	}
	else {
		initIBFV(w, h, image_path);
	}
}

IBFV::~IBFV()
{
	poly = NULL;
	free(pixels);
}

void IBFV::freepixels()
{
	free(pixels);
}


void IBFV::initIBFV(int w, int h, char* image_path)
{

	width = w;
	height = h;
	pixels = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);

	memset(pixels, 255, sizeof(unsigned char) * width * height * 3);

	tmax = width / (scale * npn);
	dmax = scale / width;

	int im_width, im_height, im_chans;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* image = stbi_load(image_path, &im_width, &im_height, &im_chans, 4);

	if (!image) {
		std::runtime_error(std::string("could not load noise image"));
	}

	for (int i = 0; i < im_width * im_height; i++) {
		image[i * 4 + 3] = alpha;
	}

	glNewList(1, GL_COMPILE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, im_width, im_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glEndList();

	stbi_image_free(image);
}

float IBFV::getXYVal(int x, int y, int width, std::vector<float>& vec) {
	return vec[y * width + x];
};

void IBFV::readSobelImage(char* sobel_img_path) {

	useSobel = true;

	int im_chans;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* image = stbi_load(sobel_img_path, &sobel_img_width, &sobel_img_height, &im_chans, 3);

	if (!image) {
		std::runtime_error(std::string("could not load noise image"));
	}

	std::vector<float> gray(sobel_img_height * sobel_img_width);

	for (int y = 0; y < sobel_img_height; y++) {
		for (int x = 0; x < sobel_img_width; x++) {
			int i = (y * sobel_img_width + x) * 3;
			gray[y * sobel_img_width + x] = (0.299 * image[i] + 0.587 * image[i + 1] + 0.114 * image[i + 2]) / 255.0f;
		}
	}

	stbi_image_free(image);

	// calculate gradient per pixel
	edgeFlow.resize(sobel_img_width * sobel_img_height, icVector2(0, 0));

	std::vector<float> magnitude(sobel_img_height * sobel_img_width, 0.f);

	float maxMag = 0.0f;

	for (int y = 1; y < sobel_img_height - 1; y++) {

		for (int x = 1; x < sobel_img_width - 1; x++) {

			float gx = -getXYVal(x - 1, y - 1, sobel_img_width, gray) + getXYVal(x + 1, y - 1, sobel_img_width, gray)
				- 2 * getXYVal(x - 1, y, sobel_img_width, gray); +2 * getXYVal(x + 1, y, sobel_img_width, gray)
				-getXYVal(x - 1, y + 1, sobel_img_width, gray) + getXYVal(x + 1, y + 1, sobel_img_width, gray);

			float gy = -getXYVal(x - 1, y - 1, sobel_img_width, gray) - 2 * getXYVal(x, y - 1, sobel_img_width, gray)\
				- getXYVal(x + 1, y - 1, sobel_img_width, gray) + getXYVal(x - 1, y + 1, sobel_img_width, gray)\
				+ 2 * getXYVal(x, y + 1, sobel_img_width, gray) + getXYVal(x + 1, y + 1, sobel_img_width, gray);

			icVector2 dp(gx, -gy);

			float mag = sqrt(dp.x * dp.x + dp.y * dp.y);

			maxMag = std::max(maxMag, mag);

			normalize(dp);

			edgeFlow[y * sobel_img_width + x] = dp;
		}
	}

	std::cout << "Max Sobel Gradient " << maxMag << std::endl;
}


void IBFV::initIBFV(int w, int h)
{
	width = w;
	height = h;

	pixels = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
	memset(pixels, 255, sizeof(unsigned char) * width * height * 3);

	tmax = width / (scale * npn);
	dmax = scale / width;

	int lut[256];
	int phase[npn][npn];
	GLubyte pat[npn][npn][4];
	int i, j, k;

	for (i = 0; i < 256; i++) lut[i] = i < 127 ? 0 : 255;
	for (i = 0; i < npn; i++)
		for (j = 0; j < npn; j++) phase[i][j] = rand() % 256;

	for (i = 0; i < npn; i++)
	{
		for (j = 0; j < npn; j++)
		{
			pat[i][j][0] =
				pat[i][j][1] =
				pat[i][j][2] = lut[(phase[i][j]) % 255];
			pat[i][j][3] = alpha;
		}
	}

	//glBindTexture(GL_TEXTURE_2D, noiseTexId);
	glNewList(1, GL_COMPILE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, npn, npn, 0, GL_RGBA, GL_UNSIGNED_BYTE, pat);
	glEndList();
}

void IBFV::drawIBFV()
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1., 2.);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_BLEND);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.5, 0.5, 0.5, 1.0);  // background for rendering color coding and lighting
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw the mesh using pixels and use vector field to advect texture coordinates
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	double modelview_matrix[16], projection_matrix[16];
	int viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	// std::cout << "DRAWING w/ SOBEL? " << useSobel << std::endl;

	for (int i = 0; i < poly->nquads; i++)
	{
		Quad* qtemp = poly->qlist[i];

		glBegin(GL_QUADS);
		for (int j = 0; j < 4; j++)
		{
			Vertex* vtemp = qtemp->verts[j];

			double tx, ty, dummy;
			gluProject((GLdouble)vtemp->x, (GLdouble)vtemp->y, (GLdouble)vtemp->z,
				modelview_matrix, projection_matrix, viewport, &tx, &ty, &dummy);


			float px ;
			float py ;

			if (useSobel) {
				float s = (tx - viewport[0]) / viewport[2];
				float t = (ty - viewport[1]) / viewport[3];

				// use border vector to evect
				int ix = std::max(0, std::min(int(s * (sobel_img_width - 1)), sobel_img_width - 1));
				int iy = std::max(0, std::min(int((1.0f - t) * (sobel_img_height - 1)), sobel_img_height - 1));

				icVector2 dp = edgeFlow[iy * sobel_img_width + ix];

				dp *= dmax;

				double dx = dp.x;
				double dy = dp.y;

				px = s - dx;
				py = t - dy;
			}
			else {
				tx = tx / width;
				ty = ty / height;

				icVector2 dp = icVector2(vtemp->vx, vtemp->vy);
				normalize(dp);
				dp *= dmax;

				double dx = -dp.x;
				double dy = -dp.y;

				px = tx + dx;
				py = ty + dy;

			}

			glTexCoord2f(px, py);
			glVertex3d(vtemp->x, vtemp->y, vtemp->z);
		}
		glEnd();
	}

	glEnable(GL_BLEND);

	// blend in noise pattern
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(-1.0, -1.0, 0.0);
	glScalef(2.0, 2.0, 1.0);

	glCallList(1);

	glBegin(GL_QUAD_STRIP);

	glTexCoord2f(0.0, 0.0);  glVertex2f(0.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
	glTexCoord2f(1.0, 0.0);  glVertex2f(1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
	glEnd();
	glDisable(GL_BLEND);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	//
	// draw the mesh using pixels without advecting texture coords
	glClearColor(1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	for (int i = 0; i < poly->nquads; i++)
	{
		Quad* qtemp = poly->qlist[i];
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; j++)
		{
			Vertex* vtemp = qtemp->verts[j];
			double tx, ty, dummy;
			gluProject((GLdouble)vtemp->x, (GLdouble)vtemp->y, (GLdouble)vtemp->z,
				modelview_matrix, projection_matrix, viewport, &tx, &ty, &dummy);
			tx = tx / width;
			ty = ty / height;
			glTexCoord2f(tx, ty);
			glVertex3d(vtemp->x, vtemp->y, vtemp->z);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glShadeModel(GL_SMOOTH);
}