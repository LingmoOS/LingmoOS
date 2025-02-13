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
#include "details.h"

#include <QLocale>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#ifdef IN_TEST
#include <QStandardPaths>
#endif

PaddleOCRApp *PaddleOCRApp::instance()
{
    static PaddleOCRApp ocrInterface;
    return &ocrInterface;
}

//获取系统当前的语言
//当前策略：中文环境下，简体环境使用简体中文+英文模型，其余环境使用繁体模型；非中文环境下使用英文增强模型
PaddleOCRApp::Languages PaddleOCRApp::getSystemLang()
{
    QLocale locale;
    if (locale.language() == QLocale::Language::Chinese) {
        //查询当前国别代码
        QLocale::Country t_countryId = locale.country();
        if (t_countryId == QLocale::Country::China) {
            return Languages::CHI_SIM;
        } else {
            return Languages::CHI_TRA;
        }
    } else {
        return Languages::ENG;
    }
}

PaddleOCRApp::PaddleOCRApp()
{
    //初始化变量
    m_isRunning = false;

    //初始化识别模型路径
    //检测模型是全语种通用，不需要初始化
#ifdef IN_TEST
    QString rootPath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/ocr_test/testResource/");
#else
    QString rootPath("/usr/share/lingmo-ocr/model/"); //模型存放位置
#endif
    QString paramPath;  //模型结构文件路径
    QString binPath;    //权重文件路径
    QString dictPath;   //字典文件路径
    int recOutIndex = 0;//识别模型的输出位置（需要预先确认好，然后写进程序里）
    switch (getSystemLang()) {
    default: //使用英语
        paramPath = rootPath + "rec_eng.param.bin";
        binPath = rootPath + "rec_eng.bin";
        dictPath = "://assets/dict/dict_eng.txt";
        recOutIndex = 146;
        break;
    case Languages::CHI_TRA: //使用繁中
        paramPath = rootPath + "rec_chi_tra.param.bin";
        binPath = rootPath + "rec_chi_tra.bin";
        dictPath = "://assets/dict/dict_chi_tra.txt";
        recOutIndex = 146;
        break;
    case Languages::CHI_SIM: //使用简中
        paramPath = rootPath + "rec_chi_sim.param.bin";
        binPath = rootPath + "rec_chi_sim.bin";
        dictPath = "://assets/dict/dict_chi_sim.txt";
        recOutIndex = 78;
        break;
    }

    auto dict = loadDict(dictPath);

    //初始化神经网络
    ocrDetails = new Details(paramPath.toStdString().c_str(), binPath.toStdString().c_str(), dict, recOutIndex);
}

PaddleOCRApp::~PaddleOCRApp()
{
    delete ocrDetails;
}

std::vector<std::string> PaddleOCRApp::loadDict(const QString &dictPath)
{
    //字典初始化：字典文件的每一行都是一个单独的字符，但需要在开头额外插入一个不参与识别的占位符，同时在末尾插入空格
    QFile qrcFile(dictPath);
    qrcFile.open(QIODevice::ReadOnly);
    QTextStream stream(&qrcFile);
    std::vector<std::string> result;
    result.push_back("#");
    while (1) {
        auto temp = stream.readLine();
        if (temp.isEmpty()) {
            break;
        }
        result.push_back(temp.toStdString());
    }
    result.push_back(" ");
    return result;
}

QString PaddleOCRApp::getRecogitionResult(const QImage &image)
{
    m_isRunning = true;

    auto stdImg = image.convertToFormat(QImage::Format_RGB888).rgbSwapped(); //确保数据格式是BGR888以匹配模型
    cv::Mat mat = cv::Mat(stdImg.height(), stdImg.width(), CV_8UC3, stdImg.bits(), static_cast<size_t>(stdImg.bytesPerLine())).clone(); //转换到OpenCV格式
    auto result = ocrDetails->run(mat); //执行识别，获取结果

    //组装识别结果，注意：目前没有版面识别功能，只能这样简单堆叠，然后将结果刷到界面上
    QString text;
    std::for_each(result.begin(), result.end(), [&text](const std::string & eachText) {
        text += QString(eachText.c_str());
        text += "\n";
    });

    m_isRunning = false;
    return text;
}

void PaddleOCRApp::setLanguages(PaddleOCRApp::Languages data)
{

    if (ocrDetails)
    {
        delete ocrDetails;
        ocrDetails = nullptr;
    }

    //初始化识别模型路径
    //检测模型是全语种通用，不需要初始化
#ifdef IN_TEST
    QString rootPath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/ocr_test/testResource/");
#else
    QString rootPath("/usr/share/lingmo-ocr/model/"); //模型存放位置
#endif
    QString paramPath;  //模型结构文件路径
    QString binPath;    //权重文件路径
    QString dictPath;   //字典文件路径
    int recOutIndex = 0;//识别模型的输出位置（需要预先确认好，然后写进程序里）
    switch (data) {
    case Languages::CHI_TRA: //使用繁中
        paramPath = rootPath + "rec_chi_tra.param.bin";
        binPath = rootPath + "rec_chi_tra.bin";
        dictPath = "://assets/dict/dict_chi_tra.txt";
        recOutIndex = 146;
        break;
    case Languages::CHI_SIM: //使用简中
        paramPath = rootPath + "rec_chi_sim.param.bin";
        binPath = rootPath + "rec_chi_sim.bin";
        dictPath = "://assets/dict/dict_chi_sim.txt";
        recOutIndex = 78;
        break;
    default: //使用英语
        paramPath = rootPath + "rec_eng.param.bin";
        binPath = rootPath + "rec_eng.bin";
        dictPath = "://assets/dict/dict_eng.txt";
        recOutIndex = 146;
        break;
    }

    auto dict = loadDict(dictPath);

    //初始化神经网络
    ocrDetails = new Details(paramPath.toStdString().c_str(), binPath.toStdString().c_str(), dict, recOutIndex);
}
