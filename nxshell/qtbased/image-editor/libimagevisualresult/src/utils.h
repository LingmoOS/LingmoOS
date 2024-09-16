// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <map>
#include <cstdint>
#include <string>

typedef std::vector<std::vector<uint8_t>>       lutData;
typedef std::map<std::string, lutData>          map_lut;

class Libutils
{
public:
    // 根据滤镜名称获取滤镜色标数据
    static lutData* getFilterLut(std::string filter);

    // 读取路径下所有cube文件滤镜数据到内存中
    static void readFilters(const std::string& dir);
    // 读取目标cube文件滤镜数据到内存中
    static bool readCubeFile(std::string filename, lutData& lut);
    // 从二进制文件读取3dLut数据
    static bool readCubeFileFromDat(std::string filename, lutData& lut);
    // 用来拆分颜色值字串
    static void split(std::string &str, std::string delimit, std::vector<std::string>&result);

private:

    static map_lut m_map_lut; //滤镜数据表，key：滤镜名 value：滤镜数据
};

#endif // UTILS_H
