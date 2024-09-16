// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "OCREngine.h"
#include <DOcr>

OCREngine *OCREngine::instance()
{
    static OCREngine *ocr_detail = nullptr;
    if (ocr_detail == nullptr) {
        ocr_detail = new OCREngine;
    }
    return ocr_detail;
}

OCREngine::OCREngine()
{
    //初始化变量
    m_isRunning = false;

    //初始化插件管理库
    //此处存在产品设计缺陷: 无法选择插件，无鉴权入口，无性能方面的高级设置入口
    //因此此处直接硬编码使用默认插件
    ocrDriver = new Dtk::Ocr::DOcr;
    ocrDriver->loadDefaultPlugin();
    ocrDriver->setUseMaxThreadsCount(2);
    ocrDriver->setUseHardware({{Dtk::Ocr::HardwareID::GPU_Vulkan, 0}});
}

void OCREngine::setImage(const QImage &image)
{
    auto inputImage = image.convertToFormat(QImage::Format_RGB888);
    ocrDriver->setImage(image);
}

QString OCREngine::getRecogitionResult()
{
    m_isRunning = true;

    ocrDriver->analyze();

    m_isRunning = false;
    return ocrDriver->simpleResult();
}

bool OCREngine::setLanguage(const QString &language)
{
    if(ocrDriver->isRunning()) {
        ocrDriver->breakAnalyze();
    }

    return ocrDriver->setLanguage(language);
}
