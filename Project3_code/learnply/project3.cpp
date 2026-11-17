#include "project3.h"


VectorTopology::VectorTopology(Polyhedron* poly) {
	
	m_poly = poly;
	findBounds();
	calcSingulars();

}

VectorTopology::~VectorTopology() {
	singulars.clear();
}



void VectorTopology::getQuadValues(Quad* quad,
	double& x1, double& x2,
	double& y1, double& y2,
	icVector2& v11, icVector2& v12, icVector2& v21, icVector2& v22)
{
	x1 = y1 = INFINITY;
	x2 = y2 = -INFINITY;
	for (int j = 0; j < 4; j++)
	{
		if (quad->verts[j]->x < x1)
			x1 = quad->verts[j]->x;
		if (quad->verts[j]->y < y1)
			y1 = quad->verts[j]->y;
		if (quad->verts[j]->x > x2)
			x2 = quad->verts[j]->x;
		if (quad->verts[j]->y > y2)
			y2 = quad->verts[j]->y;
	}
	for (int j = 0; j < 4; j++)
	{
		if (quad->verts[j]->x == x1 && quad->verts[j]->y == y1)
			v11 = icVector2(quad->verts[j]->vx, quad->verts[j]->vy);
		else if (quad->verts[j]->x == x1 && quad->verts[j]->y == y2)
			v12 = icVector2(quad->verts[j]->vx, quad->verts[j]->vy);
		else if (quad->verts[j]->x == x2 && quad->verts[j]->y == y1)
			v21 = icVector2(quad->verts[j]->vx, quad->verts[j]->vy);
		else
			v22 = icVector2(quad->verts[j]->vx, quad->verts[j]->vy);
	}
}


void VectorTopology::calcSingulars() {

	for (int i = 0; i < m_poly->nquads; i++) {
		
		Quad* quad = m_poly->qlist[i];

		Singularity singularity{};

		bool foundSing = getSingularity(quad, singularity);

		if (foundSing) {
			singulars.push_back(singularity);
		}
	}

	std::cout << "Number of singularities found: " << singulars.size() << std::endl;
}

void VectorTopology::findBounds() {
	bound_arr[0] = icVector2(INFINITY, INFINITY);
	bound_arr[1] = icVector2(-INFINITY, -INFINITY);

	for (int i = 0; i < m_poly->nverts; i++) {
		if (m_poly->vlist[i]->x < bound_arr[0].x) {
			bound_arr[0].x = m_poly->vlist[i]->x;
		}
		if (m_poly->vlist[i]->y < bound_arr[0].y) {
			bound_arr[0].y = m_poly->vlist[i]->y;
		}
		if (m_poly->vlist[i]->x > bound_arr[1].x) {
			bound_arr[1].x = m_poly->vlist[i]->x;
		}
		if (m_poly->vlist[i]->y > bound_arr[1].y) {
			bound_arr[1].y = m_poly->vlist[i]->y;
		}
	}

	printf("MIN (%f, %f) , MAX (%f, %f) ", bound_arr[0].x, bound_arr[0].y, bound_arr[1].x, bound_arr[1].y);
}

bool VectorTopology::addUserPoint(float x, float y) {

	Quad* q = getQuad(x, y);

	if (q == nullptr) {
		return false;
	}

	icVector2 point = icVector2(x, y);

	PolyLine polyL = calculateStreamLine(point, q);

	StreamLine sl;
	sl.start = point;
	sl.line = polyL;
	sl.type = STREAMLINE_TYPE::USER;

	streamLines.push_back(sl);

	polyL = calculateStreamLine(point, q, -1);

	printf("finished calculate streamline");

	StreamLine sl2;
	sl2.start = point;
	sl2.line = polyL;
	sl2.type = STREAMLINE_TYPE::USER;

	streamLines.push_back(sl2);


	return true;
}

bool findIntersection(icVector2 point, icVector2 vec, Edge* edge, icVector2& intersection) {
	icVector2 e0 = icVector2(edge->verts[0]->x, edge->verts[0]->y);
	icVector2 e1 = icVector2(edge->verts[1]->x, edge->verts[1]->y);

	icVector2 edgeDir = e1 - e0;

	double crossProd = dot(cross(vec), edgeDir);

	// paralell case
	if (crossProd == 0) {
		return false;
	}
	else {
		icVector2 relDir = e0 - point;

		float vec_t = dot(cross(relDir), edgeDir) / crossProd;
		float edge_t = dot(cross(relDir), vec) / crossProd;

		bool vecHit = vec_t > -EPS && vec_t < 1;

		bool edgeHit = edge_t > -EPS && edge_t < 1;

		if (vecHit && edgeHit) {
			intersection = vec * vec_t + point;
			return true;
		}
		else {
			return false;
		}
	}
}

