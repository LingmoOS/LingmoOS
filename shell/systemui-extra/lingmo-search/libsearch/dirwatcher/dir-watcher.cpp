/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#include "dir-watcher.h"

#include <QDebug>
#include <QThread>
#include <QDir>

static std::once_flag flag;
static DirWatcher *global_intance = nullptr;

DirWatcher::DirWatcher(QObject *parent)
    : QObject(parent)
{

    m_dbusInterface = new QDBusInterface("com.lingmo.search.fileindex.service",
                                         "/org/lingmo/search/privateDirWatcher",
                                         "org.lingmo.search.fileindex");
    if (!m_dbusInterface->isValid()) {
        qCritical() << "Create privateDirWatcher Interface Failed Because: " << QDBusConnection::sessionBus().lastError();
        return;
    } else {
        connect(m_dbusInterface, SIGNAL(appendIndexItem(QString, QStringList)), this, SLOT(sendAppendSignal(QString, QStringList)));
        connect(m_dbusInterface, SIGNAL(removeIndexItem(QString)), this, SLOT(sendRemoveSignal(QString)));
    }
}

DirWatcher::~DirWatcher()
{
    if (m_dbusInterface)
        delete m_dbusInterface;
    m_dbusInterface = nullptr;
}

DirWatcher *DirWatcher::getDirWatcher()
{
    std::call_once(flag, [ & ] {
        global_intance = new DirWatcher();
    });
    return global_intance;
}

void DirWatcher::initDbusService()
{
    QDBusReply<void> reply = m_dbusInterface->call("initDbusService");
    if (!reply.isValid()) {
        qCritical() << "initDbusService call filed!";
    }
}

QStringList DirWatcher::currentIndexableDir()
{

    QDBusReply<QStringList> reply = m_dbusInterface->call("currentIndexableDir");
    if (!reply.isValid()) {
        qCritical() << "currentIndexableDir call filed!";
        return QStringList() << QString(QDir::homePath());
    }
    return reply.value();
}

QStringList DirWatcher::currentBlackListOfIndex()
{
    QDBusReply<QStringList> reply = m_dbusInterface->call("currentBlackListOfIndex");
    if (!reply.isValid()) {
        qCritical() << "currentBlackListOfIndex call filed!";
        return QStringList();
    }
    return reply.value();
}

/**
 * @brief DirWatcher::blackListOfDir 根据传入目录返回当前目录下的所有不可搜索目录，没有则返回空列表
 * @param dirPath 要搜索的目录
 */
QStringList DirWatcher::blackListOfDir(const QString &dirPath)
{
    QDBusReply<QStringList> reply = m_dbusInterface->call("blackListOfDir", dirPath);
    if (!reply.isValid()) {
        qCritical() << "blackListOfDir call filed!";
        return QStringList();
    }
    return reply.value();
}

QStringList DirWatcher::getBlockDirsOfUser()
{
    QDBusReply<QStringList> reply = m_dbusInterface->call("blockDirsForUser");
    if (!reply.isValid()) {
        qCritical() << "blockDirsForUser call filed!";
        return QStringList();
    }
    return reply.value();
}

QStringList DirWatcher::currentSearchableDir()
{
    QDBusReply<QStringList> reply = m_dbusInterface->call("currentSearchableDir");
    if (!reply.isValid()) {
        qCritical() << "currentSearchableDir call filed!";
        return QStringList();
    }
    return reply.value();
}

QStringList DirWatcher::searchableDirForSearchApplication()
{
    QDBusReply<QStringList> reply = m_dbusInterface->call("searchableDirForSearchApplication");
    if (!reply.isValid()) {
        qCritical() << "searchableDirForSearchApplication call filed!";
        return QStringList();
    }
    return reply.value();
}

void DirWatcher::appendSearchDir(const QString &path)
{
    QDBusReply<int> reply = m_dbusInterface->call("appendSearchDir", path);
    if (!reply.isValid()) {
        qCritical() << "appendSearchDir call filed!";
    }
}

void DirWatcher::removeSearchDir(const QString &path)
{
    QDBusReply<void> reply = m_dbusInterface->call("removeSearchDir", path);
    if (!reply.isValid()) {
        qCritical() << "removeSearchDir call filed!";
    }
}

void DirWatcher::appendIndexableListItem(const QString &path)
{
    QDBusReply<int> reply = m_dbusInterface->call("appendIndexableListItem", path);
    if (!reply.isValid()) {
        qCritical() << "appendIndexableListItem call filed!";
    }
}

void DirWatcher::removeIndexableListItem(const QString &path)
{
    QDBusReply<void> reply = m_dbusInterface->call("removeIndexableListItem", path);
    if (!reply.isValid()) {
        qCritical() << "removeIndexableListItem call filed!";
    }
}

void DirWatcher::sendAppendSignal(const QString &path, const QStringList &blockList)
{
    Q_EMIT this->appendIndexItem(path, blockList);
}

void DirWatcher::sendRemoveSignal(const QString &path)
{
    Q_EMIT this->removeIndexItem(path);
}
