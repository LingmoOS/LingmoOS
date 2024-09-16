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

#include "paddleocr.h"

#include <toolkits.h>

#include <ncnn/net.h>
#include <ncnn/layer.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <filesystem>

void *loadPlugin()
{
    return new PaddleOCRApp();
}

int unloadPlugin(void *plugin)
{
    delete reinterpret_cast<PaddleOCRApp *>(plugin);
    return 1;
}

int pluginVersion(void)
{
    return DeepinOCRPlugin::VERSION;
}

PaddleOCRApp::PaddleOCRApp()
{
    //获取资源路径位置
    std::string fullPath;
    const char *envDirs = std::getenv("XDG_DATA_DIRS");

    do {
        if(envDirs == nullptr) {
            break;
        }

        // strtok会破坏传入字符串，拷贝副本处理
        int len = strlen(envDirs);
        char *dirs = (char *)malloc(sizeof(char) * len);
        ::memcpy(dirs, envDirs, len);

        char *token = std::strtok(dirs, ":");
        while(token != nullptr) {
            std::string pathPrefix = token;
            if(pathPrefix[pathPrefix.size() - 1] != '/') {
                pathPrefix += '/';
            }
            fullPath = pathPrefix + "deepin-ocr-plugin-manager/model/";
            if(std::filesystem::exists(fullPath)) {
                currentPath = fullPath;
                break;
            }
            token = std::strtok(nullptr, ":");
        }

        free(dirs);
    }while(0);

    if(currentPath.empty()) {
        fullPath = "/usr/share/deepin-ocr-plugin-manager/model/";
        if(std::filesystem::exists(fullPath)) {
            currentPath = fullPath;
        } else {
            DEEPIN_LOG("cannot find default model");
        }
    }
}

PaddleOCRApp::~PaddleOCRApp()
{
    resetNet();
}

void PaddleOCRApp::resetNet()
{
    delete detNet;
    delete recNet;

    detNet = nullptr;
    recNet = nullptr;

    keys.clear();

    needReset = false;
}

void PaddleOCRApp::initNet()
{
    if(currentPath.empty()) {
        DEEPIN_LOG("cannot find default model");
        DEEPIN_LOG("model load failed");
    }

    if (detNet != nullptr && recNet != nullptr && !keys.empty()) {
        return;
    }

    //筛选可用的GPU设备
    int gpuCount = ncnn::get_gpu_count();
    std::set<int> gpuCanUseSet;
    if (gpuCount != 0) {
        for (auto &eachPair : hardwareUseInfos) {
            if (eachPair.first == DeepinOCRPlugin::HardwareID::GPU_Vulkan) {
                if (eachPair.second < gpuCount) {
                    gpuCanUseSet.insert(eachPair.second);
                }
            }
        }
    }
    std::vector<int> gpuCanUse(gpuCanUseSet.begin(), gpuCanUseSet.end());

    //多线程设置
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if(maxThreads <= 0) {
        maxThreads = 1;
    }
    if (maxThreadsUsed > maxThreads) {
        maxThreadsUsed = maxThreads;
    }

    //文件名后缀
    const std::string paramSuffix = ".param.bin";
    const std::string binSuffix = ".bin";
    const std::string dictSuffix = ".txt";

    //ncnn基础设置
    ncnn::Option option;
    option.lightmode = true;
    option.use_int8_inference = false;
    option.num_threads = 1;

    //初始化检测网络
    if (detNet == nullptr) {
        std::string detModel = currentPath + "det";

        detNet = new ncnn::Net;
        detNet->opt = option;
        detNet->opt.num_threads = static_cast<int>(maxThreadsUsed);
        detNet->load_param_bin((detModel + paramSuffix).c_str());
        detNet->load_model((detModel + binSuffix).c_str());
    }

    //初始化识别网络
    if (recNet == nullptr) {
        std::string recModel = currentPath + "rec_" + languageUsed;

        recNet = new ncnn::Net;
        recNet->opt = option;

        //由于检测网络的速度足够快，因此GPU设备仅给识别网络使用以节省GPU初始化时间
        if (!gpuCanUse.empty()) {
            recNet->set_vulkan_device(gpuCanUse[0]);
            recNet->opt.use_vulkan_compute = true;
        }

        recNet->load_param_bin((recModel + paramSuffix).c_str());
        recNet->load_model((recModel + binSuffix).c_str());
    }

    //初始化字典
    if (keys.empty()) {
        std::string dictFile = currentPath + languageUsed + dictSuffix;
        std::fstream fs;
        fs.open(dictFile, std::ios::in);
        std::string line;
        keys.emplace_back("#");
        while (getline(fs, line)) {
            keys.emplace_back(line);
        }
        keys.emplace_back(" ");
    }
}

