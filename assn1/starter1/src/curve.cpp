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

Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}
	// TODO:
	// You should implement this function so that it returns a Curve
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you
	// the number of points to generate on each piece of the spline.
	// At least, that's how the sample solution is implemented and how
	// the SWP files are written.  But you are free to interpret this
	// variable however you want, so long as you can control the
	// "resolution" of the discretized spline curve with it.

	// Make sure that this function computes all the appropriate
	// Vector3fs for each CurvePoint: V,T,N,B.
	// [NBT] should be unit and orthogonal.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

	Curve curve;

	Matrix4f cpMatrix;
	for (int i = 0; i < 4; i++)
	{
		cpMatrix.setCol(i, Vector4f(P[i], 0));
	}

	Matrix4f tangentMatrix;
	tangentMatrix.setRow(0, Vector4f(P[0], 0));
	tangentMatrix.setRow(1, Vector4f(P[3], 0));
	Vector3f tgnt;
	for (int i = 0; i < 3; i++)
	{
		tgnt[i] = -3 * P[0][i] + 3 * P[1][i];
	}
	tangentMatrix.setRow(2, Vector4f(tgnt, 0));

	for (int i = 0; i < 3; i++)
	{
		tgnt[i] = -3 * P[2][i] + 3 * P[3][i];
	}
	tangentMatrix.setRow(3, Vector4f(tgnt, 0));

	float dist = 1.0f / steps;
	for (float t = 0; t < 1; t += dist)
	{
		CurvePoint newPoint;
		
		//Vertices
		Vector4f curvePoints = cpMatrix * BERNSTEIN_MATRIX * Vector4f(1, t, (float)pow(t, 2), (float)pow(t, 3));

		//Tangents
		Vector4f tangent = (HERMITE_MATRIX.transposed() * tangentMatrix) * Vector4f(3 * (float)pow(t, 2), 2 * t, 1, 0);

		newPoint.V = curvePoints.xyz();
		newPoint.T = tangent.xyz().normalized();



		curve.push_back(newPoint);
	}

	cerr << "\t>>> evalBezier has been called with the following input:" << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cerr << "\t>>> " << P[i] << endl;
	}

	cerr << "\t>>> Steps (type steps): " << steps << endl;
	cerr << "\t>>> Returning empty curve." << endl;

	// Right now this will just return this empty curve.
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

	// TODO:
	// It is suggested that you implement this function by changing
	// basis from B-spline to Bezier.  That way, you can just call
	// your evalBezier function.

	cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
	for (int i = 0; i < (int)P.size(); ++i)
	{
		cerr << "\t>>> " << P[i] << endl;
	}

	cerr << "\t>>> Steps (type steps): " << steps << endl;
	cerr << "\t>>> Returning empty curve." << endl;

	// Return an empty curve right now.
	return Curve();
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

