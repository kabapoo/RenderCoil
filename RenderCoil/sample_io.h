#pragma once
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class ParameterSample
{
private:
	std::ofstream fout;
	std::ifstream fin;

public:
	ParameterSample();

	void makeCookParams(std::string path, int cent_num, int devi_num);
	void makePBRParams(std::string path, int cent_num, int rand_num);
	void makeCookNestedParams(std::string path, int cent_num, int devi_num, int rand_num);
	std::vector<std::array<float, 7>> readCookParams(const char* filename);
	std::vector<std::array<float, 5>> readPBRParams(const char* filename);
	std::vector<std::array<float, 7>> readCookBinary(const char* filename, int rows);
	std::vector<std::array<float, 5>> readPBRBinary(const char* filename, int rows);
};