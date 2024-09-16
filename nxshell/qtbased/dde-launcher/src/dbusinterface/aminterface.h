// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMINTERFACE_H
#define AMINTERFACE_H

#include "iteminfo.h"

#include <QObject>

#include <DDBusInterface>
#include <QMap>

DCORE_USE_NAMESPACE

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using QStringMap = QMap<QString, QString>;
using PropMap = QMap<QString, QStringMap>;

Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(QStringMap)
Q_DECLARE_METATYPE(PropMap)

class ItemInfo_v3
{
    // 应用类型
    enum Categorytype {
        CategoryInternet,
        CategoryChat,
        CategoryMusic,
        CategoryVideo,
        CategoryGraphics,
        CategoryGame,
        CategoryOffice,
        CategoryReading,
        CategoryDevelopment,
        CategorySystem,
        CategoryOthers,
        CategoryErr,
    };
public:
    ItemInfo_v3();

    inline bool isInvalid() const { return m_id.isEmpty(); }

    PropMap m_actionName;
    QStringList m_actions;
    QStringList m_categories;
    bool m_autoStart = false;
    QStringMap m_genericName;
    QStringMap m_name;
    QString m_id;
    QStringMap m_icons;
    QList<QDBusObjectPath> m_instances;
    qulonglong m_lastLaunchedTime;
    bool m_X_Flatpak = false;
    bool m_X_linglong = false;
    QString m_X_Deepin_Vendor;
    qulonglong m_installedTime;
    bool m_noDisplay = false;

    Categorytype category() const;
    friend QDebug operator<<(QDebug argument, const ItemInfo_v3 &info);
    bool operator==(const ItemInfo_v3 &other) const;
};

typedef QList<ItemInfo_v3> ItemInfoList_v3;

class ApplicationDBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationDBusProxy(const QString &path, QObject *parent = nullptr);

public Q_SLOTS:
    void onPropertiesChanged(const QDBusMessage& msg);

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &objPath, const ObjectInterfaceMap &objInterMap);
    void autoStartChanged(const bool &isAutoStarted) const;

private:
    DDBusInterface *m_applicationDBus;
};

class AppManagerDBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit AppManagerDBusProxy(QObject *parent = nullptr);

    inline QDBusPendingReply<ObjectMap> GetManagedObjects()
    {
        QList<QVariant> argumentList;
        return m_objectManagerDBus->asyncCallWithArgumentList(QStringLiteral("GetManagedObjects"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void InterfacesAdded(const QDBusObjectPath &objectPath, ObjectInterfaceMap interfacesProperties);
    void InterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);

private:
    DDBusInterface *m_objectManagerDBus;
};

class AMInter: public QObject
{
    Q_OBJECT
public:
    static AMInter *instance();

    static bool isAMReborn();
    ItemInfoList_v2 allInfos();
    QStringList allNewInstalledApps() const;
    void requestRemoveFromDesktop(const QString &appId);
    void requestSendToDesktop(const QString &appId);
    bool isOnDesktop(const QString &appId);
    void launch(const QString &desktop);
    bool isLingLong(const QString &appId) const;
    void uninstallApp(const QString &appId, const bool isLinglong = false);
    void setDisableScaling(const QString &appId, const bool value);
    bool disableScaling(const QString &appId);
    QStringList autostartList() const;
    bool addAutostart(const QString &appId);
    bool removeAutostart(const QString &appId);
    bool fullScreen() const;
    void setFullScreen(const bool isFullScreen);
    int displayMode() const;
    bool isNoDisplayed(const ItemInfo_v3 &item) const;

Q_SIGNALS: // SIGNALS
    void newAppLaunched(const QString &appId);
    void autostartChanged(const QString &action, const QString &appId);
    void itemChanged(const QString &action, ItemInfo_v2 info_v2, qlonglong categoryId);
    void fullScreenChanged();
    void displayModeChanged();

public Q_SLOTS:
    void onInterfaceAdded(const QDBusObjectPath &objectPath, const ObjectInterfaceMap &interfaces);
    void onRequestRemoveFromDesktop(QDBusPendingCallWatcher* watcher);
    void onAutoStartChanged(const bool &isAutoStarted, const QString &appId);
    void getManagedObjectsFinished(QDBusPendingCallWatcher*);

private:
    AMInter(QObject *parent = Q_NULLPTR);
    Q_DISABLE_COPY(AMInter)

    void fetchAllInfos();
    void buildAppDBusConnection();
    bool llUninstall(const QString &appid);
    bool lastoreUninstall(const QString &appId);
    bool shouldDisableScaling(const QString &appId);
    QStringList disableScalingApps() const;
    void setDisableScalingApps(const QStringList &value);
    ItemInfo_v3 itemInfoV3(const ObjectInterfaceMap &values) const;
    ItemInfo_v3 itemInfoV3(const QString &appId) const;
    ItemInfo_v2 itemInfoV2(const ItemInfo_v3 &itemInfoV3);
    QString getDesktop(const QString &desktopId);
    QString getPkgName(const QString &desktop);

    AppManagerDBusProxy *m_objManagerDbusInter;
    QMap<QString, ItemInfo_v3> m_infos;
    QStringList m_autoStartApps;
};

#endif
