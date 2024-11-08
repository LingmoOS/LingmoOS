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

#ifndef LIBKYJIMAGEPROC_H
#define LIBKYJIMAGEPROC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 初始化jpeg编码器
 * 
 * @return void* 返回编码器句柄
 */
extern void* kdk_jpeg_encoder_init();

/**
 * @brief jpeg图编码接口
 * 
 * @param handle 编码器句柄
 * @param srcFile 源文件 
 * @param destFile 输出文件 
 * @return int -1 则编码失败，0，编码成功
 */
extern int kdk_jpeg_encode_to_file(void *handle, char *srcFile, char *destFile);

/**
 * @brief 释放编码器内存
 * 
 * @param handle 编码器句柄
 */
extern void kdk_jpeg_encoder_release(void* handle);


#ifdef __cplusplus
}
#endif

#endif // LIBKYJIMAGEPROC_H
