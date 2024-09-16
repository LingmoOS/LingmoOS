// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinfomanager.h"

#include <QMutex>
#include <QLoggingCategory>

QMutex mutex;
std::atomic<DeviceInfoManager *> DeviceInfoManager::s_Instance;
std::mutex DeviceInfoManager::m_mutex;

DeviceInfoManager::DeviceInfoManager(QObject *parent)
    : QObject(parent)
{

}

void DeviceInfoManager::addInfo(const QString &key, const QString &value)
{
    QMutexLocker locker(&mutex);
    if (m_MapInfo.find(key) != m_MapInfo.end()) {
        m_MapInfo[key] = value;
    } else {
        m_MapInfo.insert(key, value);
    }
}

const QString &DeviceInfoManager::getInfo(const QString &key)
{
    QMutexLocker locker(&mutex);
    return m_MapInfo[key];
}

bool DeviceInfoManager::isInfoExisted(const QString &key)
{
    QMutexLocker locker(&mutex);
    if (m_MapInfo.find(key) != m_MapInfo.end()) {
        return true;
    } else {
        return false;
    }
}

bool DeviceInfoManager::isPathExisted(const QString &path)
{
    QMutexLocker locker(&mutex);
    const QString &hwinfo = m_MapInfo["hwinfo"];
    QString pathT = path;
    if (hwinfo.contains(pathT.replace("/sys", ""))) {
        return true;
    }
    return false;
}