std::vector<std::vector<std::vector<int>>> PaddleOCRApp::detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio)
{
    int w = src.cols;
    int h = src.rows;

    //1.缩减尺寸
    float ratio = 1.f;
    if (std::max(w, h) > 960) {
        if (h > w) {
            ratio = 960.0f / h;
        } else {
            ratio = 960.0f / w;
        }
    }

    int resizeH = int(h * ratio);
    int resizeW = int(w * ratio);

    //2.扩大尺寸，确保图片的最短的边大于等于64,小于这个值可能会导致检测不到文字
    float ratio2 = 1.f;
    if (std::min(resizeW, resizeH) < 64) {
        if (resizeH > resizeW) {
            ratio2 = 64.0f / resizeW;
        } else {
            ratio2 = 64.0f / resizeH;
        }
    }
    resizeH = int(resizeH * ratio2);
    resizeW = int(resizeW * ratio2);

    //3.标准化图片尺寸，否则会错位
    resizeH = int(round(float(resizeH) / 32) * 32);
    resizeW = int(round(float(resizeW) / 32) * 32);

    //执行resize，记录变换比例
    cv::Mat resize_img;
    cv::resize(src, resize_img, cv::Size(resizeW, resizeH));
    float ratio_h = float(resizeH) / float(h);
    float ratio_w = float(resizeW) / float(w);

    //执行推理
    ncnn::Mat in_pad = ncnn::Mat::from_pixels(resize_img.data, ncnn::Mat::PIXEL_RGB, resizeW, resizeH);

    const float meanValues[3] = { 0.485f * 255, 0.456f * 255, 0.406f * 255 };
    const float normValues[3] = { 1.0f / 0.229f / 255.0f, 1.0f / 0.224f / 255.0f, 1.0f / 0.225f / 255.0f };

    in_pad.substract_mean_normalize(meanValues, normValues);
    ncnn::Extractor extractor = detNet->create_extractor();

    extractor.input(0, in_pad);
    ncnn::Mat out;
    extractor.extract(detNet->output_indexes()[0], out);

    if(needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    //解码位置数据
    //注意：thresh, boxThresh, unclipRatio三个参数将极大影响解码效果，进而会影响后面识别网络的输出结果
    std::vector<float> pred;
    std::vector<unsigned char> cbuf;

    for (int q = 0; q < out.c; q++) {
        const float *ptr = out.channel(q);
        for (int y = 0; y < out.h; y++) {
            for (int x = 0; x < out.w; x++) {
                pred.push_back(ptr[x]);
                cbuf.push_back(static_cast<unsigned char>(ptr[x] * 255.0f));
            }
            ptr += out.w;
        }
    }

    if(needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    cv::Mat cbuf_map(out.h, out.w, CV_8UC1, static_cast<unsigned char *>(cbuf.data()));
    cv::Mat pred_map(out.h, out.w, CV_32F, static_cast<float *>(pred.data()));

    const float threshold = thresh * 255.0f;
    cv::Mat bit_map;
    cv::threshold(cbuf_map, bit_map, static_cast<double>(threshold), 255, cv::THRESH_BINARY);
    cv::Mat dilation_map;
    cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::dilate(bit_map, dilation_map, dila_ele);

    if(needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    auto result = postProcessor.BoxesFromBitmap(pred_map, dilation_map, boxThresh, unclipRatio, false);

    if(needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    result = postProcessor.FilterTagDetRes(result, ratio_h, ratio_w, src);

    return result;
}

std::pair<std::string, std::vector<int>> PaddleOCRApp::ctcDecode(const std::vector<float> &recNetOutputData, int h, int w)
{
    std::string text;
    std::vector<int> baseSize;
    int currentSize = 0;
    int status = 0;
    size_t lastIndex = 0;
    for (int i = 0; i < h; i++) {
        size_t maxIndex = 0;
        maxIndex = utilityTool.argmax(recNetOutputData.begin() + i * w, recNetOutputData.begin() + i * w + w);
        ++currentSize;
        //CTC特性：连续相同即判定为同一个字，在判定为下一字的时候，之前的积累就会变成上一个字的长度
        if (maxIndex > 0 && (i == 0 || maxIndex != lastIndex)) {
            text.append(keys[static_cast<size_t>(maxIndex)]);

            if (status == 0) {
                status = 1;
            } else {
                baseSize.push_back(currentSize - 1);
                currentSize = 1;
            }
        }
        lastIndex = maxIndex;
    }
    baseSize.push_back(currentSize);
    return std::make_pair(text, baseSize);
}

std::vector<DeepinOCRPlugin::TextBox> PaddleOCRApp::lengthToBox(const std::vector<int> &lengths, std::pair<float, float> basePoint, float rectHeight, float ratio)
{
    std::vector<DeepinOCRPlugin::TextBox> result;
    float currentPos = basePoint.first;
    float yAxisT = basePoint.second;
    float yAxisB = basePoint.second + rectHeight;
    for (auto &eachLen : lengths) {
        DeepinOCRPlugin::TextBox temp;
        temp.angle = 0; //倾斜角不可用
        temp.points.push_back(std::make_pair(currentPos, yAxisT));
        temp.points.push_back(std::make_pair(currentPos + eachLen * 4 / ratio, yAxisT));
        temp.points.push_back(std::make_pair(currentPos + eachLen * 4 / ratio, yAxisB));
        temp.points.push_back(std::make_pair(currentPos, yAxisB));
        result.push_back(temp);
        currentPos += eachLen * 4 / ratio;
    }
    return result;
}

void PaddleOCRApp::rec(const std::vector<cv::Mat> &detectImg)
{
    size_t size = detectImg.size();
    allResult.clear();
    std::vector<std::string> allResultVec(detectImg.size());
    boxesResult.resize(detectImg.size());
    charBoxes.resize(detectImg.size());

    //带LSTM的模型在外面开多线程加速效果会比在里面开多线程加速好
    #pragma omp parallel for num_threads(maxThreadsUsed)
    for (size_t i = 0; i < size; ++i) {
        if(needBreak) {
            continue;
        }

        //输入图片固定高度32
        float ratio = static_cast<float>(detectImg[i].cols) / static_cast<float>(detectImg[i].rows);
        int imgW = static_cast<int>(32 * ratio);
        int resize_w;
        if (ceilf(32 * ratio) > imgW)
            resize_w = imgW;
        else
            resize_w = static_cast<int>(ceilf(32 * ratio));

        cv::Mat stdMat;
        cv::resize(detectImg[i], stdMat, cv::Size(resize_w, 32), 0, 0, cv::INTER_LINEAR);
        cv::copyMakeBorder(stdMat, stdMat, 0, 0, 0, int(imgW - stdMat.cols), cv::BORDER_CONSTANT, {127, 127, 127});

        float realRatio = static_cast<float>(stdMat.cols) / detectImg[i].cols;

        if(needBreak) {
            continue;
        }

        ncnn::Mat input = ncnn::Mat::from_pixels(stdMat.data, ncnn::Mat::PIXEL_RGB, stdMat.cols, stdMat.rows);
        const float mean_vals[3] = { 127.5, 127.5, 127.5 };
        const float norm_vals[3] = { 1.0f / 127.5f, 1.0f / 127.5f, 1.0f / 127.5f };
        input.substract_mean_normalize(mean_vals, norm_vals);

        auto outIndexes = recNet->output_indexes();
        ncnn::Extractor extractor = recNet->create_extractor();

        if(recNet->opt.use_vulkan_compute) {
            //当可用线程 > 1 同时不是第 1 个线程时，使用CPU进行计算
            //即确保显卡只处理单次的推理
            if(maxThreadsUsed > 1 && i % maxThreadsUsed != 1) {
                extractor.set_vulkan_compute(false);
            }
        }

        extractor.input(0, input);
        ncnn::Mat out;
        extractor.extract(outIndexes[outIndexes.size() - 1], out);

        if(needBreak) {
            continue;
        }

        //读取数据，执行CTC算法解析数据
        float *floatArray = static_cast<float *>(out.data);
        std::vector<float> recNetOutputData(floatArray, floatArray + out.h * out.w);

        auto ctcResult = ctcDecode(recNetOutputData, out.h, out.w);

        //总体识别结果收集
        allResultVec[i] = ctcResult.first;

        //文本块识别结果收集
        boxesResult[i] = ctcResult.first;

        //精确字符位置结果收集
        auto box = textBoxes[i];
        auto baseSize = ctcResult.second;
        auto currentCharBox = lengthToBox(baseSize, textBoxes[i].points[0], box.points[2].second - box.points[0].second, realRatio);
        charBoxes[i] = currentCharBox;

        if(needBreak) {
            continue;
        }
    }

    //总体识别结果存入
    for(const auto &eachResult : allResultVec) {
        allResult += eachResult;
        allResult += "\n";
    }
}

bool PaddleOCRApp::setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int>> &hardwareUsed)
{
    needReset = true;
    hardwareUseInfos = hardwareUsed;
    return true;
}

std::vector<DeepinOCRPlugin::HardwareID> PaddleOCRApp::getHardwareSupportList()
{
    return supportHardwares;
}

bool PaddleOCRApp::setUseMaxThreadsCount(unsigned int n)
{
    needReset = true;
    maxThreadsUsed = n;
    return true;
}

std::vector<std::string> PaddleOCRApp::getImageFileSupportFormats()
{
    return {"BMP", "JPEG", "PNG", "PBM", "PGM", "PPM"};
}

bool PaddleOCRApp::setImageFile(const std::string &filePath)
{
    imageCache = cv::imread(filePath);
    return imageCache.data != nullptr;
}

DeepinOCRPlugin::PixelType PaddleOCRApp::getPixelType()
{
    return DeepinOCRPlugin::PixelType::Pixel_BGR;
}

bool PaddleOCRApp::setMatrix(int height, int width, unsigned char *data, size_t step)
{
    imageCache = cv::Mat(height, width, CV_8UC3, data, step).clone();
    return true;
}

std::vector<std::string> PaddleOCRApp::getLanguageSupport()
{
    return supportLanguages;
}

bool PaddleOCRApp::setLanguage(const std::string &language)
{
    if (std::find(supportLanguages.begin(), supportLanguages.end(), language) == supportLanguages.end()) {
        return false;
    } else {
        languageUsed = language;
        needReset = true;
        return true;
    }
}

bool PaddleOCRApp::analyze()
{
    //初始化
    if (needReset) {
        resetNet();
    }
    initNet();

    do {
        //检测
        auto boxes = detect(imageCache, 0.3f, 0.5f, 1.6f);

        if(needBreak) {
            break;
        }

        //排序
        std::sort(boxes.begin(), boxes.end(), [](const std::vector<std::vector<int>> &boxL, const std::vector<std::vector<int>> &boxR) {
            //左侧
            int x_collect_L[4] = {boxL[0][0], boxL[1][0], boxL[2][0], boxL[3][0]};
            int y_collect_L[4] = {boxL[0][1], boxL[1][1], boxL[2][1], boxL[3][1]};

            //右侧
            int x_collect_R[4] = {boxR[0][0], boxR[1][0], boxR[2][0], boxR[3][0]};
            int y_collect_R[4] = {boxR[0][1], boxR[1][1], boxR[2][1], boxR[3][1]};

            //判断顺序：先上下，后左右

            //完全超过时，在上面的靠前，在下面的靠后
            int y_L = *std::min_element(y_collect_L, y_collect_L + 4);
            int height_L = *std::max_element(y_collect_L, y_collect_L + 4) - y_L;
            int y_R = *std::min_element(y_collect_R, y_collect_R + 4);
            int height_R = *std::max_element(y_collect_R, y_collect_R + 4) - y_R;
            if (y_R - y_L > height_R / 3.0f * 2.0f) {
                return true;
            } else if (y_L - y_R > height_L / 3.0f * 2.0f) {
                return false;
            }

            //部分超过时，在左边的靠前，在右边的靠后（TODO：如果是维语/阿拉伯语，则需要反过来）
            //注意：由于检测算法的机制，各个矩形框按理来说不会出现重叠
            int x_L = *std::min_element(x_collect_L, x_collect_L + 4);
            int x_R = *std::min_element(x_collect_R, x_collect_R + 4);
            if (x_L < x_R) {
                return true;
            } else {
                return false;
            }
        });

        if(needBreak) {
            break;
        }

        //整理成可输出的格式
        textBoxes.clear();
        for (auto &eachBox : boxes) {
            //校准矩形框

            //上
            eachBox[0][1] = std::min(eachBox[0][1], eachBox[1][1]);
            eachBox[1][1] = std::min(eachBox[0][1], eachBox[1][1]);

            //下
            eachBox[2][1] = std::max(eachBox[2][1], eachBox[3][1]);
            eachBox[3][1] = std::max(eachBox[2][1], eachBox[3][1]);

            //左
            eachBox[0][0] = std::min(eachBox[0][0], eachBox[3][0]);
            eachBox[3][0] = std::min(eachBox[0][0], eachBox[3][0]);

            //右
            eachBox[1][0] = std::max(eachBox[1][0], eachBox[2][0]);
            eachBox[2][0] = std::max(eachBox[1][0], eachBox[2][0]);

            //存入数据
            DeepinOCRPlugin::TextBox temp;
            temp.points.push_back(make_pair(eachBox[0][0], eachBox[0][1]));
            temp.points.push_back(make_pair(eachBox[1][0], eachBox[1][1]));
            temp.points.push_back(make_pair(eachBox[2][0], eachBox[2][1]));
            temp.points.push_back(make_pair(eachBox[3][0], eachBox[3][1]));
            temp.angle = 0; //倾斜角不可用
            textBoxes.push_back(temp);
        }

        if(needBreak) {
            break;
        }

        //裁切
        std::vector<cv::Mat> images;
        std::transform(boxes.begin(), boxes.end(), std::back_inserter(images), [this](const std::vector<std::vector<int>> &box) {
            return utilityTool.GetRotateCropImage(imageCache, box);
        });

        if(needBreak) {
            break;
        }

        //识别
        rec(images);
    }while(0);

    if(needBreak) {
        textBoxes.clear();
        charBoxes.clear();
        allResult.clear();
        boxesResult.clear();
        needBreak = false;
        return false;
    } else {
        //对识别结果进行最后清理，将未识别到文字的检测框排除掉
        for(uint32_t i = 0;i != boxesResult.size();++i) {
            if(boxesResult[i].empty()) {
                boxesResult.erase(boxesResult.begin() + i);
                textBoxes.erase(textBoxes.begin() + i);
                charBoxes.erase(charBoxes.begin() + i);
                --i;
            }
        }

        return !textBoxes.empty();
    }
}

bool PaddleOCRApp::breakAnalyze()
{
    if(!needBreak) {
        needBreak = true;
        return true;
    } else {
        return false;
    }
}

std::vector<DeepinOCRPlugin::TextBox> PaddleOCRApp::getTextBoxes()
{
    return textBoxes;
}

std::vector<DeepinOCRPlugin::TextBox> PaddleOCRApp::getCharBoxes(size_t index)
{
    if (index >= charBoxes.size()) {
        return std::vector<DeepinOCRPlugin::TextBox>();
    } else {
        return charBoxes[index];
    }
}

std::string PaddleOCRApp::getAllResult()
{
    return allResult;
}

std::string PaddleOCRApp::getResultFromBox(size_t index)
{
    return boxesResult[index];
}