PolyLine VectorTopology::calculateStreamLine(icVector2 startPoint, Quad* startQuad, int direction, int numIter, float stepSize) {

	Quad* currQuad = startQuad;
	icVector2 currPoint = startPoint;

	int currIter = 0;

	PolyLine streamLine;

	while (currIter < numIter) {
		icVector2 resultVec;

		double x1, x2, y1, y2;
		icVector2 v11, v12, v21, v22;

		getQuadValues(currQuad, x1, x2, y1, y2, v11, v12, v21, v22);

		resultVec = (1/ ((x2 - x1) * (y2 - y1) )) * ((x2 - currPoint.x) * (y2 - currPoint.y) * v11 + (currPoint.x - x1) * (y2 - currPoint.y) * v21 +
			(x2 - currPoint.x) * (currPoint.y - y1) * v12 + (currPoint.x - x1) * (currPoint.y - y1) * v22);
	
		normalize(resultVec);

		icVector2 newPoint = currPoint + (resultVec * stepSize * direction);

		if (newPoint.x < x2 && newPoint.y < y2 && newPoint.x >= x1 && newPoint.y >= y1) {
			LineSegment ls = LineSegment(icVector3(currPoint.x, currPoint.y, 0), icVector3(newPoint.x, newPoint.y, 0.f));
			
			streamLine.push_back(ls);

			currPoint = newPoint;
		}
		else {
			Edge* intersectEdge = nullptr;
			icVector2 intersection;

			for (int i = 0; i < 4; i++) {

				Edge* currEdge = currQuad->edges[i];

				bool edgeFound = findIntersection(currPoint, (resultVec * stepSize * direction), currEdge, intersection);

				if (edgeFound) {
					intersectEdge = currEdge;
					break;
				}
			}

			if (intersectEdge == nullptr) {
				throw std::logic_error("No edge intersection for Quad found");
			}

			LineSegment ls = LineSegment(icVector3(currPoint.x, currPoint.y, 0.f), icVector3(intersection.x, intersection.y, 0.f));
			streamLine.push_back(ls);

			currPoint = intersection + resultVec * 0.01 * direction;

			ls = LineSegment(icVector3(intersection.x, intersection.y, 0.f), icVector3(currPoint.x, currPoint.y, 0.f));
			streamLine.push_back(ls);

			if (intersectEdge && intersectEdge->nquads > 1) {
				currQuad = (currQuad == intersectEdge->quads[0]) ? intersectEdge->quads[1] : intersectEdge->quads[0];
			}
			else {
				break;
			}
		}

		currIter++;
	}

	return streamLine;

}

Quad* VectorTopology::getQuad(float x, float y) {

	if (x < (float)bound_arr[0].x || x > (float)bound_arr[1].x ||
		y < (float)bound_arr[0].y || y > (float)bound_arr[1].y) {

		return nullptr;
	}

	for (int i = 0; i < m_poly->nquads; i++) {

		Quad* quad = m_poly->qlist[i];

		double x1, x2, y1, y2;
		icVector2 v11, v12, v21, v22;

		getQuadValues(quad, x1, x2, y1, y2, v11, v12, v21, v22);

		if (x >= x1 && x < x2 && y >= y1 && y < y2) {

			printf("Within quad: %d", quad->index);
			return quad;
		}

	}

	return nullptr;
}

