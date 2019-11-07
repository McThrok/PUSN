#include "PathGenerator.h"

void PathGenerator::LoadElephant()
{
	char cCurrentPath[FILENAME_MAX];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = std::string(cCurrentPath) + "\\Models\\";
	string filePath = path + "wt_elephant.mg1";

	ifstream file(filePath);
	string str;
	while (getline(file, str)) {
		string a = str;

	}
}