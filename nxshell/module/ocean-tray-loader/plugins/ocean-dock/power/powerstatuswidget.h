// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POWERSTATUSWIDGET_H
#define POWERSTATUSWIDGET_H

#include "commoniconbutton.h"
#include "powerapplet.h"

#include <QWidget>

#define POWER_KEY "power"

class DBusPower;

// from https://upower.freedesktop.org/docs/Device.html#Device:State
enum BatteryState {
    UNKNOWN = 0,        // 未知
    CHARGING = 1,       // 充电中
    DIS_CHARGING = 2,   // 放电
    NOT_CHARGED = 3,    // 未充
    FULLY_CHARGED = 4   // 充满
};

class PowerStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PowerStatusWidget(QWidget *parent = 0);

    void refreshIcon();
    void refreshBatteryPercentage(const QString &percentage, const QString &tips);
    QWidget* itemWidget() const { return m_iconWidget; }
    PowerApplet *popupApplet() const { return m_applet; }

signals:
    void requestContextMenu(const QString &itemKey) const;
    void requestHideApplet();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DBusPower *m_powerInter;
    CommonIconButton *m_iconWidget;
    PowerApplet *m_applet;
};

#endif // POWERSTATUSWIDGET_H
