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


#ifndef OCRAPPLICATION_H
#define OCRAPPLICATION_H
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ocr.h"
#include <QObject>
#include <QImage>

class OcrApplication : public QObject
{
    Q_OBJECT
public:
    static OcrApplication *instance ();

    Q_INVOKABLE bool openFile(QString filePath);

    Q_INVOKABLE void openImage(QImage image);

    Q_INVOKABLE void openImageAndName(QImage image, QString imageName);


signals:

public slots:

private:
    explicit OcrApplication(QObject *parent = nullptr);
    QQmlApplicationEngine m_engine;
    int m_loadingCount{0};//启动次数
};

#endif // OCRAPPLICATION_H
