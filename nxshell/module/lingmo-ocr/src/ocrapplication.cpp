// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrapplication.h"
#include "mainwindow.h"
#include "engine/OCREngine.h"
#include <DWidgetUtil>

OcrApplication::OcrApplication(QObject *parent) : QObject(parent)
{

}

bool OcrApplication::openFile(QString filePath)
{
    qDebug() << __FUNCTION__ << __LINE__ << filePath;
    bool bRet = false;
    if (!OCREngine::instance()->isRunning()) {
        MainWindow *win = new MainWindow();
        //增加判断，空图片不会启动
        bRet = win->openFile(filePath);
        if (bRet) {
            win->show();
            //第一次启动才居中
            if (m_loadingCount == 0) {
                Dtk::Widget::moveToCenter(win);
                m_loadingCount++;
            }
        }
    } else {
        qDebug() << "正在识别中！";
    }

    return bRet;
}

void OcrApplication::openImage(QImage image)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qDebug() << __FUNCTION__ << __LINE__ << image.size();
        if (!OCREngine::instance()->isRunning()) {
            MainWindow *win = new MainWindow();
            win->openImage(image);
            win->show();
            //第一次启动才居中
            if (m_loadingCount == 0) {
                Dtk::Widget::moveToCenter(win);
                m_loadingCount++;
            }
        } else {
            qDebug() << "正在识别中！";
        }
    }
}

void OcrApplication::openImageAndName(QImage image, QString imageName)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        if (!OCREngine::instance()->isRunning()) {
            MainWindow *win = new MainWindow();
            win->openImage(image, imageName);
            win->show();
            //第一次启动才居中
            if (m_loadingCount == 0) {
                Dtk::Widget::moveToCenter(win);
                m_loadingCount++;
            }
        } else {
            qDebug() << "正在识别中！";
        }
    }
}

