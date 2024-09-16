// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amapplet.h"

#include "pluginfactory.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include <DConfig>
#include <DUtil>

DCORE_USE_NAMESPACE
DS_BEGIN_NAMESPACE
namespace am {

AMApplet::AMApplet(QObject *parent)
    : DApplet(parent)
{
}

AMApplet::~AMApplet()
{
}

bool AMApplet::load()
{
    // the signal maybe loss when AM is crashed.
    auto conn = QDBusConnection::sessionBus();
    bool ret = conn.connect("org.desktopspec.ApplicationManager1",
                 "/org/desktopspec/ApplicationManager1",
                 "org.desktopspec.DBus.ObjectManager",
                 "InterfacesRemoved",
                 this,
                 SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList))
                 );
    if (!ret) {
        qWarning() << "Failed to connect InterfacesReoved signal for AM" << conn.lastError().message();
        return false;
    }
    return DApplet::load();
}


static QString unescapeAppIdFromObjectPath(const QString &path)
{
    if (path.isEmpty()) return {};
    const auto startIndex = QString("/org/desktopspec/ApplicationManager1").size();
    auto endIndex = path.indexOf("/", startIndex + 1);
    const auto id = endIndex <= -1 ? path.mid(startIndex + 1) :
                        path.sliced(startIndex + 1, endIndex - (startIndex + 1));
    return DUtil::unescapeFromObjectPath(id);
}

void AMApplet::onInterfacesRemoved(const QDBusObjectPath &objPath, const QStringList &interfaces)
{
    const QString &path(objPath.path());
    qDebug() << "InterfacesRemoved by AM, path:" << path;

    const QString &appId = unescapeAppIdFromObjectPath(path);
    updateAppsLaunchedTimes(appId);
}

void AMApplet::updateAppsLaunchedTimes(const QString &appId)
{
    std::unique_ptr<DConfig> config(DConfig::create("org.deepin.dde.application-manager", "org.deepin.dde.am"));
    if (!config->isValid()) {
        qWarning() << "DConfig is invalid when updating launched times.";
        return;
    }

    const QString AppsLaunchedTimes(u8"appsLaunchedTimes");
    auto value = config->value(AppsLaunchedTimes).toMap();
    if (auto iter = value.find(appId); iter != value.end()) {
        value.remove(appId);
        config->setValue(AppsLaunchedTimes, value);
        qDebug() << "Reset launched times for the app:" << appId;
    }
}

D_APPLET_CLASS(AMApplet)
}
DS_END_NAMESPACE

#include "amapplet.moc"
