#include "MillingData.h"


MillingData* MillingData::ReadFromFile(std::string filePath)
{
	MillingData * data = new MillingData();

	std::string extension = filePath.substr(filePath.rfind("."));
	data->flatCut = extension[0] == 'f';
	data->cutSize = data->flatCut = std::stoi(filePath.substr(1));

	std::ifstream file(filePath);
	std::string str;
	while (std::getline(file, str)) {
		if (str[0] != 'N')
			continue;

		int pos = filePath.find("G");
		if (pos == -1)
			continue;

		if (str.substr(pos + 1, 2) != "01")
			continue;

		Position position;
		str = str.substr(pos + 3);

		if (str[0] == 'X') {
			pos = str.find(".");
			position.x = std::stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.x = data->moves.back().x;
		}

		if (str[0] == 'Y') {
			pos = str.find(".");
			position.y = std::stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.y = data->moves.back().y;
		}

		if (str[0] == 'Z') {
			pos = str.find(".");
			position.z = std::stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.z = data->moves.back().z;
		}
	}

	return data;
}

