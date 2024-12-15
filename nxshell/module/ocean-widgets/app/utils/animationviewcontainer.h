// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <QWidget>
#include <DRegionMonitor>
#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>

class QPropertyAnimation;
DGUI_USE_NAMESPACE
WIDGETS_FRAME_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE
class AnimationViewContainer : public DBlurEffectWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal currentX READ currentX WRITE setCurrentX)
public:
    explicit AnimationViewContainer (QWidget *parent = nullptr);
    virtual ~AnimationViewContainer() override;

    void showView();
    void hideView();
    void refreshView();
    void updateGeometry(const QRect &rect);

protected:
    void paintEvent(QPaintEvent *e) override;

Q_SIGNALS:
    void outsideAreaReleased();

private Q_SLOTS:
    void regionMonitorHide(const QPoint & p, const int flag);

private:
    qreal currentX() const;
    void setCurrentX(const qreal x);
    void registerRegion();
    void unRegisterRegion();

private:
    QRectF m_targetRect;
    QPropertyAnimation *m_currentXAni = nullptr;
    DRegionMonitor *m_regionMonitor = nullptr;

    DGuiApplicationHelper::ColorType m_themeType;
    int m_cornerRadius;
    DPlatformWindowHandle *m_windowHandle = nullptr;
};
WIDGETS_FRAME_END_NAMESPACE
