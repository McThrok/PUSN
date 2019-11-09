#include "PathGenerator.h"

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
		string elementName = header[0];
		int n = stoi(header[1]);
		for (int j = 0; j < n; j++)
		{
			getline(file, line);
			if (elementName == "surfaceC0")
				model.push_back(make_shared<BezierSurfaceC0>(new BezierSurfaceC0(line)));
			else if (elementName == "tubeC0")
				model.push_back(make_shared<BezierSurfaceC0>(new BezierSurfaceC0(line), true));
		}

	}
}