// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LUT_H
#define LUT_H

#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

/**
* @brief 将3dLut写入二进制文件
* @param lut 指向相应LUT的指针
* @param lut_size LUT size
* @param name 文件名
*/
void write_lut_binary(vector<vector<float>> &lut, const int lut_size, const char *name);

/**
* @brief 解析.CUBE文件
* @param filename CUBE文件名
* @param binary_name 二进制文件名
*/
void parse_lut_cube(const string &filename, const char *binary_name);

#endif // LUT_H
