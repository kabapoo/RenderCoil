#include "sample_io.h"

ParameterSample::ParameterSample()
{
    
}

void ParameterSample::makeCookParams(std::string path, int cent_num, int rand_num)
{
    srand((unsigned int)time(0));
    
    float centrics[7];
    float param[7];

    std::string filename = path + "cook_" + std::to_string(cent_num);
    filename = filename + "_" + std::to_string(rand_num);
    filename = filename + "_params.bin";
    fout.open(filename.c_str(), std::ios::out | std::ios::binary);
    fout.write((char*)&cent_num, sizeof(int));
    fout.write((char*)&rand_num, sizeof(int));

    for (int i = 0; i < cent_num; ++i)
    {
        for (int k = 0; k < 7; ++k)
        {
            centrics[k] = (float)(rand() % 80 + 8) / (float)100.0f;
        }
        
        for (int j = 0; j < rand_num; ++j)
        {
            for (int k = 0; k < 7; ++k)
            {
                param[k] = centrics[k] + (float)((rand() % 120) - 60) / 1000.0f;
            }
            fout.write((char*)param, sizeof(param));
        }
    }
    fout.close();
}

void ParameterSample::makePBRParams(std::string path, int cent_num, int rand_num)
{
    srand((unsigned int)time(0));

    float centrics[5];
    float param[5];

    std::string filename = path + "pbr5_" + std::to_string(cent_num);
    filename = filename + "_" + std::to_string(rand_num);
    filename = filename + "_params.bin";
    fout.open(filename.c_str(), std::ios::out | std::ios::binary);
    fout.write((char*)&cent_num, sizeof(int));
    fout.write((char*)&rand_num, sizeof(int));

    for (int i = 0; i < cent_num; ++i)
    {
        for (int k = 0; k < 5; ++k)
        {
            centrics[k] = (float)(rand() % 80 + 8) / (float)100.0f;
        }

        for (int j = 0; j < rand_num; ++j)
        {
            for (int k = 0; k < 5; ++k)
            {
                param[k] = centrics[k] + (float)((rand() % 120) - 60) / 1000.0f;
            }
            fout.write((char*)param, sizeof(param));
        }
    }
    fout.close();
}

void ParameterSample::makeCookNestedParams(std::string path, int cent_num, int devi_num, int rand_num)
{
    srand((unsigned int)time(0));

    float param[7];
    float rough_centric = 0.0f;
    int rough_num = cent_num * devi_num;
    int cnt = 0;

    std::string filename = path + "cook_c" + std::to_string(cent_num);
    filename = filename + "_d" + std::to_string(devi_num);
    filename = filename + "_r" + std::to_string(rand_num);
    filename = filename + "_params.bin";

    fout.open(filename.c_str(), std::ios::out | std::ios::binary);
    fout.write((char*)&rough_num, sizeof(int));
    fout.write((char*)&rand_num, sizeof(int));

    for (int i = 0; i < cent_num; ++i)
    {
        rough_centric = ((i + 1) % cent_num) / (float)cent_num;
        for (int j = 0; j < devi_num; ++j)
        {
            param[3] = rough_centric + (float)((rand() % 10) / (float)cent_num * 0.5f) - (1.0f / (float)cent_num * 0.25f);
            for (int k = 0; k < rand_num; ++k)
            {
                param[0] = (float)(rand() % 90 + 5) / 100.0f;
                param[1] = (float)(rand() % 90 + 5) / 100.0f;
                param[2] = (float)(rand() % 90 + 5) / 100.0f;
                param[4] = ((float)(rand() % 80) + 2.0f) / 100.0f;
                param[5] = ((float)(rand() % 80) + 2.0f) / 100.0f;
                param[6] = ((float)(rand() % 80) + 2.0f) / 100.0f;

                fout.write((char*)param, sizeof(param));
                cnt++;
            }
        }
    }
    std::cout << cnt << std::endl;
    fout.close();
}

std::vector<std::array<float, 7>> ParameterSample::readCookParams(const char* filename)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 7>> samples;
    std::array<float, 7> temp;

    int cent_num = 0;
    int rand_num = 0;
    float param[7];
    fin.read((char*)&cent_num, sizeof(int));
    fin.read((char*)&rand_num, sizeof(int));
    for (int i = 0; i < cent_num * rand_num; ++i)
    {
        fin.read((char*)param, sizeof(param));
        for (int j = 0; j < 7; ++j)
        {
            temp[j] = param[j];
        }
        samples.push_back(temp);
    }
    fin.close();
    return samples;
}

std::vector<std::array<float, 5>> ParameterSample::readPBRParams(const char* filename)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 5>> samples;
    std::array<float, 5> temp;

    int cent_num = 0;
    int rand_num = 0;
    float param[5];
    fin.read((char*)&cent_num, sizeof(int));
    fin.read((char*)&rand_num, sizeof(int));
    for (int i = 0; i < cent_num * rand_num; ++i)
    {
        fin.read((char*)param, sizeof(param));
        for (int j = 0; j < 5; ++j)
        {
            temp[j] = param[j];
        }
        samples.push_back(temp);
    }
    fin.close();
    return samples;
}


std::vector<std::array<float, 7>> ParameterSample::readCookBinary(const char* filename, int rows)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 7>> samples;
    std::array<float, 7> temp;

    float param[7];
    for (int i = 0; i < rows; ++i)
    {
        fin.read((char*)param, sizeof(param));
        for (int j = 0; j < 7; ++j)
        {
            temp[j] = param[j];
        }
        samples.push_back(temp);
    }
    fin.close();
    return samples;
}

std::vector<std::array<float, 5>> ParameterSample::readPBRBinary(const char* filename, int rows)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 5>> samples;
    std::array<float, 5> temp;

    float param[5];
    for (int i = 0; i < rows; ++i)
    {
        fin.read((char*)param, sizeof(param));
        for (int j = 0; j < 5; ++j)
        {
            temp[j] = param[j];
        }
        samples.push_back(temp);
    }
    fin.close();
    return samples;
}

std::vector<std::array<float, 8>> ParameterSample::loadParams(const char* filename, int rows)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 8>> samples;
    std::array<float, 8> temp;

    float param[8];
    for (int i = 0; i < rows; ++i)
    {
        fin.read((char*)param, sizeof(param));
        for (int j = 0; j < 8; ++j)
        {
            temp[j] = param[j];
        }
        samples.push_back(temp);
    }
    fin.close();
    return samples;
}