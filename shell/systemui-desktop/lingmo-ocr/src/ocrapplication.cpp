/*
* Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
*
* Author: He MingYang Hao<hemingyang@uniontech.com>
*
* Maintainer: He MingYang <hemingyang@uniontech.com>
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

#include "ocrapplication.h"
#include "ocr.h"
#include "paddleocr-ncnn/paddleocr.h"
//#include <DWidgetUtil>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

static OcrApplication * ocrApp =nullptr;
OcrApplication *OcrApplication::instance()
{
    if(!ocrApp){
        ocrApp = new OcrApplication();
    }
    return ocrApp;
}

OcrApplication::OcrApplication(QObject *parent) : QObject(parent)
{
    qmlRegisterType<Ocr>("Lingmo.Ocr", 1, 0, "Ocr");
    m_engine.addImportPath(QStringLiteral("qrc:/"));
    m_engine.load(QUrl(QStringLiteral("qrc:/src/qml/main.qml")));
}

bool OcrApplication::openFile(QString filePath)
{
    qDebug() << __FUNCTION__ << __LINE__ << filePath;
    QObject *mainObject = m_engine.rootObjects().first();
    bool bRet = false;
    if (!PaddleOCRApp::instance()->isRunning()) {
        if (mainObject) {
            QMetaObject::invokeMethod(mainObject, "openImage", Q_ARG(QVariant, filePath));
        }
    } else {
        qDebug() << "正在识别中！";
    }

    return bRet;
}

void OcrApplication::openImage(QImage image)
{
    QObject *mainObject = m_engine.rootObjects().first();
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qDebug() << __FUNCTION__ << __LINE__ << image.size();
        if (!PaddleOCRApp::instance()->isRunning()) {
            Ocr *win = new Ocr();
            win->openImage(image);
        } else {
            qDebug() << "正在识别中！";
        }
    }
}

void OcrApplication::openImageAndName(QImage image, QString imageName)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        if (!PaddleOCRApp::instance()->isRunning()) {
            Ocr *win = new Ocr();
            win->openImage(image, imageName);
        } else {
            qDebug() << "正在识别中！";
        }
    }
}

