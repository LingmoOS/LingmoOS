// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTime>
#include <QDebug>
#include "performancemonitor.h"

const QString LOG_FLAG = "[PerformanceMonitor]";

const QString GRAB_POINT = "[GRABPOINT]";
const QString APP_NAME = "DEEPIN_FONT_MANAGER";
const QString INIT_APP_TIME = "0001";
const QString LOAD_FONTS_TIME = "0002";
const QString INSTALL_FONTS_TIME = "0003";
const QString EXPORT_FONTS_TIME = "0004";
const QString DELETE_FONTS_TIME = "0005";
const QString FAVORITE_FONTS_TIME = "0006";
const QString UNFAVORITE_FONTS_TIME = "0010";
const QString INIT_AND_LOAD_TIME = "0007";
const QString ACTIVE_FONTS_TIME = "0008";
const QString DEACTIVE_FONTS_TIME = "0009";

qint64 PerformanceMonitor::initializeAppStartMs = 0;
qint64 PerformanceMonitor::initializeAppFinishMs = 0;
qint64 PerformanceMonitor::loadFontStartMs = 0;
qint64 PerformanceMonitor::loadFontFinishMs = 0;
qint64 PerformanceMonitor::installFontStartMs = 0;
qint64 PerformanceMonitor::installFontFinishMs = 0;
qint64 PerformanceMonitor::exportFontStartMs = 0;
qint64 PerformanceMonitor::exportFontFinishMs = 0;
qint64 PerformanceMonitor::deleteFontStartMs = 0;
qint64 PerformanceMonitor::deleteFontFinishMs = 0;
qint64 PerformanceMonitor::favoriteFontStartMs = 0;
qint64 PerformanceMonitor::favoriteFontFinishMs = 0;
qint64 PerformanceMonitor::activeFontStartMs = 0;
qint64 PerformanceMonitor::activeFontFinishMs = 0;

PerformanceMonitor::PerformanceMonitor(QObject *parent) : QObject(parent)
{
}

/*************************************************************************
 <Function>     initializeMainDialogStart
 <Description>  记录程序初始化开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::initializeAppStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to initialize app";
    initializeAppStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     initializeAppFinish
 <Description>  记录程序初始化结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::initializeAppFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to initialize app";

    initializeAppFinishMs = current.toMSecsSinceEpoch();
    qint64 time = initializeAppFinishMs - initializeAppStartMs;
    qInfo() << QString("%1 %2-%3 %4 #(Init app time)").arg(GRAB_POINT).arg(APP_NAME).arg(INIT_APP_TIME).arg(time);
}

/*************************************************************************
 <Function>     loadFontStart
 <Description>  记录程序启动时，开始加载字体时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::loadFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to load fonts";
    loadFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     loadFontFinish
 <Description>  记录程序启动时，完成加载字体时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::loadFontFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to load fonts";

    loadFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time1 = loadFontFinishMs - loadFontStartMs;
    qInfo() << QString("%1 %2-%3 %4 #(Load fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(LOAD_FONTS_TIME).arg(time1);
    qint64 time2 = loadFontFinishMs - initializeAppStartMs;
    qInfo() << QString("%1 %2-%3 %4 #(Init and load time)").arg(GRAB_POINT).arg(APP_NAME).arg(INIT_AND_LOAD_TIME).arg(time2);
}

/*************************************************************************
 <Function>     installFontStart
 <Description>  记录安装字体的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::installFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to install fonts";
    installFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     installFontFinish
 <Description>  记录安装字体的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>   fontCount 安装字体个数
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::installFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to install fonts";

    installFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = installFontFinishMs - installFontStartMs;
    qInfo() << QString("%1 %2-%3 %4 %5 #(Install fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(INSTALL_FONTS_TIME).arg(fontCount).arg(time);
}

///*************************************************************************
// <Function>     calcInitAndLoadTime
// <Description>  统计程序启动到字体加载完成的时间
// <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
// <Maintainer>
// <Input>
// <Return>
// <Note>
//*************************************************************************/
//qint64 PerformanceMonitor::calcInitAndLoadTime()
//{
//    qint64 time = loadFontFinishMs - initializeAppStartMs;
//    qDebug() << LOG_FLAG << "init app and load fonts cost time(ms): " << time;
//    return time;
//}

/*************************************************************************
 <Function>     exportFontStart
 <Description>  记录字体导出的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::exportFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to export fonts";
    exportFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     exportFontFinish
 <Description>  记录字体导出的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>   fontCount 导出字体个数
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::exportFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to export fonts";

    exportFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = exportFontFinishMs - exportFontStartMs;
    qInfo() << QString("%1 %2-%3 %4 %5 #(Export fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(EXPORT_FONTS_TIME).arg(fontCount).arg(time);
}

/*************************************************************************
 <Function>     deleteFontStart
 <Description>  记录删除字体的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::deleteFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to delete fonts";
    deleteFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     deleteFontFinish
 <Description>  记录删除字体的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>        fontCount 删除字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::deleteFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to delete fonts";

    deleteFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = deleteFontFinishMs - deleteFontStartMs;
    qInfo() << QString("%1 %2-%3 %4 %5 #(delete fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(DELETE_FONTS_TIME).arg(fontCount).arg(time);
}

/*************************************************************************
 <Function>     activeFontStart
 <Description>  激活字体开始时间
 <Author>
 <Maintainer>
 <Input>        fontCount 激活字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::activeFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to active fonts";
    activeFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     activeFontFinish
 <Description>  激活字体结束时间
 <Author>
 <Maintainer>
 <Input>        isActive  激活或禁用
 <Input>        fontCount 激活字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::activeFontFinish(bool isDeActive, int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to active or deactive fonts";

    activeFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = activeFontFinishMs - activeFontStartMs;
    if (isDeActive)
        qInfo() << QString("%1 %2-%3 %4 %5 #(deactive fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(DEACTIVE_FONTS_TIME).arg(fontCount).arg(time);
    else {
        qInfo() << QString("%1 %2-%3 %4 %5 #(active fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(ACTIVE_FONTS_TIME).arg(fontCount).arg(time);
    }
}

/*************************************************************************
 <Function>     favoriteFontStart
 <Description>  收藏开始时间
 <Author>
 <Maintainer>
 <Input>        fontCount 收藏字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::favoriteFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to favorite fonts";
    favoriteFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     favoriteFontFinish
 <Description>  收藏结束时间
 <Author>
 <Maintainer>
 <Input>        isFavorite 收藏或取消收藏
 <Input>        fontCount 收藏字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::favoriteFontFinish(bool isFavorite, int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to favorite fonts";

    favoriteFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = favoriteFontFinishMs - favoriteFontStartMs;
    if (isFavorite)
        qInfo() << QString("%1 %2-%3 %4 %5 #(Favorite fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(FAVORITE_FONTS_TIME).arg(fontCount).arg(time);
    else {
        qInfo() << QString("%1 %2-%3 %4 %5 #(UnFavorite fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(UNFAVORITE_FONTS_TIME).arg(fontCount).arg(time);
    }
}
