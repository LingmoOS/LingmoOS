// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "lut.h"

#include <string.h>
#include <dirent.h>

#include <fstream>

using namespace std;

map_lut Libutils::m_map_lut;

lutData* Libutils::getFilterLut(string filter)
{
    if (filter.empty())
        return nullptr;

    if (m_map_lut.find(filter) != m_map_lut.end()) {
        //printf("filter:%s is finded..\n", filter.c_str());

        return &(m_map_lut[filter]);
    }

    return nullptr;
}

void Libutils::readFilters(const std::string& dir)
{
    if (dir.empty())
        return;

    int count=0;								//临时计数，[0，SINGLENUM]

    DIR *dp ;
    struct dirent *dirp ;

    //打开指定目录
    if((dp = opendir( dir.c_str())) == nullptr) {
        printf("open %s failed, file is not *.CUBE file, or dir does not exist.\n", dir.c_str());
        return;
    }

    m_map_lut.clear();
    lutData lut;

    //开始遍历目录
    while((dirp = readdir(dp)) != nullptr)
    {
        //跳过'.'和'..'两个目录
        if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0)
            continue;

        size_t size = strlen(dirp->d_name);

        //如果是.CUBE文件，长度至少是6
        if(size < 6)
            continue;

        //只存取扩展名为.CUBE
        if(strcmp((dirp->d_name + (size - 5)), ".CUBE") != 0)
            continue;

        //根据CUBE文件获取对应dat文件所在路径
        string filename = dirp->d_name;
        string filter = filename.substr(0, filename.find_last_of('.'));
        string filepath = dir + "/" + filename;
        string datfilepath = dir + "/" + filter + ".dat";

        //若读取dat文件失败，先从CUBE文件转为data，存放到当前目录，然后重新从dat中读取lut数据
        if (!readCubeFileFromDat(datfilepath, lut)) {
            parse_lut_cube(filepath, datfilepath.c_str());
            if (readCubeFileFromDat(datfilepath, lut))
                count++;
            else if (readCubeFile(filepath, lut))
                count++;
        }
        else {
            count++;
        }
    }

    closedir(dp);

    printf("read %d CUBE/dat files...\n", count);
}

bool Libutils::readCubeFile(std::string filename, lutData &lut)
{
    lut.clear();

    bool cubeDataStart = false, sizeDataStart = false;
    ifstream in(filename);
    std::string line;
    if (in.fail()) {
        printf("%s read *.CUBE fail, may be not exist!\n", filename.c_str());
        return false;
    }

    size_t n = 0;
    vector<uint8_t> rgbInt;
    while (getline(in, line)){//按行读取文件
        if(cubeDataStart) { //读取lut数据
            vector<string> rgbStr;
            split(line, " ", rgbStr);

            if (rgbStr.size() == 3) {
                lut.push_back(rgbInt);
                for (size_t i = 0; i < 3; i++)
                    lut[n].push_back(static_cast<uint8_t>(atof(rgbStr[i].c_str()) * 255));
                n++;
            }
        }

        if(sizeDataStart) { //读取lut尺寸
            sizeDataStart = false;
        }

        if(strncmp(line.c_str(), "#LUT data points", sizeof("#LUT data points")) == 0) {
            cubeDataStart = true;
        }
        if(strncmp(line.c_str(), "#LUT size", sizeof("#LUT size")) == 0) {
            sizeDataStart = true;
        }
    }

    in.close();


    string filter = filename.substr(0, filename.find_last_of('.'));
    filter = filter.substr(filter.find_last_of('/') + 1);

    m_map_lut[filter] = lut;

    printf("read %s success,filtername:%s..\n", filename.c_str(), filter.c_str());

    return true;
}

bool Libutils::readCubeFileFromDat(string filename, lutData &lut)
{
    lut.clear();

    int rgbSize = 3 * sizeof(int);
    ifstream inFile(filename, ios::in | ios::binary);

    if (inFile.fail()) {
        printf("%s read *.dat fail, may be not exist!\n", filename.c_str());
        return false;
    }

    //先读尺寸
    int lutSize = -1;
    inFile.read((char *)&lutSize, sizeof(int));

    //再读数据
    int temp[3];
    while (inFile.read((char *)&temp[0], rgbSize)) {
        vector<uint8_t> rgbInt;
        for (int i = 0; i < 3; i++) {
            rgbInt.push_back(static_cast<uint8_t>(temp[i]));
        }
        lut.push_back(rgbInt);
    }
    inFile.close();

    string filter = filename.substr(0, filename.find_last_of('.'));
    filter = filter.substr(filter.find_last_of('/') + 1);

    m_map_lut[filter] = lut;

    printf("read %s success,filtername:%s..\n", filename.c_str(), filter.c_str());

    return true;
}

//按字符“delimit”分割字符串
void Libutils::split(string &str, string delimit, vector<string> &result)
{
    size_t pos = str.find(delimit);
    str += delimit;//将分隔符加入到最后一个位置，方便分割最后一位

    while (pos != string::npos) {
        result.push_back(str.substr(0, pos));
        str = str.substr(pos + 1);
        pos = str.find(delimit);
    }
}
