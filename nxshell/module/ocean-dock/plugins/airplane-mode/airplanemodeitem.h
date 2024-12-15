// Copyright (C) 2020 ~ 2022 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANEMODEITEM_H
#define AIRPLANEMODEITEM_H

#include "org_lingmo_ocean_airplanemode1.h"

#include <QWidget>

using DBusAirplaneMode = org::lingmo::ocean::AirplaneMode1;

namespace Dock {
class TipsWidget;
}

class AirplaneModeApplet;
class AirplaneModeItem : public QWidget
{
    Q_OBJECT

public:
    explicit AirplaneModeItem(QWidget *parent = nullptr);

    QWidget *tipsWidget();
    QWidget *popupApplet();
    const QString contextMenu() const;
    void invokeMenuItem(const QString menuId, const bool checked);
    void refreshIcon();
    void updateTips();

    bool airplaneEnable();

protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);

signals:
    void airplaneEnableChanged(bool enable);

private:
    Dock::TipsWidget *m_tipsLabel;
    AirplaneModeApplet *m_applet;
    DBusAirplaneMode *m_airplaneModeInter;
    QPixmap m_iconPixmap;
};

#endif // AIRPLANEMODEITEM_H