/*
* get singularity for single Quad
*
*/
bool VectorTopology::getSingularity(Quad* quad, Singularity& singularity) {

	double x1, x2, y1, y2;

	icVector2 v11, v12, v21, v22;

	getQuadValues(quad, x1, x2, y1, y2, v11, v12, v21, v22);

	double a_00 = v11.x;
	double a_10 = v21.x - v11.x;
	double a_01 = v12.x - v11.x;
	double a_11 = v11.x - v21.x - v12.x + v22.x;

	double b_00 = v11.y;
	double b_10 = v21.y - v11.y;
	double b_01 = v12.y - v11.y;
	double b_11 = v11.y - v21.y - v12.y + v22.y;

	double c_00 = (a_11 * b_00) * (a_00 * b_11);
	double c_10 = (a_11 * b_10) - (a_10 * b_11);
	double c_01 = (a_11 * b_01) - (a_01 * b_11);

	if (c_01 == 0) {
		return false;
	}

	double c = a_00 - a_01 * (c_00 / c_01);
	double b = (-a_01 * (c_10 / c_01) - a_11 * (c_00 / c_01) + a_10);
	double a = (-a_11 * (c_10 / c_01));

	std::vector<double> s_values = solveQuadratic(a, b, c);

	double dx = x2 - x1;
	double dy = y2 - y1;

	for (int i = 0; i < s_values.size(); i++) {
		double s = s_values[i];
		double t = -(c_00 + c_10 * s) / c_01;

		if (s < 1 && s >= 0 && t < 1 && t >= 0) {
			double x_target = x1 + s * (x2 - x1);
			double y_target = y1 + t * (y2 - y1);

			// calculate jacobian
			double j_00 = (a_10 + a_11 * t) / dx;
			double j_01 = (a_01 + a_11 * s) / dy;
			double j_10 = (b_10 + b_11 * t) / dx;
			double j_11 = (b_10 + b_11 * s) / dy;

			double determinant = j_00 * j_11 - j_01 * j_10;
			double trace = j_00 + j_11;
			double discriminant = trace * trace - 4.0 * determinant;

			singularity = Singularity();

			if (determinant < 0) {
				singularity.type = SINGULARITY_TYPE::SADDLE;
			}
			else {
				if (discriminant < 0) {
					singularity.type = SINGULARITY_TYPE::FOCUS;
				}
				else {
					if (trace > 0) {

					}
					else if (trace < 0) {
						singularity.type = SINGULARITY_TYPE::SINK;
					}
					else {
						std::cout << "Singularity with Jacobian trace of 0 found!" << std::endl;
					}
				}
			}

			singularity.loc = icVector2(x_target, y_target);

			return true;
		}
	}

	return false;
}


std::vector<double> VectorTopology::solveQuadratic(
	double a, double b, double c)
{
	const double eps = 1e-15;

	// case: linear equation
	if (std::abs(a) < eps) {
		if (std::abs(b) < eps)
			return {};

		return { -c / b };
	}

	double disc = b * b - 4.0 * a * c;
	//
	// no real sln
	if (disc < 0.0)
		return {};

	// repeated root
	if (disc == 0.0)
		return { -b / (2.0 * a) };

	double sqrtDisc = std::sqrt(disc);

	//quadratic formula
	double q = -0.5 * (b + std::copysign(sqrtDisc, b) );

	double x1 = q / a;
	double x2 = c / q;


	return { x1, x2 };
}

/*
 *
 * DRAWING METHODS
 *
 */
void VectorTopology::drawSingulars() {

	for (const Singularity& singular : singulars) {
		icVector2 loc = singular.loc;
		double z = 0;

		switch (singular.type) {
		case SINGULARITY_TYPE::SINK:
			// blue sinks
			drawSphere(loc.x, loc.y, z, 0.1, 0.0, 0.0, 1.0);
			break;
		case SINGULARITY_TYPE::SOURCE:
			// red sources
			drawSphere(loc.x, loc.y, z, 0.1, 1.0, 0.0, 0.0);
			break;
		case SINGULARITY_TYPE::SADDLE:
			// purple saddles
			drawSphere(loc.x, loc.y, z, 0.1, 0.5, 0.0, 1.0);
			break;
		case SINGULARITY_TYPE::FOCUS:
			// green focus
			drawSphere(loc.x, loc.y, z, 0.1, 0.0, 1.0, 0.0);
			break;
		default:
			break;
		}
	}

}

void VectorTopology::drawStreamLines() {
	if (!streamLines.empty()) {
		for (const StreamLine& sl : streamLines) {
			switch (sl.type) {
				case STREAMLINE_TYPE::USER: {
					drawPolyLine(sl.line, 1.25, 0.8, 0.0, 0.0);
				}
				break;
				case STREAMLINE_TYPE::MAJOR_SEP: {
					drawPolyLine(sl.line, 1.25, 1.0, 0.0, 0.5);
				}
				break;
				case STREAMLINE_TYPE::MINOR_SEP: {
					drawPolyLine(sl.line, 1.25, 1.0, 0.5, 0.0);
				}
				break;
			}
		 }
	}
}