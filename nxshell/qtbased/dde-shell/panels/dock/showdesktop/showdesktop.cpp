// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applet.h"
#include "showdesktop.h"
#include "pluginfactory.h"

#include <QProcess>
#include <QGuiApplication>
#include <QDBusInterface>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(showDesktop, "dde.shell.dock.showdesktop")

namespace dock {

ShowDesktop::ShowDesktop(QObject *parent)
    : DApplet(parent)
{

}

bool ShowDesktop::load()
{
    return QStringLiteral("xcb") == QGuiApplication::platformName();
}

bool ShowDesktop::init()
{
    DApplet::init();
    return true;
}

void ShowDesktop::toggleShowDesktop()
{
    QProcess::startDetached("/usr/lib/deepin-daemon/desktop-toggle", QStringList());
}

bool ShowDesktop::checkNeedShowDesktop()
{
    QDBusInterface wmInter("com.deepin.wm", "/com/deepin/wm", "com.deepin.wm");
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
