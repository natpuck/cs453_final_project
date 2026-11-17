#pragma once
#include "polyhedron.h"
#include "drawUtil.h"
#include <iostream>
#include <unordered_map>

// singularity, that is
enum class SINGULARITY_TYPE
{
	SINK,
	SOURCE,
	SADDLE,
	FOCUS
};

enum class STREAMLINE_TYPE {
	USER,
	MAJOR_SEP,
	MINOR_SEP
};

struct Singularity
{
	icVector2 loc;
	SINGULARITY_TYPE type;
};

struct StreamLine {
	icVector2 start;
	PolyLine line;
	STREAMLINE_TYPE type;
};

class VectorTopology
{
private:

	Polyhedron* m_poly;
	std::vector<Singularity> singulars;

	std::vector<StreamLine> streamLines;

	icVector2 bound_arr[2];

	std::unordered_map<Quad*, icVector2> user_points;

	bool getSingularity(Quad* quad, Singularity& singularity);

	void getQuadValues(Quad* quad,
		double& x1, double& x2,
		double& y1, double& y2,
		icVector2& v11, icVector2& v12, icVector2& f21, icVector2& f22);

	std::vector<double> solveQuadratic(
		double a, double b, double c);

	Quad* getQuad(float x, float y);

	void findBounds();

		
public:
	VectorTopology(Polyhedron* poly);
	~VectorTopology();

	void calcSingulars();

	void drawSingulars();

	void drawStreamLines();

	bool addUserPoint(float, float);

	PolyLine calculateStreamLine(icVector2 startPoint, Quad* startQuad, int direction = 1, int numIter = 100, float stepSize = 0.5);

};