/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */


#include "app-database-interface.h"
#include "user-config.h"
#include "settings.h"

#include <application-info.h>
#include <QDebug>

#define APP_ICON_PREFIX "image://theme/"

namespace LingmoUIMenu {

class AppDatabaseWorkerPrivate : public QObject
{
    Q_OBJECT
    friend class AppDatabaseInterface;

public:
    explicit AppDatabaseWorkerPrivate(AppDatabaseInterface *parent = nullptr);
    DataEntityVector getAllApps();
    bool getApp(const QString &appid, DataEntity &app);

    // 数据库操作函数
    void setAppProperty(const QString &appid, const LingmoUISearch::ApplicationPropertyMap &propertyMap);
    void setAppProperty(const QString &appid, const LingmoUISearch::ApplicationProperty::Property &property, const QVariant &value);

private Q_SLOTS:
    /**
     * 应用数据库的添加信号处理函数
     * @param infos 新增应用的id列表
     */
    void onAppDatabaseAdded(const QStringList &infos);
    void onAppDatabaseUpdate(const LingmoUISearch::ApplicationInfoMap &infoMap);
    void onAppDatabaseUpdateAll(const QStringList &infos);
    void onAppDatabaseDeleted(const QStringList &infos);

private:
    static void addInfoToApp(const QMap<LingmoUISearch::ApplicationProperty::Property, QVariant> &info, DataEntity &app);
    bool isFilterAccepted(const LingmoUISearch::ApplicationPropertyMap &appInfo) const;

private:
    AppDatabaseInterface *q {nullptr};

