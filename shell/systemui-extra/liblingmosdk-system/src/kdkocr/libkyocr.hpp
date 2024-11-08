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

#ifndef LINGMOOCR_H
#define LINGMOOCR_H

/**
 * @file libkyocr.hpp
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief OCR 文字识别功能
 * @version 0.1
 * @date 2022-04-07
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-ocr
 * @{
 * 
 */

#include <iostream>
#include <string>
#include <vector>

// namespace kdk {

//     class kdkOCR {
//     public:

//         std::vector<std::vector<std::vector<int>>> getRect(const std::string &imagePath);

//         std::vector<std::string> getCls(const std::string &imagePath, int nums);



//     };
// }

namespace kdk
{
    /**
     * @brief ocr的识别结果
     *
     */
    struct TOcrResult
    {
        std::string imagePath;
        volatile bool hasGotRect;
        std::vector<std::vector<std::vector<int>>> textRects;
        std::vector<std::string> texts;

        TOcrResult();

        void reset();

        bool isNewPath(const std::string &strImagePath) const;
    };

    /**
     * @brief OCR文字识别功能类
     * 主要用来获取文字框和获取文字内容
     */
    class kdkOCR
    {
    protected:
        TOcrResult objOcrRet;

    public:
        kdkOCR();
        
        /**
         * @brief 获取文字框
         * 
         * @param String 图片文件路径
         * @return Vector<vector<vector<vector<int> 文字框点的坐标矩阵、以及文字框的个数
         */
        std::vector<std::vector<std::vector<int>>> getRect(const std::string &imagePath);

        /**
         * @brief 获取文字内容
         * 
         * @param String 图片文件路径
         * @param nums 同时处理的文字栈个数
         * @return Vector<string> 图片中的文字字符串
         */
        std::vector<std::string> getCls(const std::string &imagePath, int nums);

    protected:
        void handleImage(const std::string &imagePath);
    };
}

extern "C" {
        /**
         * @brief 获取文字框
         * 
         * @param String 图片文件路径
         * @return Vector<vector<vector<vector<int> 文字框点的坐标矩阵、以及文字框的个数
         */
        std::vector<std::vector<std::vector<int>>> getRect(const std::string &imagePath) {
            kdk::kdkOCR obj;
            return obj.getRect(imagePath);
        }

        /**
         * @brief 获取文字内容
         * 
         * @param String 图片文件路径
         * @param nums 同时处理的文字栈个数
         * @return Vector<string> 图片中的文字字符串
         */
        std::vector<std::string> getCls(const std::string &imagePath, int nums) {
                            kdk::kdkOCR obj;
                return obj.getCls(imagePath, nums);
        }
}

#endif //LINGMOOCR_H
