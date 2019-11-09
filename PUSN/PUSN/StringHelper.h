#pragma once
#include <string>
#include <vector>

class StringHelper
{
public:
	static std::wstring StringToWide(std::string str);
	static std::string GetDirectoryFromPath(const std::string & filepath);
	static std::string GetFileExtension(const std::string & filename);
	static void Split(const std::string& str, std::vector<std::string>& cont, char delim = ' ');
};