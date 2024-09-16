// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "visualresult.h"
#include "utils.h"

#include <string.h>
#include <cmath>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_EXPOSURE 100
#define MIN_EXPOSURE -100

void initFilters(const char* dir)
{
    std::string path = dir;
    if (path.empty())
        path = "/usr/share/libimagevisualresult/filter_cube";
    Libutils::readFilters(path);
}

void imageFilter24(uint8_t *data, int width, int height, const char *filterName, int strength)
{
    //printf("image_filter24 called, filter is %s\n", filterName);

    uint8_t* frame = data;
    if (!frame)
        return;

    lutData* pLut = Libutils::getFilterLut(filterName);
    if (!pLut || pLut->size() == 0) {
        printf("filter:%s file is not found..", filterName);
        return;
    }

    lutData& lut = *pLut;

    int nRowBytes = (width * 24 + 31) / 32 * 4;
    int  lineNum_24 = 0;     //行数
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            lineNum_24 = i * nRowBytes;
            uint8_t *R = &(frame[lineNum_24 + j * 3 + 0]);
            uint8_t *G = &(frame[lineNum_24 + j * 3 + 1]);
            uint8_t *B = &(frame[lineNum_24 + j * 3 + 2]);

            unsigned int index = static_cast<unsigned int>((*R >> 3) + ((*G >> 3) << 5)  + ((*B >> 3) << 10));

            if (strength == 100) {
                *R = lut[index][0];
                *G = lut[index][1];
                *B = lut[index][2];
            }
            else if (strength == 0) {
                continue;
            }
            else {
                *R = static_cast<uint8_t>(*R + (lut[index][0] - *R) * strength / 100);
                *G = static_cast<uint8_t>(*G + (lut[index][1] - *G) * strength / 100);
                *B = static_cast<uint8_t>(*B + (lut[index][2] - *B) * strength / 100);
            }
        }
    }
}

void exposure(uint8_t *data, const int width, const int height, int value)
{
    if(value > MAX_EXPOSURE || value < MIN_EXPOSURE)
        value = 0;
    if(value == 0)
        return;

    uint8_t* frame = static_cast<uint8_t*>(data);
    if (nullptr == frame)
        return;

    if (!frame)
        return;

    float step = value / 100.0;
    int size = width * height;

    float pow_result = pow(2, step);
    for(int i = 0; i < size; i++) {
        int r = frame[i*3] * pow_result;
        int g = frame[i*3 + 1] * pow_result;
        int b = frame[i*3 + 2] * pow_result;

        frame[i*3] = r > 255 ? 255 : r;
        frame[i*3 + 1] = g > 255 ? 255 : g;
        frame[i*3 + 2] = b > 255 ? 255 : b;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
