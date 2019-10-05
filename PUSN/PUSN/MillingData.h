#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct Position {
	float x, y, z;
	Position() {}
	Position(float _x, float _y, float _z) :x(_x), y(_y), z(_z) {}
};

class MillingData {
	bool flatCut;
	float cutSize;
	std::vector<Position> moves;

	static MillingData* ReadFromFile(std::string filePath);

};


