#include "curve.h"
#include "vertexrecorder.h"
#include "cmath"
using namespace std;

const float c_pi = 3.14159265358979323846f;
const Matrix4f BERNSTEIN_MATRIX = Matrix4f(Vector4f(1, 0, 0, 0),
	Vector4f(-3, 3, 0, 0),
	Vector4f(3, -6, 3, 0),
	Vector4f(-1, 3, -3, 1));

const Matrix4f HERMITE_MATRIX = Matrix4f(Vector4f(2, -3, 0, 1),
	Vector4f(-2, 3, 0, 0),
	Vector4f(1, -2, 1, 0),
	Vector4f(1, -1, 0, 0));

const Matrix4f B_SPLINE_BASIS = Matrix4f(
	1.0f, 4.0f, 1.0f, 0.0f,
	-3.0f, 0.0f, 3.0f, 0.0f,
	3.0f, -6.0f, 3.0f, 0.0f,
	-1.0f, 3.0f, -3.0f, 1.0f
).transposed() * (1.0f / 6.0f);

namespace
{
	// Approximately equal to.  We don't want to use == because of
	// precision issues with floating point.
	inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
	{
		const float eps = 1e-8f;
		return (lhs - rhs).absSquared() < eps;
	}

	inline float binomialCoeficient(size_t n, size_t k) {
		if (k == 0 || k == n)
		{
			return 1;
		}

		return binomialCoeficient(n - 1, k - 1) + binomialCoeficient(n - 1, k);
	}
}

Vector3f getB(Vector3f p)
{
	Vector3f zero(0, 0, 0);
	Vector3f B(0, 0, 1);
	if (approx(zero, Vector3f::cross(p, B)))
	{
		B = Vector3f(1, 0, 0);
	}
	if (approx(zero, Vector3f::cross(p, B)))
	{
		B = Vector3f(0, 1, 0);
	}
	if (approx(zero, Vector3f::cross(p, B)))
	{
		B = Vector3f(0, 0, 0);
	}

	return B;
}

Curve evalFourPointBezier(const vector< Vector3f >& subP, unsigned steps, Vector3f& lastB) {
	Curve curve;
	Matrix4f cpMatrix;
	for (int i = 0; i < 4; i++)
	{
		cpMatrix.setCol(i, Vector4f(subP[i], 0));
	}

	Matrix4f tangentMatrix;
	tangentMatrix.setRow(0, Vector4f(subP[0], 0));
	tangentMatrix.setRow(1, Vector4f(subP[3], 0));

	Vector3f tgnt;
	for (int i = 0; i < 3; i++)
	{
		tgnt[i] = -3 * subP[0][i] + 3 * subP[1][i];
	}
	tangentMatrix.setRow(2, Vector4f(tgnt, 0));

	for (int i = 0; i < 3; i++)
	{
		tgnt[i] = -3 * subP[2][i] + 3 * subP[3][i];
	}
	tangentMatrix.setRow(3, Vector4f(tgnt, 0));
	Matrix4f coefficentMatrix = (HERMITE_MATRIX * tangentMatrix).transposed();

	for (float i = 0; i <= steps; i++)
	{
		float t = i / steps;
		CurvePoint newPoint;
		//Vertices
		Vector4f curvePoints = cpMatrix * BERNSTEIN_MATRIX * Vector4f(1, t, (float)pow(t, 2), (float)pow(t, 3));
		//Tangents
		Vector4f tangent = coefficentMatrix * Vector4f(3 * (float)pow(t, 2), 2 * t, 1, 0);

		newPoint.V = curvePoints.xyz();
		newPoint.T = tangent.xyz().normalized();

		if (approx(lastB, Vector3f(0, 0, 0)) || approx(Vector3f::cross(newPoint.T, lastB), Vector3f(0, 0, 0)))
		{
			lastB = getB(newPoint.T);
		}

		newPoint.N = Vector3f::cross(lastB, newPoint.T).normalized();
		newPoint.B = Vector3f::cross(newPoint.T, newPoint.N).normalized();
		lastB = newPoint.B;

		curve.push_back(newPoint);
	}
	return curve;
}

Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	Curve curve;

	Vector3f lastB(0, 0, 0);

	std::vector<Vector3f> subP;
	for (int start = 0; start < P.size() - 3; start += 3)
	{
		for (int offset = 0; offset < 4; offset++)
		{
			subP.push_back(P[start + offset]);
		}

		Curve temp = evalFourPointBezier(subP, steps, lastB);
		for (int i = 0; i < temp.size(); i++)
		{
			curve.push_back(temp[i]);
		}

		subP.clear();
	}

	cerr << "\t>>> evalBezier has been called with the following input:" << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cerr << "\t>>> " << P[i] << endl;
	}

	cerr << "\t>>> Steps (type steps): " << steps << endl;
	return curve;
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	Curve bSplineCurve;
	vector<Vector3f> pointsInBernstein;
	Vector3f lastB(0, 0, 0);
	for (int start = 0; start <= P.size() - 4; start++)
	{
		Matrix4f cpMatrix;
		for (int i = 0; i < 4; i++)
		{
			cpMatrix.setCol(i, Vector4f(P[start + i], 0));
		}
		Matrix4f points = cpMatrix * B_SPLINE_BASIS * BERNSTEIN_MATRIX.inverse();
		for (int i = 0; i < 4; i++)
		{
			pointsInBernstein.push_back(points.getCol(i).xyz());
		}

		Curve curvePortion = evalFourPointBezier(pointsInBernstein, steps, lastB);
		if (start == P.size() - 4)
		{
			for (int i = 0; i < curvePortion.size(); i++)
			{
				bSplineCurve.push_back(curvePortion[i]);
			}
		}
		else
		{
			for (int i = 0; i < curvePortion.size() - 1; i++)
			{
				bSplineCurve.push_back(curvePortion[i]);
			}
		}

		pointsInBernstein.clear();
	}

	int curveSize = (int)bSplineCurve.size();
	if (approx(bSplineCurve[0].V, bSplineCurve[curveSize - 1].V)
		&& approx(bSplineCurve[0].T, bSplineCurve[curveSize - 1].T)
		&& !approx(bSplineCurve[0].N, bSplineCurve[curveSize - 1].N))
	{
		CurvePoint beg = bSplineCurve[0];
		CurvePoint end = bSplineCurve[curveSize - 1];

		float diff = acos(Vector3f::dot(end.N, beg.N));

		for (int i = 0; i < curveSize-1; i++)
		{
			CurvePoint point = bSplineCurve[i];
			float angle = diff * ((float)(curveSize - i) / curveSize);

			bSplineCurve[i].N = Matrix3f::rotation(point.T, angle) * point.N;
			bSplineCurve[i].B = Matrix3f::rotation(point.T, angle) * point.B;
		}
	}

	cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cerr << "\t>>> " << P[i] << endl;
	}

	cerr << "\t>>> Steps (type steps): " << steps << endl;

	return bSplineCurve;
}

