// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUWORKER_H
#define MENUWORKER_H

#include "dbustartmanager.h"
#include "appsmanager.h"
#include "appslistmodel.h"

#include <QVariant>
#include <QProcess>
#include <QX11Info>
#include <QDBusObjectPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDBusPendingReply>
#include <QtCore>
#include <QModelIndex>

class QMenu;
class Menu;
class AMDBusLauncherInter;
class AMDBusDockInter;

class MenuWorker : public QObject
{
    Q_OBJECT

public:
    explicit MenuWorker(QObject *parent = nullptr);
    ~MenuWorker();

    enum MenuAction
    {
        Open = 1,
        Desktop = 2,
        Dock = 3,
        Startup = 4,
        Proxy = 5,
        SwitchScale = 6,
        Uninstall = 7,
        MoveToTop = 8,
        EditCollected = 9        // 添加到收藏列表或从收藏列表中移除
    };

    bool isMenuShown() const {return m_menuIsShown;}
    bool isItemOnDock(QString appKey);
    bool isItemOnDesktop(QString appKey);
    bool isItemStartup(QString appKey);
    QRect menuGeometry() const {return m_menuGeometry;}
    void creatMenuByAppItem();
    bool isMenuVisible();

signals:
    void appLaunched();
    void menuAccepted();
    void unInstallApp(const QModelIndex &index);
    void menuShowMouseMoving();
    void requestMoveToTop(const QModelIndex &index);
    void requestEditCollected(const QModelIndex &index, const bool isInCollected);

public slots:
    void showMenuByAppItem(QPoint pos, const QModelIndex &index);

    void handleOpen();
    void handleToDesktop();
    void handleToDock();
    void handleToStartup();
    void handleToProxy();
    void handleSwitchScaling();

    void handleMenuClosed();
    void setCurrentModelIndex(const QModelIndex &index);
    const QModelIndex getCurrentModelIndex();
    void handleMenuAction(int index);
    void onHideMenu();

private:
    AMDBusLauncherInter *m_amDbusLauncher;
    AMDBusDockInter *m_amDbusDockInter;

    DBusStartManager* m_startManagerInterface;
    CalculateUtil *m_calcUtil;
    AppsManager *m_appManager;

    QModelIndex m_currentModelIndex = QModelIndex();
    QString m_appKey;
    QString m_appDesktop;
    QString m_currentMenuObjectPath;
    QMap<QString, QString> m_menuObjectPaths;
    QRect m_menuGeometry;
    bool m_isItemOnDesktop;
    bool m_isItemOnDock;
    bool m_isItemStartup;
    bool m_isItemProxy;
    bool m_isItemEnableScaling;
    bool m_isItemInCollected;

    bool m_menuIsShown = false;
    Menu *m_menu;
    QSignalMapper *m_signalMapper;
};

#endif // MENUWORKER_H