    LingmoUISearch::ApplicationInfo       *appDatabase {nullptr};
    LingmoUISearch::ApplicationProperties  properties;
    // 设置我们需要的属性和值
    LingmoUISearch::ApplicationPropertyMap filter;
};

AppDatabaseWorkerPrivate::AppDatabaseWorkerPrivate(AppDatabaseInterface *parent) : QObject(parent), q(parent)
{
    // 注册需要在信号和槽函数中使用的数据结构
    qRegisterMetaType<LingmoUIMenu::DataEntityVector>("DataEntityVector");

    // 初始化应用数据库链接
    appDatabase = new LingmoUISearch::ApplicationInfo(this);

    // 首次启动时，为某些应用设置标志位
    if (UserConfig::instance()->isFirstStartUp()) {
        // 默认收藏应用
        for (const auto &appid : GlobalSetting::instance()->defaultFavoriteApps()) {
            appDatabase->setAppToFavorites(appid);
            appDatabase->setAppLaunchedState(appid, true);
        }
    }

    // 设置从数据库查询哪些属性
    properties << LingmoUISearch::ApplicationProperty::Property::Top
                  << LingmoUISearch::ApplicationProperty::Property::Lock
                  << LingmoUISearch::ApplicationProperty::Property::Favorites
                  << LingmoUISearch::ApplicationProperty::Property::LaunchTimes
                  << LingmoUISearch::ApplicationProperty::Property::DesktopFilePath
                  << LingmoUISearch::ApplicationProperty::Property::Icon
                  << LingmoUISearch::ApplicationProperty::Property::LocalName
                  << LingmoUISearch::ApplicationProperty::Property::Category
                  << LingmoUISearch::ApplicationProperty::Property::FirstLetterAll
                  << LingmoUISearch::ApplicationProperty::Property::DontDisplay
                  << LingmoUISearch::ApplicationProperty::Property::AutoStart
                  << LingmoUISearch::ApplicationProperty::Property::InsertTime
                  << LingmoUISearch::ApplicationProperty::Property::Launched;
    
    // 需要从数据库过滤的属性，满足该条件的数据才会被查询出来
    filter.insert(LingmoUISearch::ApplicationProperty::Property::DontDisplay, 0);
    filter.insert(LingmoUISearch::ApplicationProperty::Property::AutoStart, 0);

    // 链接数据库信号
    connect(appDatabase, &LingmoUISearch::ApplicationInfo::appDBItems2BAdd, this, &AppDatabaseWorkerPrivate::onAppDatabaseAdded);
    connect(appDatabase, &LingmoUISearch::ApplicationInfo::appDBItems2BUpdate, this, &AppDatabaseWorkerPrivate::onAppDatabaseUpdate);
    connect(appDatabase, &LingmoUISearch::ApplicationInfo::appDBItems2BUpdateAll, this, &AppDatabaseWorkerPrivate::onAppDatabaseUpdateAll);
    connect(appDatabase, &LingmoUISearch::ApplicationInfo::appDBItems2BDelete, this, &AppDatabaseWorkerPrivate::onAppDatabaseDeleted);
    connect(appDatabase, &LingmoUISearch::ApplicationInfo::DBOpenFailed, q, &AppDatabaseInterface::appDatabaseOpenFailed);
}

void AppDatabaseWorkerPrivate::addInfoToApp(const LingmoUISearch::ApplicationPropertyMap &info, DataEntity &app)
{
    app.setTop(info.value(LingmoUISearch::ApplicationProperty::Property::Top).toInt());
    app.setLock(info.value(LingmoUISearch::ApplicationProperty::Property::Lock).toInt() == 1);
    app.setFavorite(info.value(LingmoUISearch::ApplicationProperty::Property::Favorites).toInt());
    app.setLaunchTimes(info.value(LingmoUISearch::ApplicationProperty::Property::LaunchTimes).toInt());
    app.setId(info.value(LingmoUISearch::ApplicationProperty::Property::DesktopFilePath).toString());
    app.setIcon(info.value(LingmoUISearch::ApplicationProperty::Property::Icon).toString());
    app.setName(info.value(LingmoUISearch::ApplicationProperty::Property::LocalName).toString());
    app.setCategory(info.value(LingmoUISearch::ApplicationProperty::Property::Category).toString());
    app.setFirstLetter(info.value(LingmoUISearch::ApplicationProperty::Property::FirstLetterAll).toString());
    app.setInsertTime(info.value(LingmoUISearch::ApplicationProperty::Property::InsertTime).toString());
    app.setLaunched(info.value(LingmoUISearch::ApplicationProperty::Property::Launched).toInt());
}

bool AppDatabaseWorkerPrivate::isFilterAccepted(const LingmoUISearch::ApplicationPropertyMap &appInfo) const
{
    QMapIterator<LingmoUISearch::ApplicationProperty::Property, QVariant> iterator(filter);
    while (iterator.hasNext()) {
        iterator.next();
        if (appInfo.value(iterator.key()) != iterator.value()) {
            return false;
        }

        // TODO: 根据数据的类型进行比较
//        bool equals = false;
//        QVariant value = appInfo.value(iterator.key());
//        value.userType();
    }

    return true;
}

DataEntityVector AppDatabaseWorkerPrivate::getAllApps()
{
    LingmoUISearch::ApplicationInfoMap appInfos = appDatabase->getInfo(properties, filter);
    if (appInfos.isEmpty()) {
        return {};
    }

    DataEntityVector apps;
    for (const auto &info : appInfos) {
        DataEntity app;
        AppDatabaseWorkerPrivate::addInfoToApp(info, app);
        apps.append(app);
    }

    return apps;
}

void AppDatabaseWorkerPrivate::onAppDatabaseAdded(const QStringList &infos)
{
    if (infos.isEmpty()) {
        return;
    }

    DataEntityVector apps;
    for (const QString &appid : infos) {
        const LingmoUISearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);
        if (!isFilterAccepted(appInfo)) {
            continue;
        }

        DataEntity app;
        addInfoToApp(appInfo, app);
        apps.append(app);
    }

    if (apps.isEmpty()) {
        return;
    }

    Q_EMIT q->appAdded(apps);
}