Curve evalCircle(float radius, unsigned steps)
{
	// This is a sample function on how to properly initialize a Curve
	// (which is a vector< CurvePoint >).

	// Preallocate a curve with steps+1 CurvePoints
	Curve R(steps + 1);

	// Fill it in counterclockwise
	for (unsigned i = 0; i <= steps; ++i)
	{
		// step from 0 to 2pi
		float t = 2.0f * c_pi * float(i) / steps;

		// Initialize position
		// We're pivoting counterclockwise around the y-axis
		R[i].V = radius * Vector3f(cos(t), sin(t), 0);

		// Tangent vector is first derivative
		R[i].T = Vector3f(-sin(t), cos(t), 0);

		// Normal vector is second derivative
		R[i].N = Vector3f(-cos(t), -sin(t), 0);

		// Finally, binormal is facing up.
		R[i].B = Vector3f(0, 0, 1);
	}

	return R;
}

void recordCurve(const Curve& curve, VertexRecorder* recorder)
{
	const Vector3f WHITE(1, 1, 1);
	for (int i = 0; i < (int)curve.size() - 1; ++i)
	{
		recorder->record_poscolor(curve[i].V, WHITE);
		recorder->record_poscolor(curve[i + 1].V, WHITE);
	}
}
void recordCurveFrames(const Curve& curve, VertexRecorder* recorder, float framesize)
{
	Matrix4f T;
	const Vector3f RED(1, 0, 0);
	const Vector3f GREEN(0, 1, 0);
	const Vector3f BLUE(0, 0, 1);

	const Vector4f ORGN(0, 0, 0, 1);
	const Vector4f AXISX(framesize, 0, 0, 1);
	const Vector4f AXISY(0, framesize, 0, 1);
	const Vector4f AXISZ(0, 0, framesize, 1);

	for (int i = 0; i < (int)curve.size(); ++i)
	{
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));

		// Transform orthogonal frames into model space
		Vector4f MORGN = T * ORGN;
		Vector4f MAXISX = T * AXISX;
		Vector4f MAXISY = T * AXISY;
		Vector4f MAXISZ = T * AXISZ;

		// Record in model space
		recorder->record_poscolor(MORGN.xyz(), RED);
		recorder->record_poscolor(MAXISX.xyz(), RED);

		recorder->record_poscolor(MORGN.xyz(), GREEN);
		recorder->record_poscolor(MAXISY.xyz(), GREEN);

		recorder->record_poscolor(MORGN.xyz(), BLUE);
		recorder->record_poscolor(MAXISZ.xyz(), BLUE);
	}
}