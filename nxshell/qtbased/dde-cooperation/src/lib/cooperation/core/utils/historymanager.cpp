// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_defines.h"
#include "historymanager.h"
#include "configs/settings/configmanager.h"

using namespace cooperation_core;

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
{
    connect(ConfigManager::instance(), &ConfigManager::appAttributeChanged, this, &HistoryManager::onAttributeChanged);
}

HistoryManager *HistoryManager::instance()
{
    static HistoryManager ins;
    return &ins;
}

void HistoryManager::onAttributeChanged(const QString &group, const QString &key, const QVariant &value)
{
    Q_UNUSED(value)

    if (group != AppSettings::CacheGroup)
        return;

    if (key == AppSettings::TransHistoryKey) {
        Q_EMIT transHistoryUpdated();
        return;
    }

    if (key == AppSettings::ConnectHistoryKey)
        Q_EMIT connectHistoryUpdated();
}

QMap<QString, QString> HistoryManager::getTransHistory()
{
    QMap<QString, QString> dataMap;

    if (qApp->property("onlyTransfer").toBool())
        return dataMap;
    const auto &list = ConfigManager::instance()->appAttribute(AppSettings::CacheGroup, AppSettings::TransHistoryKey).toList();
    for (const auto &item : list) {
        const auto &map = item.toMap();
        const auto &ip = map.value("ip").toString();
        const auto &path = map.value("savePath").toString();
        if (ip.isEmpty() || path.isEmpty())
            continue;

        dataMap.insert(ip, path);
    }

    return dataMap;
}

void HistoryManager::refreshHistory(bool found)
{
    if (!found) return;
    auto connectHistory = getConnectHistory();

    if (!connectHistory.isEmpty())
        emit historyConnected(connectHistory);
}

void HistoryManager::writeIntoTransHistory(const QString &ip, const QString &savePath)
{
    auto history = getTransHistory();
    if (history.contains(ip) && history.value(ip) == savePath)
        return;

    history.insert(ip, savePath);
    QVariantList list;
    auto iter = history.begin();
    while (iter != history.end()) {
        QVariantMap map;
        map.insert("ip", iter.key());
        map.insert("savePath", iter.value());

        list << map;
        ++iter;
    }

    ConfigManager::instance()->setAppAttribute(AppSettings::CacheGroup, AppSettings::TransHistoryKey, list);
}

void HistoryManager::removeTransHistory(const QString &ip)
{
    auto history = getTransHistory();
    if (history.remove(ip) == 0)
        return;

    QVariantList list;
    auto iter = history.begin();
    while (iter != history.end()) {
        QVariantMap map;
        map.insert("ip", iter.key());
        map.insert("savePath", iter.value());

        list << map;
        ++iter;
    }

    ConfigManager::instance()->setAppAttribute(AppSettings::CacheGroup, AppSettings::TransHistoryKey, list);
}

QMap<QString, QString> HistoryManager::getConnectHistory()
{
    QMap<QString, QString> dataMap;

    if (qApp->property("onlyTransfer").toBool())
        return dataMap;
    const auto &list = ConfigManager::instance()->appAttribute(AppSettings::CacheGroup, AppSettings::ConnectHistoryKey).toList();
    for (const auto &item : list) {
        const auto &map = item.toMap();
        const auto &ip = map.value("ip").toString();
        const auto &devName = map.value("devName").toString();
        if (ip.isEmpty() || devName.isEmpty())
            continue;

        dataMap.insert(ip, devName);
    }

    return dataMap;
}

void HistoryManager::writeIntoConnectHistory(const QString &ip, const QString &devName)
{
    auto history = getConnectHistory();
    if (history.contains(ip) && history.value(ip) == devName)
        return;

    history.insert(ip, devName);
    QVariantList list;
    auto iter = history.begin();
    while (iter != history.end()) {
        QVariantMap map;
        map.insert("ip", iter.key());
        map.insert("devName", iter.value());

        list << map;
        ++iter;
    }

    ConfigManager::instance()->setAppAttribute(AppSettings::CacheGroup, AppSettings::ConnectHistoryKey, list);
}
