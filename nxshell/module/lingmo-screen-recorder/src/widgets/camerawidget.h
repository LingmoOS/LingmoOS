// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H
#include "../camera/majorimageprocessingthread.h"

#include <QObject>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QList>
#include <QDebug>
#include <QImage>
#include <QTimer>
#include <DLabel>
#include <DWidget>
#include <QFile>

DWIDGET_USE_NAMESPACE

class CameraWidget : public DWidget
{
    Q_OBJECT
public:
    enum Position {
        leftTop,
        leftBottom,
        rightTop,
        rightBottom,
    };
public:
    explicit CameraWidget(DWidget *parent = nullptr);
    ~CameraWidget();
    /**
     * @brief 设置录屏区域范围
     * @param x: 录屏区域起点x坐标
     * @param y: 录屏区域起点y坐标
     * @param width: 录屏区域宽度
     * @param height: 录屏区域高度
     */
    void setRecordRect(int x, int y, int width, int height);
    /**
     * @brief 在指定位置显示摄像头界面
     * @param pos: 指定位置
     */
    void showAt(QPoint pos);
    int getRecordX();
    int getRecordY();
    int getRecordWidth();
    int getRecordHeight();

    /**
     * @brief 初始化UI界面
     */
    void initUI();

    /**
     * @brief 初始化camera，使用v4l2流程
     */
    void cameraStart();

    /**
     * @brief 停止采集摄像头画面
     */
    void cameraStop();

    Position postion();
    QPixmap scaledPixmap(const QPixmap &src, int width, int height);
    // 设置CameraWidget是否可以拖动
    void setCameraWidgetImmovable(bool immovable);

    /**
     * @brief getcameraStatus
     */
    int getCameraStatus();

    void setDevcieName(const QString &devcieName);
protected:
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);

    /**
     * @brief 重启摄像头设备
     */
    void restartDevices();

    /**
     * @brief 开始采集摄像头画面
     * @param device
     * @return
     */
    int startCameraV4l2(const char *device);

public slots:
    void onReceiveMajorImage(QImage image);

private:
    int recordX = 0;
    int recordY = 0;
    int recordWidth = 0;
    int recordHeight = 0;
    bool m_move = false;
    QPoint m_startPoint;
    QPoint m_windowTopLeftPoint;
    QPoint m_windowTopRightPoint;
    QPoint m_windowBottomLeftPoint;
    DLabel *m_cameraUI = nullptr;
    QString m_deviceName;

    bool m_Immovable = false; // 窗口不固定
    bool m_isInitCamera = false;
    MajorImageProcessingThread *m_imgPrcThread = nullptr;
};

#endif // CAMERAWIDGET_H
