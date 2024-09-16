// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"
#include "../dockiteminfo.h"

namespace dock {

class ClipboardItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool clipboardVisible READ clipboardVisible NOTIFY clipboardVisibleChanged)
public:
    explicit ClipboardItem(QObject *parent = nullptr);

    Q_INVOKABLE void toggleClipboard();

    Q_INVOKABLE DockItemInfo dockItemInfo();

    inline bool visible() const { return m_visible;}
    Q_INVOKABLE void setVisible(bool visible);

    Q_INVOKABLE bool clipboardVisible() const { return m_clipboardVisible; }

Q_SIGNALS:
    void visibleChanged(bool);
    void clipboardVisibleChanged(bool);

private slots:
    void onClipboardVisibleChanged(bool);

private:
    bool m_visible;
    bool m_clipboardVisible;
};

}
