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
#include "application-info.h"
#include "app-info-table.h"

using namespace LingmoUISearch;
namespace LingmoUISearch {
class ApplicationInfoPrivate : public QObject
{
public:
    explicit ApplicationInfoPrivate(ApplicationInfo *parent = nullptr) : QObject(parent), m_infoTable(new AppInfoTable(this)) {}
    AppInfoTable *m_infoTable = nullptr;
};
}

ApplicationInfo::ApplicationInfo(QObject *parent) : QObject(parent), d(new ApplicationInfoPrivate(this)) {
    connect(d->m_infoTable, &AppInfoTable::appDBItems2BUpdate, this, &ApplicationInfo::appDBItems2BUpdate);
    connect(d->m_infoTable, &AppInfoTable::appDBItems2BUpdateAll, this, &ApplicationInfo::appDBItems2BUpdateAll);
    connect(d->m_infoTable, &AppInfoTable::appDBItems2BAdd, this, &ApplicationInfo::appDBItems2BAdd);
    connect(d->m_infoTable, &AppInfoTable::appDBItems2BDelete, this, &ApplicationInfo::appDBItems2BDelete);
}

QVariant ApplicationInfo::getInfo(const QString &desktopFile, ApplicationProperty::Property property) {
    ApplicationPropertyMap map;
    d->m_infoTable->query(map, desktopFile, ApplicationProperties{property});
    return map.value(property);
}

ApplicationPropertyMap ApplicationInfo::getInfo(const QString& desktopFile, ApplicationProperties properties) {
    ApplicationPropertyMap propertyMap;
    d->m_infoTable->query(propertyMap, desktopFile, properties);
    return propertyMap;
}

ApplicationInfoMap ApplicationInfo::getInfo(ApplicationProperties properties) {
    ApplicationInfoMap infoMap;
    d->m_infoTable->query(infoMap, properties);
    return infoMap;
}

ApplicationInfoMap ApplicationInfo::getInfo(ApplicationProperties properties, ApplicationPropertyMap restrictions) {
    ApplicationInfoMap infoMap;
    d->m_infoTable->query(infoMap, properties, restrictions);
    return infoMap;
}

ApplicationInfoMap ApplicationInfo::searchApp(ApplicationProperties properties, const QString &keyword, ApplicationPropertyMap restrictions) {
    ApplicationInfoMap infoMap;
    d->m_infoTable->query(infoMap, properties, QStringList{keyword}, restrictions);
    return infoMap;
}

ApplicationInfoMap ApplicationInfo::searchApp(ApplicationProperties properties, const QStringList &keywords, ApplicationPropertyMap restrictions) {
    ApplicationInfoMap infoMap;
    d->m_infoTable->query(infoMap, properties, keywords, restrictions);
    return infoMap;
}

void ApplicationInfo::setAppToFavorites(const QString &desktopFilePath) {
    return d->m_infoTable->setAppFavoritesState(desktopFilePath);
}

void ApplicationInfo::setFavoritesOfApp(const QString &desktopFilePath, size_t num) {
    return d->m_infoTable->setAppFavoritesState(desktopFilePath, num);
}

void ApplicationInfo::setAppToTop(const QString &desktopFilePath) {
    return d->m_infoTable->setAppTopState(desktopFilePath);
}

void ApplicationInfo::setTopOfApp(const QString &desktopFilePath, size_t num) {
    return d->m_infoTable->setAppTopState(desktopFilePath, num);
}

void ApplicationInfo::setAppLaunchedState(const QString &desktopFilePath, bool launched) {
    return d->m_infoTable->setAppLaunchedState(desktopFilePath, launched);
}

bool ApplicationInfo::tranPidToDesktopFp(int pid, QString &desktopFilePath) {
    return d->m_infoTable->tranPidToDesktopFp(pid, desktopFilePath);
}

bool ApplicationInfo::tranPidToDesktopFp(uint pid, QString &desktopFilePath) {
    return d->m_infoTable->tranPidToDesktopFp(pid, desktopFilePath);
}

bool ApplicationInfo::desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath) {
    return d->m_infoTable->desktopFilePathFromName(desktopFileName, desktopFilePath);
}

bool ApplicationInfo::tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopFilePath) {
    return d->m_infoTable->tranWinIdToDesktopFilePath(winId, desktopFilePath);
}

void ApplicationInfo::removeAppFromFavorites(const QString &desktopFilePath) {
    return d->m_infoTable->setAppFavoritesState(desktopFilePath, 0);
}

ApplicationInfo::~ApplicationInfo() = default;
