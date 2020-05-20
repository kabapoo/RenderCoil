#include "sample_io.h"

ParameterSample::ParameterSample()
{
    
}

void ParameterSample::makeCookParams(const char* filename, int cent_num, int devi_num)
{
    srand((unsigned int)time(0));
    
    float centrics[7];
    float param[7];

    fout.open(filename, std::ios::out | std::ios::binary);
    fout.write((char*)&cent_num, sizeof(int));
    fout.write((char*)&devi_num, sizeof(int));

    for (int i = 0; i < cent_num; ++i)
    {
        for (int k = 0; k < 7; ++k)
        {
            centrics[k] = (float)(rand() % 100) / (float)100.0f;
        }
        
        for (int j = 0; j < devi_num; ++j)
        {
            for (int k = 0; k < 7; ++k)
            {
                param[k] = centrics[k] + (float)((rand() % 160) - 80) / 1000.0f;
            }
            fout.write((char*)param, sizeof(param));
        }
    }
    fout.close();
}

void ParameterSample::makeCookNestedParams(const char* filename, int cent_num, int devi_num, int rand_num)
{
    srand((unsigned int)time(0));

    float centrics[7];
    float param[7];
    float rough_centric = 0.0f;
    int rough_num = cent_num * devi_num;
    fout.open(filename, std::ios::out | std::ios::binary);
    fout.write((char*)&rough_num, sizeof(int));
    fout.write((char*)&devi_num, sizeof(int));

    for (int i = 0; i < cent_num; ++i)
    {
        rough_centric = ((i + 1) % cent_num) / (float)cent_num;
        for (int j = 0; j < devi_num; ++j)
        {
            param[3] = rough_centric + (float)((rand() % cent_num) / (float)cent_num * 0.5f) - (1.0f / (float)cent_num * 0.25f);
            for (int k = 0; k < rand_num; ++k)
            {
                param[0] = (float)(rand() % 100) / 100.0f;
                param[1] = (float)(rand() % 100) / 100.0f;
                param[2] = (float)(rand() % 100) / 100.0f;
                param[4] = ((float)(rand() % 80) + 2.0f) / 100.0f;
                param[5] = ((float)(rand() % 80) + 2.0f) / 100.0f;
                param[6] = ((float)(rand() % 80) + 2.0f) / 100.0f;

                fout.write((char*)param, sizeof(param));
            }
        }
    }
    fout.close();
}

std::vector<std::array<float, 7>> ParameterSample::readCookParams(const char* filename)
{
    fin.open(filename, std::ios::in | std::ios::binary);

    std::vector<std::array<float, 7>> samples;
    std::array<float, 7> temp;

    int cent_num = 0;
    int devi_num = 0;
    float param[7];
    fin.read((char*)&cent_num, sizeof(int));
    fin.read((char*)&devi_num, sizeof(int));
    for (int i = 0; i < cent_num * devi_num; ++i)
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