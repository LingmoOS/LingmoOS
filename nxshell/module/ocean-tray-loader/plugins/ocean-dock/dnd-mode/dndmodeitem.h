// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DND_MODE_ITEM_H
#define DND_MODE_ITEM_H

#include "commoniconbutton.h"
#include "commonapplet.h"

#include <QWidget>

namespace Dock {
class TipsWidget;
}

class DndModeItem : public QWidget
{
    Q_OBJECT

public:
    explicit DndModeItem(QWidget *parent = nullptr);

    QWidget *tipsWidget();
    QWidget *popupApplet();
    const QString contextMenu() const;
    void invokeMenuItem(const QString menuId, const bool checked);
    void refreshIcon();
    void updateTips();

    bool airplaneEnable();

signals:
    void requestHideApplet();

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void init();

private:
    Dock::TipsWidget *m_tipsLabel;
    CommonApplet *m_applet;
    CommonIconButton *m_icon;
    QPixmap m_iconPixmap;
};

#endif // DND_MODE_ITEM_H
