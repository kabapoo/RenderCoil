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

	void makeCookParams(const char* filename, int cent_num, int devi_num);
	void makeCookNestedParams(const char* filename, int cent_num, int devi_num, int rand_num);
	std::vector<std::array<float, 7>> readCookParams(const char* filename);
};