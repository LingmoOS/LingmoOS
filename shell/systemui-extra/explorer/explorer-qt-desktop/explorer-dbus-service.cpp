/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */

#include "desktop-icon-view.h"
#include "explorer-dbus-service.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QDebug>
using namespace Peony;

PeonyDbusService::PeonyDbusService(DesktopIconView *view, QObject *parent) : QObject(parent)
{
    m_desktopIconView = view;
}

PeonyDbusService::~PeonyDbusService()
{
    m_desktopIconView = nullptr;
}

void PeonyDbusService::DbusServerRegister()
{
    QDBusConnection::sessionBus().unregisterService("org.lingmo.explorer");
    QDBusConnection::sessionBus().registerService("org.lingmo.explorer");
    QDBusConnection::sessionBus().registerObject("/org/lingmo/explorer", this, QDBusConnection :: ExportAllSlots | QDBusConnection::ExportAllSignals);
}

QString PeonyDbusService::GetSecurityConfigPath()
{
    QString jsonPath=QDir::homePath()+"/.config/explorer-security-config.json";

    return jsonPath;
}

int PeonyDbusService::ReloadSecurityConfig()
{
    m_desktopIconView->updateBWList();
    Q_EMIT black_and_white_update();

    return 0;
}

void PeonyDbusService::receiveSrcAndDestUrisOfCopy(const QStringList& sourceUris, const QStringList& destUris)
{
    Q_EMIT sendSrcAndDestUrisOfCopyDspsFiles(sourceUris, destUris);
}

QString PeonyDbusService::getBlackAndWhiteModel()
{
    if (!m_desktopIconView)
        return "";

    return m_desktopIconView->getBlackAndWhiteModel();
}

bool PeonyDbusService::getBlackAndWhiteListExist(QString name)
{
    if (!m_desktopIconView)
        return false;

    return m_desktopIconView->getBlackAndWhiteListExist(name);
}

QStringList PeonyDbusService::getBWListInfo()
{
    QSet<QString> info;
    if (!m_desktopIconView)
        return info.toList();

    info = m_desktopIconView->getBWListInfo();
    qDebug() << "getBWListInfo in service:"<<info;
    return info.toList();
}

void PeonyDbusService::sendEngrampaOpreateFinishSig(const QString &path, bool finish)
{
    Q_EMIT opreateFinishedOfEngrampa(path, finish);
}
