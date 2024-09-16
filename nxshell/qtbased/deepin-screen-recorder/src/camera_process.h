// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERA_PROCESS_H
#define CAMERA_PROCESS_H

#include <QObject>
#include <DWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QList>
#include <QDebug>
#include <QImage>


class CameraProcess : public QObject
{
    Q_OBJECT
public:
    explicit CameraProcess(QObject *parent = nullptr);
    ~CameraProcess();
    //static bool checkCameraAvailability();

signals:

public slots:
    void ShowTheCapture();
    //void displayImage(int, QImage image);

private:
    QCamera *camera;//摄像头
    QCameraViewfinder *viewfinder; //摄像头取景器部件
    QCameraImageCapture *imageCapture; //截图部件
};

#endif // CAMERA_PROCESS_H
