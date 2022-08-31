#include "surf.h"
#include "vertexrecorder.h"
using namespace std;

namespace
{
	const float c_pi = 3.14159265358979323846f;
	// We're only implenting swept surfaces where the profile curve is
	// flat on the xy-plane.  This is a check function.
	static bool checkFlat(const Curve& profile)
	{
		for (unsigned i = 0; i < profile.size(); i++)
			if (profile[i].V[2] != 0.0 ||
				profile[i].T[2] != 0.0 ||
				profile[i].N[2] != 0.0)
				return false;

		return true;
	}
}

// DEBUG HELPER
Surface quad() {
	Surface ret;
	ret.VV.push_back(Vector3f(-1, -1, 0));
	ret.VV.push_back(Vector3f(+1, -1, 0));
	ret.VV.push_back(Vector3f(+1, +1, 0));
	ret.VV.push_back(Vector3f(-1, +1, 0));

	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));

	ret.VF.push_back(Tup3u(0, 1, 2));
	ret.VF.push_back(Tup3u(0, 2, 3));
	return ret;
}

Surface makeSurfRev(const Curve& profile, unsigned steps)
{
	Surface surface;
	//surface = quad();

	if (!checkFlat(profile))
	{
		cerr << "surfRev profile curve must be flat on xy plane." << endl;
		exit(0);
	}

	int profDotCount = (int)profile.size();

	for (unsigned s = 0; s <= steps; s++)
	{
		Matrix3f rotMatrix = Matrix3f::rotateY(2 * c_pi * (float(s) / steps));
		Matrix3f nRotMatrix = rotMatrix.inverse().transposed();
		for (int n = 0; n < profDotCount; n++)
		{
			Vector4f vec = rotMatrix * profile[n].V;

			surface.VV.push_back(vec.xyz());

			surface.VN.push_back(-(nRotMatrix * profile[n].N).normalized());
		}

		if (s > 0)
		{
			for (int n = 0; n < profDotCount - 1; n++)
			{
				int p1 = (s * profDotCount) + n;

				int p2 = (s - 1) * profDotCount + n;
				int p3 = (s - 1) * profDotCount + n + 1;

				surface.VF.push_back(Tup3u(p1, p2, p3));

				p2 = (s - 1) * profDotCount + n + 1;
				p3 = s * profDotCount + n + 1;

				surface.VF.push_back(Tup3u(p1, p2, p3));
			}
		}
	}


	/*for (int i = 0; i < 30; i++)
	{
		for (unsigned j = 1; j <= steps; j++)
		{
			int p1 = i * steps + j;

			int p2;
			int p3;

			p2 = p1 - 1;
			p3 = (1 + i) * steps + p2;

			surface.VF.push_back(Tup3u(p1, p2, p3));

			p2 = (1 + i) * steps + j - 1;
			p3 = (1 + i) * steps + p1;

			surface.VF.push_back(Tup3u(p1, p2, p3));
		}
	}*/

	return surface;
}

Surface makeGenCyl(const Curve& profile, const Curve& sweep)
{
	Surface surface;
	surface = quad();

	if (!checkFlat(profile))
	{
		cerr << "genCyl profile curve must be flat on xy plane." << endl;
		exit(0);
	}

	// TODO: Here you should build the surface.  See surf.h for details.

	cerr << "\t>>> makeGenCyl called (but not implemented).\n\t>>> Returning empty surface." << endl;

	return surface;
}

void recordSurface(const Surface& surface, VertexRecorder* recorder) {
	const Vector3f WIRECOLOR(0.4f, 0.4f, 0.4f);
	for (int i = 0; i < (int)surface.VF.size(); i++)
	{
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][1]], surface.VN[surface.VF[i][1]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][2]], surface.VN[surface.VF[i][2]], WIRECOLOR);
	}
}

void recordNormals(const Surface& surface, VertexRecorder* recorder, float len)
{
	const Vector3f NORMALCOLOR(0, 1, 1);
	for (int i = 0; i < (int)surface.VV.size(); i++)
	{
		recorder->record_poscolor(surface.VV[i], NORMALCOLOR);
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR);
	}
}

void outputObjFile(ostream& out, const Surface& surface)
{

	for (int i = 0; i < (int)surface.VV.size(); i++)
		out << "v  "
		<< surface.VV[i][0] << " "
		<< surface.VV[i][1] << " "
		<< surface.VV[i][2] << endl;

	for (int i = 0; i < (int)surface.VN.size(); i++)
		out << "vn "
		<< surface.VN[i][0] << " "
		<< surface.VN[i][1] << " "
		<< surface.VN[i][2] << endl;

	out << "vt  0 0 0" << endl;

	for (int i = 0; i < (int)surface.VF.size(); i++)
	{
		out << "f  ";
		for (unsigned j = 0; j < 3; j++)
		{
			unsigned a = surface.VF[i][j] + 1;
			out << a << "/" << "1" << "/" << a << " ";
		}
		out << endl;
	}
}
