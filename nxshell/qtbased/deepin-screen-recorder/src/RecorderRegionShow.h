// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDERREGION_H
#define RECORDERREGION_H

#include "widgets/camerawidget.h"
#include "widgets/keybuttonwidget.h"

#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>

/**
 * @brief The RecorderRegionShow class 2d模式的录屏
 */
class RecorderRegionShow : public DWidget
{
    Q_OBJECT
public:
    RecorderRegionShow();
    virtual ~RecorderRegionShow();
    void initCameraInfo(const CameraWidget::Position position, const QSize size);
    void showKeyBoardButtons(const QString &key);
    void updateKeyBoardButtonStyle();
    void setCameraShow(const bool isVisible = true); //设置Camera窗口显示
    void setDevcieName(const QString &devcieName);
protected:
    void paintEvent(QPaintEvent *event);
    void updateMultiKeyBoardPos();


private:
    QPainter *m_painter;
    CameraWidget *m_cameraWidget;
    QList<KeyButtonWidget *> m_keyButtonList;
    QString m_deviceName;


};
#endif
