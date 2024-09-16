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

#include <deepinocrplugin_p.h>
#include <deepinocrplugindef.h>
#include <postprocess_op.h>
#include <utility.h>

#include <opencv2/opencv.hpp>

#include <utility>
#include <atomic>

namespace ncnn {
    class Net;
}

class PaddleOCRApp : public DeepinOCRPlugin::Plugin
{
public:
    PaddleOCRApp();
    ~PaddleOCRApp() override;

    bool setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int> > &hardwareUsed) override;
    std::vector<DeepinOCRPlugin::HardwareID> getHardwareSupportList() override;
    bool setUseMaxThreadsCount(unsigned int n) override;
    std::vector<std::string> getImageFileSupportFormats() override;
    bool setImageFile(const std::string &filePath) override;
    DeepinOCRPlugin::PixelType getPixelType() override;
    bool setMatrix(int height, int width, unsigned char *data, size_t step) override;
    std::vector<std::string> getLanguageSupport() override;
    bool setLanguage(const std::string &language) override;
    bool analyze() override;
    bool breakAnalyze() override;
    std::vector<DeepinOCRPlugin::TextBox> getTextBoxes() override;
    std::vector<DeepinOCRPlugin::TextBox> getCharBoxes(size_t index) override;
    std::string getAllResult() override;
    std::string getResultFromBox(size_t index) override;

private:
    //推理过程控制
    std::string currentPath;
    std::atomic_bool needReset = false;
    std::atomic_bool needBreak = false;
    ncnn::Net *detNet = nullptr;
    ncnn::Net *recNet = nullptr;
    std::vector<std::string> keys;
    PaddleOCR::PostProcessor postProcessor;
    PaddleOCR::Utility utilityTool;
    void resetNet(); //重置网络
    void initNet();  //初始化网络
    std::vector<std::vector<std::vector<int>>> detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio);   //检测
    std::pair<std::string, std::vector<int>> ctcDecode(const std::vector<float> &recNetOutputData, int h, int w); //CTC解码
    void rec(const std::vector<cv::Mat> &detectImg); //识别
    std::vector<DeepinOCRPlugin::TextBox> lengthToBox(const std::vector<int> &lengths, std::pair<float, float> basePoint, float rectHeight, float ratio);

    //推理设置缓存
    cv::Mat imageCache;
    std::vector<std::string> supportLanguages = {"zh-Hans_en", "zh-Hant_en", "en"};
    std::vector<DeepinOCRPlugin::HardwareID> supportHardwares = {DeepinOCRPlugin::HardwareID::CPU_Any,
                                                                 DeepinOCRPlugin::HardwareID::GPU_Vulkan};
    std::vector<std::pair<DeepinOCRPlugin::HardwareID, int>> hardwareUseInfos;
    std::string languageUsed = "zh-Hans_en";
    unsigned int maxThreadsUsed = 1;

    //推理结果缓存
    std::vector<DeepinOCRPlugin::TextBox> textBoxes;
    std::vector<std::vector<DeepinOCRPlugin::TextBox>> charBoxes;
    std::string allResult;
    std::vector<std::string> boxesResult;
};