void AppDatabaseWorkerPrivate::onAppDatabaseUpdate(const LingmoUISearch::ApplicationInfoMap &infoMap)
{
    if (infoMap.isEmpty()) {
        return;
    }

    QVector<QPair<DataEntity, QVector<int> > > updates;
    QMapIterator<QString, LingmoUISearch::ApplicationPropertyMap> iterator(infoMap);
    while (iterator.hasNext()) {
        iterator.next();

        DataEntity app;
        QVector<int> roles;

        QMapIterator<LingmoUISearch::ApplicationProperty::Property, QVariant> it(iterator.value());
        while (it.hasNext()) {
            it.next();

            switch (it.key()) {
                case LingmoUISearch::ApplicationProperty::LocalName:
                    app.setName(it.value().toString());
                    roles.append(DataEntity::Name);
                    break;
                case LingmoUISearch::ApplicationProperty::FirstLetterAll:
                    app.setFirstLetter(it.value().toString());
                    roles.append(DataEntity::FirstLetter);
                    break;
                case LingmoUISearch::ApplicationProperty::Icon:
                    app.setIcon(it.value().toString());
                    roles.append(DataEntity::Icon);
                    break;
                case LingmoUISearch::ApplicationProperty::InsertTime:
                    app.setInsertTime(it.value().toString());
                    roles.append(DataEntity::Icon);
                    break;
                case LingmoUISearch::ApplicationProperty::Category:
                    app.setCategory(it.value().toString());
                    roles.append(DataEntity::Category);
                    break;
                case LingmoUISearch::ApplicationProperty::LaunchTimes:
                    app.setLaunchTimes(it.value().toInt());
                    roles.append(DataEntity::LaunchTimes);
                    break;
                case LingmoUISearch::ApplicationProperty::Favorites:
                    app.setFavorite(it.value().toInt());
                    roles.append(DataEntity::Favorite);
                    break;
                case LingmoUISearch::ApplicationProperty::Launched:
                    app.setLaunched(it.value().toInt());
                    roles.append(DataEntity::IsLaunched);
                    break;
                case LingmoUISearch::ApplicationProperty::Top:
                    app.setTop(it.value().toInt());
                    roles.append(DataEntity::Top);
                    break;
                case LingmoUISearch::ApplicationProperty::Lock:
                    app.setLock(it.value().toBool());
                    roles.append(DataEntity::IsLocked);
                    break;
                default:
                    break;
            }
        }

        // 这个函数中，没有更新列不会发送信号
        if (roles.isEmpty()) {
            continue;
        }

        app.setId(iterator.key());
        updates.append({app, roles});
    }

    if (!updates.isEmpty()) {
        Q_EMIT q->appUpdated(updates);
    }
}

void AppDatabaseWorkerPrivate::onAppDatabaseDeleted(const QStringList &infos)
{
    if (infos.empty()) {
        return;
    }

    Q_EMIT q->appDeleted(infos);
}

void AppDatabaseWorkerPrivate::onAppDatabaseUpdateAll(const QStringList &infos)
{
    if (infos.isEmpty()) {
        return;
    }

    QVector<QPair<DataEntity, QVector<int> > > updates;
    DataEntityVector apps;

    for (const auto &appid : infos) {
        const LingmoUISearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);
        DataEntity app;
        addInfoToApp(appInfo, app);
        apps.append(app);

        updates.append({app, {}});
    }

    Q_EMIT q->appUpdated(updates);
}

void AppDatabaseWorkerPrivate::setAppProperty(const QString &appid, const LingmoUISearch::ApplicationPropertyMap &propertyMap)
{
    QMapIterator<LingmoUISearch::ApplicationProperty::Property, QVariant> iterator(propertyMap);
    while (iterator.hasNext()) {
        iterator.next();
        setAppProperty(appid, iterator.key(), iterator.value());
    }
}

void AppDatabaseWorkerPrivate::setAppProperty(const QString &appid, const LingmoUISearch::ApplicationProperty::Property &property,
                                              const QVariant &value)
{
    switch (property) {
        case LingmoUISearch::ApplicationProperty::Favorites:
            appDatabase->setFavoritesOfApp(appid, value.toInt());
            break;
        case LingmoUISearch::ApplicationProperty::Launched:
            appDatabase->setAppLaunchedState(appid, value.toBool());
            break;
        case LingmoUISearch::ApplicationProperty::Top:
            appDatabase->setTopOfApp(appid, value.toInt());
            break;
        default:
            break;
    }
}

bool AppDatabaseWorkerPrivate::getApp(const QString &appid, DataEntity &app)
{
    if (appid.isEmpty()) {
        return false;
    }

    const LingmoUISearch::ApplicationPropertyMap appInfo = appDatabase->getInfo(appid, properties);
    addInfoToApp(appInfo, app);
    return true;
}

// ====== AppDatabaseInterface ====== //
// TODO: 多线程
AppDatabaseInterface::AppDatabaseInterface(QObject *parent) : QObject(parent), d(new AppDatabaseWorkerPrivate(this))
{

}

DataEntityVector AppDatabaseInterface::apps() const
{
    return d->getAllApps();
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    if (index < 0) {
        index = 0;
    }

    d->setAppProperty(appid, LingmoUISearch::ApplicationProperty::Top, index);
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    if (index < 0) {
        index = 0;
    }

    d->setAppProperty(appid, LingmoUISearch::ApplicationProperty::Favorites, index);
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    d->setAppProperty(appid, LingmoUISearch::ApplicationProperty::Launched, state);
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    return d->getApp(appid, app);
}

} // LingmoUIMenu

#include "app-database-interface.moc"
