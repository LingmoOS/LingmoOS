// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "showdesktop.h"
#include "applet.h"
#include "pluginfactory.h"
#include "treelandwindowmanager.h"

#include <QProcess>
#include <QGuiApplication>
#include <QDBusInterface>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(showDesktop, "ocean.shell.dock.showdesktop")

namespace dock {

ShowDesktop::ShowDesktop(QObject *parent)
    : DApplet(parent)
    , m_windowManager(nullptr)
{

}

bool ShowDesktop::load()
{
    return true;
}

bool ShowDesktop::init()
{
    if (QStringLiteral("wayland") == QGuiApplication::platformName()) {
        m_windowManager = new TreelandWindowManager(this);
    }
    DApplet::init();
    return true;
}

void ShowDesktop::toggleShowDesktop()
{
    if (m_windowManager) {
        m_windowManager->desktopToggle();
        return;
    }

    QProcess::startDetached("/usr/lib/lingmo-daemon/desktop-toggle", QStringList());
}

bool ShowDesktop::checkNeedShowDesktop()
{
    QDBusInterface wmInter("com.lingmo.wm", "/com/lingmo/wm", "com.lingmo.wm");
    QList<QVariant> argumentList;
    QDBusMessage reply = wmInter.callWithArgumentList(QDBus::Block, QStringLiteral("GetIsShowDesktop"), argumentList);
    if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 1) {
        return !reply.arguments().at(0).toBool();
    }

    qCWarning(showDesktop) << "wm call GetIsShowDesktop fail, res:" << reply.type();
    return false;
}

D_APPLET_CLASS(ShowDesktop)
}


#include "showdesktop.moc"
