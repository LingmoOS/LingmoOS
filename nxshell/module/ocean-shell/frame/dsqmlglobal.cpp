// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/dsqmlglobal_p.h"

#include "applet.h"
#include "appletbridge.h"
#include "pluginloader.h"

#include <QLoggingCategory>
#include <QCoreApplication>
#include <QQueue>
#include <QWindow>
#include <QGuiApplication>

#ifdef BUILD_WITH_X11
#include "private/utility_x11_p.h"
#endif

#include <dobject_p.h>

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

class DQmlGlobalPrivate : public DObjectPrivate
{
public:
    explicit DQmlGlobalPrivate(DQmlGlobal *qq)
        : DObjectPrivate(qq)
    {
    }

    D_DECLARE_PUBLIC(DQmlGlobal)
};

DQmlGlobal::DQmlGlobal(QObject *parent)
    : QObject(parent)
    , DObject(*new DQmlGlobalPrivate(this))
{
}

DQmlGlobal::~DQmlGlobal()
{
}

DAppletProxy *DQmlGlobal::applet(const QString &pluginId) const
{
    DAppletBridge bridge(pluginId);
    return bridge.applet();
}

QList<DAppletProxy *> DQmlGlobal::appletList(const QString &pluginId) const
{
    DAppletBridge bridge(pluginId);
    return bridge.applets();
}

QList<QWindow *> DQmlGlobal::allChildrenWindows(QWindow *target)
{
    return Utility::instance()->allChildrenWindows(target);
}

void DQmlGlobal::closeChildrenWindows(QWindow *target)
{
    for (const auto item : Utility::instance()->allChildrenWindows(target))
        if (item && item->isVisible())
            item->close();
}

bool DQmlGlobal::grabKeyboard(QWindow *target, bool grab)
{
    return Utility::instance()->grabKeyboard(target, grab);
}

bool DQmlGlobal::grabMouse(QWindow *target, bool grab)
{
    return Utility::instance()->grabMouse(target, grab);
}

DApplet *DQmlGlobal::rootApplet() const
{
    return DPluginLoader::instance()->rootApplet();
}

DQmlGlobal *DQmlGlobal::instance()
{
    static DQmlGlobal *gInstance = nullptr;
    if (!gInstance) {
        gInstance = new DQmlGlobal();
        // ensure DQmlGlobal is live in main thread.
        gInstance->moveToThread(qApp->thread());
    }
    return gInstance;
}

DS_END_NAMESPACE
