/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LIBKYEDID_H
#define LIBKYEDID_H

/**
 * @file libkyedid.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 显示器信息
 * @version 0.1
 * @date 2023-6-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _BrightnessInfo
{
    int brightness_percentage;               //当前/最大亮度百分比
    int brightness_value;                    //当前/最大亮度值
}BrightnessInfo;

typedef struct _ChromaticityCoordinates
{
    char *xCoordinate;                       //色度坐标x轴
    char *yCoordinate;                       //色度坐标y轴
}ChromaticityCoordinates;

/**
 * @brief 获取显示器的当前接口
 * 
 * @return char** 显示器的当前接口，由NULL字符串表示结尾；由alloc生成，需要被kdk_edid_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_edid_get_interface();

/**
 * @brief 获取显示器的伽马值
 * 
 * @param name 显示器的当前接口
 * @return float 显示器的伽马值；若获取出错，返回0.00；
 */
extern float kdk_edid_get_gamma(char *name);

/**
 * @brief 获取显示器的屏幕尺寸（英寸）
 * 
 * @param name 显示器的当前接口
 * @return float 显示器的屏幕尺寸（英寸）；若获取出错，返回0.00；
 */
extern float kdk_edid_get_size(char *name);

/**
 * @brief 获取显示器的最大分辨率
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的最大分辨率，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_max_resolution(char *name);

/**
 * @brief 获取显示器的显示器型号
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的显示器型号，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_model(char *name);

/**
 * @brief 获取显示器的可视面积
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的可视面积，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_visible_area(char *name);

/**
 * @brief 获取显示器的厂商
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的厂商，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_manufacturer(char *name);

/**
 * @brief 获取显示器的生产日期/周
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的生产日期/周，失败返回-1。
 */
extern int kdk_edid_get_week(char *name);

/**
 * @brief 获取显示器的生产日期/年
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的生产日期/年，失败返回-1。
 */
extern int kdk_edid_get_year(char *name);

/**
 * @brief 获取是否是主显示器（是/否）
 * 
 * @param name 显示器的当前接口
 * @return int 是否是主显示器，1为是，0为否
 */
extern int kdk_edid_get_primary(char *name);

/**
 * @brief 获取分辨率
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回分辨率，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_resolution(char *name);

/**
 * @brief 获取图像高宽比
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回图像高宽比，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_ratio(char *name);

/**
 * @brief 获取显示器edid未解析的字符串
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回edid未解析的字符串，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_character(char *name);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_edid_freeall(char **ptr);

/**
 * @brief 获取显示器的最大亮度
 * 
 * @param name 显示器的当前接口
 * @return BrightnessInfo* 显示器的最大亮度结构体。
 */
extern BrightnessInfo* kdk_edid_get_max_brightness(char *name);

/**
 * @brief 获取显示器的当前亮度
 * 
 * @param name 显示器的当前接口
 * @return BrightnessInfo* 显示器的当前亮度结构体。
 */
extern BrightnessInfo* kdk_edid_get_current_brightness(char *name);

/**
 * @brief 获取显示器红色的色度坐标值
 * 
 * @param name 显示器的当前接口
 * @return ChromaticityCoordinates* 显示器色度坐标值结构体。
 */
extern ChromaticityCoordinates* kdk_edid_get_red_primary(char *name);

/**
 * @brief 获取显示器绿色的色度坐标值
 * 
 * @param name 显示器的当前接口
 * @return ChromaticityCoordinates* 显示器色度坐标值结构体。
 */
extern ChromaticityCoordinates* kdk_edid_get_green_primary(char *name);

/**
 * @brief 获取显示器蓝色的色度坐标值
 * 
 * @param name 显示器的当前接口
 * @return ChromaticityCoordinates* 显示器色度坐标值结构体。
 */
extern ChromaticityCoordinates* kdk_edid_get_blue_primary(char *name);

/**
 * @brief 获取显示器白色的色度坐标值
 * 
 * @param name 显示器的当前接口
 * @return ChromaticityCoordinates* 显示器色度坐标值结构体。
 */
extern ChromaticityCoordinates* kdk_edid_get_white_primary(char *name);

/**
 * @brief 获取显示器物理水平DPI
 * 
 * @param name 显示器的当前接口
 * @return float 显示器物理水平DPI。
 */
extern float kdk_edid_get_rawDpiX(char *name);

/**
 * @brief 获取显示器物理垂直DPI
 * 
 * @param name 显示器的当前接口
 * @return float 显示器物理垂直DPI。
 */
extern float kdk_edid_get_rawDpiY(char *name);

/**
 * @brief 获取显示器的刷新率
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器刷新率，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_refreshRate(char *name);

/**
 * @brief 获取显示器的方向
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的方向，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_rotation(char *name);

/**
 * @brief 获取显示器的序列号
 * 
 * @param name 显示器的当前接口
 * @return char* 成功返回显示器的序列号，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_edid_get_serialNumber(char *name);

/**
 * @brief 释放由kdk_edid_get_max_brightness和kdk_edid_get_current_brightness返回的显示器亮度
 * 
 * @param disk 由kdk_edid_get_max_brightness和kdk_edid_get_current_brightness返回的结构体指针
 */
extern void kdk_free_brightnessInfo(BrightnessInfo *info);

/**
 * @brief 释放由kdk_edid_get_red_primary
 *              kdk_edid_get_green_primary
 *              kdk_edid_get_blue_primary
 *              kdk_edid_get_white_primary返回的色度坐标结构体
 * 
 * @param disk 由kdk_edid_get_red_primary、kdk_edid_get_green_primary
 *              kdk_edid_get_blue_primary、kdk_edid_get_white_primary返回的结构体指针
 */
extern void kdk_free_chromaticityCoordinates(ChromaticityCoordinates *info);


#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kyedid-test.c
 * 
 */
