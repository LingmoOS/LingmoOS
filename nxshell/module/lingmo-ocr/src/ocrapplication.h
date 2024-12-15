// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRAPPLICATION_H
#define OCRAPPLICATION_H

#include "mainwindow.h"
#include <QObject>
#include <QImage>

class OcrApplication : public QObject
{
    Q_OBJECT
public:
    explicit OcrApplication(QObject *parent = nullptr);

    Q_INVOKABLE bool openFile(QString filePath);

    Q_INVOKABLE void openImage(QImage image);

    Q_INVOKABLE void openImageAndName(QImage image, QString imageName);


signals:

public slots:

private:
    int m_loadingCount{0};//启动次数
};

#endif // OCRAPPLICATION_H
