// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "applet.h"
#include "dsglobal.h"
#include "dockiteminfo.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QQmlContext>
#include <QQuickWindow>

#include "windowmanager.h"
#include "xcbgetinfo.h"
#include "qmlengine.h"

namespace dock {

class AppRuntimeItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool appruntimeVisible READ appruntimeVisible NOTIFY appruntimeVisibleChanged)
public:
    explicit AppRuntimeItem(QObject *parent = nullptr);
    Q_INVOKABLE void toggleruntimeitem();
    Q_INVOKABLE DockItemInfo dockItemInfo();
    inline bool visible() const { return m_visible;}
    Q_INVOKABLE void setVisible(bool visible);
    Q_INVOKABLE bool appruntimeVisible() const { return m_appRuntimeVisible; }

Q_SIGNALS:
    void visibleChanged(bool);
    void appruntimeVisibleChanged(bool);

private Q_SLOTS:
    void onappruntimeVisibleChanged(bool);

private:
    bool m_visible;
    bool m_appRuntimeVisible;
    QScopedPointer<XcbGetInfo> m_xcbGetInfo;
    QScopedPointer<WindowManager> m_windowManager;
};

}
