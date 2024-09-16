// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VISUALRESULT_H
#define VISUALRESULT_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 初始化滤镜数据表
void initFilters(const char* dir);

// 传入的数据，默认是RGB888 24位深buffer
/**
 * @brief imageFilter24 使用指定滤镜处理一帧图像
 * @param            data: 图像数据buffer 格式为RGB888 24位深
 * @param           width: 图像分辨率，宽度值
 * @param          height: 图像分辨率，高度值
 * @param      filterName: 滤镜名称 如：warm(暖色)、cold(冷色)、black(黑白)、gray(灰度)、brown(褐色)、print(冲印)、classic(古典)、blues(蓝调)
 * @param          strength: 滤镜强度，范围为0~100，0：没有滤镜 100：有滤镜  传入参数100时，滤镜强度最大；默认为100
 * @return
 */
void imageFilter24(uint8_t* data, int width, int height, const char* filterName, int strength);

/**
* @brief 曝光调节
* @param data 数据指针
* @param width 图像宽度
* @param height 图像高度
* @param value 调整值 -100 ～ 100, -100：最暗， 0：没有曝光， 100：最强曝光
*/
void exposure(uint8_t *data, const int width, const int height, int value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // VISUALRESULT_H
