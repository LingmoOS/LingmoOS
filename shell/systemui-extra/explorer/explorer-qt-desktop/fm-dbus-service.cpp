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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "fm-dbus-service.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QDebug>

using namespace Peony;

FMDBusService::FMDBusService(QObject *parent) : QObject(parent)
{
    qDebug()<<QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/FileManager1"), this,
            QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);
    qDebug()<<QDBusConnection::sessionBus().interface()->registerService(QStringLiteral("org.freedesktop.FileManager1"),
            QDBusConnectionInterface::QueueService);
}

void FMDBusService::ShowFolders(const QStringList& uriList, const QString& startUpId)
{
    qDebug()<<"show folders:"<<uriList;
    Q_EMIT showFolderRequest(uriList, startUpId);
}

void FMDBusService::ShowItems(const QStringList& uriList, const QString& startUpId)
{
    qDebug()<<"show items:"<<uriList;
    Q_EMIT showItemsRequest(uriList, startUpId);
}

void FMDBusService::ShowItemProperties(const QStringList& uriList, const QString& startUpId)
{
    qDebug()<<"show item properties:"<<uriList;
    Q_EMIT showItemPropertiesRequest(uriList, startUpId);
}
