// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANEMODEITEM_H
#define AIRPLANEMODEITEM_H

#include "commoniconbutton.h"
#include "commonapplet.h"

#include <QWidget>

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
    QWidget *iconWidget() const { return m_icon; }
    const QString contextMenu() const;
    void invokeMenuItem(const QString menuId, const bool checked);
    void refreshIcon();
    void updateTips();

    bool airplaneEnable();

protected:
    void resizeEvent(QResizeEvent *e);

signals:
    void airplaneEnableChanged(bool enable);
    void requestHideApplet();

private:
    Dock::TipsWidget *m_tipsLabel;
    CommonApplet *m_applet;
    CommonIconButton *m_icon;
};

#endif // AIRPLANEMODEITEM_H
