#include "mesh.h"

#include "vertexrecorder.h"

using namespace std;

namespace {
	vector<string> parse(string line, char sep) {
		vector<string> data;
		string curr;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == sep)
			{
				data.push_back(curr);
				curr.clear();
				continue;
			}

			curr.push_back(line[i]);
		}

		if (curr != "")
		{
			data.push_back(curr);
		}
		
		return data;
	}
}

void Mesh::load( const char* filename )
{
	// 4.1. load() should populate bindVertices, currentVertices, and faces

	ifstream file(filename);
	if (!file.is_open())
	{
		cout << "Unable to open .obj file\n";
		return;
	}

	while (!file.eof())
	{
		char buffer[256];

		file.getline(buffer, 256);

		vector<string> data = parse(buffer, ' ');

		if (data.size() != 4)
		{
			continue;
		}
		
		if (data[0] == "v")
		{
			Vector3f vertex(stof(data[1]), stof(data[2]), stof(data[3]));
			bindVertices.push_back(vertex);
		}
		else if (data[0] == "f")
		{
			Tuple3u face;
			for (int i = 1; i < 4; i++)
			{
				int vIndex = stoi(data[i]) - 1;
				if (vIndex >= bindVertices.size())
				{
					cout << "Invalid vertex index\n";
					return;
				}

				face[i - 1] = vIndex;
			}

			faces.push_back(face);
		}
	}

	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::draw()
{
	// 4.2 Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".

	VertexRecorder rec;
	
	for (Tuple3u face : faces)
	{
		Vector3f v1(currentVertices[face[0]]);
		Vector3f v2(currentVertices[face[1]]);
		Vector3f v3(currentVertices[face[2]]);

		Vector3f s1 = v2 - v1;
		Vector3f s2 = v3 - v1;

		Vector3f norm = Vector3f::cross(s1, s2);

		rec.record(v1, norm);
		rec.record(v2, norm);
		rec.record(v3, norm);
	}

	rec.draw();
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
	// 4.3. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments

	ifstream file(filename);
	if (!file.is_open())
	{
		cout << "Unable to open file\n";
		return;
	}

	while (!file.eof())
	{
		char buffer[256];

		file.getline(buffer, 256);

		vector<string> data = parse(buffer, ' ');

		if (data.size() != numJoints)
		{
			continue;
		}

		vector<float> weights;
		weights.push_back(0.0f);
		for (string d : data)
		{
			weights.push_back(stof(d));
		}

		attachments.push_back(weights);
	}

}
