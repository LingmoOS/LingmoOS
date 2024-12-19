/*
* Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
*
* Author: WangZhengYang<wangzhengyang@uniontech.com>
*
* Maintainer: WangZhengYang<wangzhengyang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <vector>
#include <string>
#include <postprocess_op.h>
#include <utility.h>

namespace ncnn {
class Net;
}

class Details
{
public:
    Details(const char *recParamPath, const char *recBinPath, const std::vector<std::string> &dict, int recOut);
    ~Details();

    std::vector<std::string> run(const cv::Mat matrix);

private:
    std::vector<std::string> recognizeTexts(const std::vector<cv::Mat> &detectImg);
    std::string ctcDecode(const std::vector<float> &recNetOutputData, int h, int w);
    std::vector<std::vector<std::vector<int> > > detectText(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio);

    ncnn::Net *detNet; //检测网络
    ncnn::Net *recNet; //识别网络
    std::vector<std::string> keys; //字典
    int recOutIndex; //识别结果输出位置，默认的MobileNetV3模型出来的都是146

    PaddleOCR::PostProcessor postProcessor;
    PaddleOCR::Utility utilityTool;
};
