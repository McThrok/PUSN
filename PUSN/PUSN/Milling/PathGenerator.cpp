#include "PathGenerator.h"

PathGenerator::PathGenerator(int _resX, int _resY, XMFLOAT3 _size)
{
	resX = _resX;
	resY = _resY;
	size = _size;
	XMMATRIX transform = XMMatrixScaling(resX / 35.0f, resY / 35.0f, 3) * XMMatrixTranslation(resX / 2.25f, resY / 2.25f, 3);
}

vector<BezierSurfaceC0*> PathGenerator::GetModel()
{
	vector<BezierSurfaceC0*> result;

	for (int i = 0; i < model.size(); i++)
		result.push_back(model[i].get());

	return result;
}

void PathGenerator::LoadElephant()
{
	char cCurrentPath[FILENAME_MAX];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = std::string(cCurrentPath) + "\\Models\\";
	string filePath = path + "wt_elephant.mg1";

	ifstream file(filePath);
	string line;
	while (getline(file, line)) {

		vector<string> header;
		StringHelper::Split(line, header);

		if (header.empty())
			continue;

		string elementName = header[0];
		int n = stoi(header[1]);

		for (int j = 0; j < n; j++)
		{
			getline(file, line);

			if (elementName == "surfaceC0")
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line)));
			else if (elementName == "tubeC0")
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line, true)));
		}
	}
}
void PathGenerator::SavePath(vector<XMFLOAT3> moves, string filePath)
{
	stringstream ss;

	for (int i = 0; i < moves.size(); i++)
	{
		ss << "N9G01";

		XMFLOAT3& point = moves[i];

		if (i == 0 || point.x != moves[i - 1].x)
			ss << "X" << fixed << std::setprecision(3) << point.x;

		if (i == 0 || point.y != moves[i - 1].y)
			ss << "Y" << fixed << std::setprecision(3) << point.y;

		if (i == 0 || point.z != moves[i - 1].z)
			ss << "Z" << fixed << std::setprecision(3) << point.z;

		ss << endl;
	}

	ofstream file(filePath);
	if (file.is_open())
	{
		file.clear();
		file << ss.str();

		file.close();
	}
}


void PathGenerator::GenerateHeightMap()
{
	heightMap.resize(resX);
	for (int i = 0; i < resX; i++)
		heightMap[i] = vector<float>(resY, 0.0f);
}
vector<XMFLOAT3> PathGenerator::GenerateFirstPath()
{


	return vector<XMFLOAT3>();
}

void PathGenerator::GeneratePaths() {
	GenerateHeightMap();
	vector<XMFLOAT3> moves = GenerateFirstPath();
	SavePath(moves, "Paths\\elephant\\test1.k16");

}