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

#include "details.h"

// ncnn
#include "layer.h"
#include "net.h"

#ifdef IN_TEST
#include <QStandardPaths>
#endif

std::vector<std::vector<std::vector<int>>> Details::detectText(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio)
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
    extractor.extract(137, out);

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

    cv::Mat cbuf_map(out.h, out.w, CV_8UC1, static_cast<unsigned char *>(cbuf.data()));
    cv::Mat pred_map(out.h, out.w, CV_32F, static_cast<float *>(pred.data()));

    const float threshold = thresh * 255.0f;
    cv::Mat bit_map;
    cv::threshold(cbuf_map, bit_map, static_cast<double>(threshold), 255, cv::THRESH_BINARY);
    cv::Mat dilation_map;
    cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::dilate(bit_map, dilation_map, dila_ele);

    auto result = postProcessor.BoxesFromBitmap(pred_map, dilation_map, boxThresh, unclipRatio, false);

    result = postProcessor.FilterTagDetRes(result, ratio_h, ratio_w, src);

    return result;
}

std::string Details::ctcDecode(const std::vector<float> &recNetOutputData, int h, int w)
{
    std::string text;
    size_t lastIndex = 0;
    for (int i = 0; i < h; i++) {
        size_t maxIndex = 0;
        maxIndex = utilityTool.argmax(recNetOutputData.begin() + i * w, recNetOutputData.begin() + i * w + w);
        if (maxIndex > 0 && (i == 0 || maxIndex != lastIndex)) { //CTC特性：连续相同即判定为同一个字
            text.append(keys[static_cast<size_t>(maxIndex)]);
        }
        lastIndex = maxIndex;
    }
    return text;
}

std::vector<std::string> Details::recognizeTexts(const std::vector<cv::Mat> &detectImg)
{
    size_t size = detectImg.size();
    std::vector<std::string> textLines(size);

    //带LSTM的模型在外面开多线程加速效果会比在里面开多线程加速好
    #pragma omp parallel for num_threads(2)
    for (size_t i = 0; i < size; ++i) {
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

        //保存传入的检测结果，debug用
        /*static int i = 0;
        char saveStr[7];
        std::sprintf(saveStr, "%d.png", i++);
        cv::imwrite(saveStr, stdMat);*/

        ncnn::Mat input = ncnn::Mat::from_pixels(stdMat.data, ncnn::Mat::PIXEL_RGB, stdMat.cols, stdMat.rows);
        const float mean_vals[3] = { 127.5, 127.5, 127.5 };
        const float norm_vals[3] = { 1.0f / 127.5f, 1.0f / 127.5f, 1.0f / 127.5f };
        input.substract_mean_normalize(mean_vals, norm_vals);

        ncnn::Extractor extractor = recNet->create_extractor();
        extractor.input(0, input);
        ncnn::Mat out;
        extractor.extract(recOutIndex, out);

        //读取数据，执行CTC算法解析数据
        float *floatArray = static_cast<float *>(out.data);
        std::vector<float> recNetOutputData(floatArray, floatArray + out.h * out.w);
        textLines[i] = ctcDecode(recNetOutputData, out.h, out.w);
    }

    return textLines;
}

Details::Details(const char *recParamPath, const char *recBinPath, const std::vector<std::string> &dict, int recOut)
{
    ncnn::Option opt;
    opt.lightmode = true; //最小化内存占用
    opt.num_threads = 2;  //神经网络推理过程中最多只开2个线程

    //初始化检测网络
    detNet = new ncnn::Net;
    detNet->opt = opt;
#ifdef IN_TEST
    detNet->load_param_bin((QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/ocr_test/testResource/det.param.bin").toStdString().c_str());
    detNet->load_model((QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/ocr_test/testResource/det.bin").toStdString().c_str());
#else
    detNet->load_param_bin("/usr/share/lingmo-ocr/model/det.param.bin");
    detNet->load_model("/usr/share/lingmo-ocr/model/det.bin");
#endif

    //初始化识别网络
    recNet = new ncnn::Net;
    recNet->opt = opt;
    recNet->load_param_bin(recParamPath);
    recNet->load_model(recBinPath);

    //加载字典
    keys = dict;

    //设置识别结果位置
    recOutIndex = recOut;
}

Details::~Details()
{
    delete detNet;
    delete recNet;
}

std::vector<std::string> Details::run(const cv::Mat matrix)
{
    //1.获取文本位置
    auto boxes = detectText(matrix, 0.3f, 0.5f, 1.6f);

    //2.执行排序
    //限于开源协议，暂时无法采用更高效的策略
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

    //3.获取对应位置的图片
    std::vector<cv::Mat> images;
    std::transform(boxes.begin(), boxes.end(), std::back_inserter(images), [matrix, this](const std::vector<std::vector<int>> &box) {
        return utilityTool.GetRotateCropImage(matrix, box);
    });

    //4.对每一张图片进行识别
    std::vector<std::string> recResults = recognizeTexts(images);

    return recResults;
}
