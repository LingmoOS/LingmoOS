// SPDX-FileCopyrightText: 2014 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUBBLEMANAGER_H
#define BUBBLEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QQueue>
#include <QDesktopWidget>
#include <QApplication>
#include <QGuiApplication>
#include <QTimer>

#include "org_deepin_dde_sessionmanager1.h"
#include "org_deepin_dde_soundeffect1.h"
#include "org_deepin_dde_gesture1.h"
#include "org_deepin_dde_display1.h"
#include "org_deepin_dde_daemon_dock1.h"
#include "org_deepin_dde_appearance1.h"

#include "bubble.h"
#include "constants.h"

using Appearance = org::deepin::dde::Appearance1;
using UserInter = org::deepin::dde::SessionManager1;
using SoundeffectInter = org::deepin::dde::SoundEffect1;
using GestureInter = org::deepin::dde::Gesture1;
using DisplayInter = org::deepin::dde::Display1;
using DockInter = org::deepin::dde::daemon::Dock1;

static const QString NotificationsDBusService = "org.freedesktop.Notifications";
static const QString NotificationsDBusPath = "/org/freedesktop/Notifications";
static const QString DDENotifyDBusServer = "org.deepin.dde.Notification1";
static const QString DDENotifyDBusPath = "/org/deepin/dde/Notification1";
static const QString Login1DBusService = "org.freedesktop.login1";
static const QString Login1DBusPath = "/org/freedesktop/login1";
static const QString DockDaemonDBusServie = "org.deepin.dde.daemon.Dock1";
static const QString DockDaemonDBusPath = "/org/deepin/dde/daemon/Dock1";
static const QString DisplayDaemonDBusServie = "org.deepin.dde.Display1";
static const QString DisplayDaemonDBusPath = "/org/deepin/dde/Display1";
static const QString LauncherDaemonDBusServie = "org.deepin.dde.daemon.Launcher1";
static const QString LauncherDaemonDBusPath = "/org/deepin/dde/daemon/Launcher1";
static const QString SoundEffectDaemonDBusServie = "org.deepin.dde.SoundEffect1";
static const QString SoundEffectDaemonDBusPath = "/org/deepin/dde/SoundEffect1";
static const QString SessionDBusServie = "org.deepin.dde.SessionManager1";
static const QString SessionDaemonDBusPath = "/org/deepin/dde/SessionManager1";

class DBusControlCenter;
class Login1ManagerInterface;
class Persistence;
class AbstractPersistence;
class NotifyCenterWidget;
class NotifySettings;
class AbstractNotifySetting;

class DBusDockInterface;

class BubbleManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QString allSetting READ getAllSetting WRITE setAllSetting)
    Q_PROPERTY(QString systemSetting READ getSystemSetting WRITE setSystemSetting)
    Q_PROPERTY(uint recordCount READ recordCount NOTIFY recordCountChanged)

public:
    explicit BubbleManager(AbstractPersistence *persistence,
                           AbstractNotifySetting *setting, QObject *parent = nullptr);
    ~BubbleManager();

    enum ClosedReason {
        Expired = 1,
        Dismissed = 2,
        Closed = 3,
        Unknown = 4,
        Action = 5,
        Processed = 6,
        NotProcessedYet,
    };

    enum AnimationPath {
        Down = 0,
        Up = 1
    };

Q_SIGNALS:
    // Standard Notifications dbus implementation
    void ActionInvoked(uint, const QString &);
    void NotificationClosed(uint, uint);

    // Extra DBus APIs
    void RecordAdded(const QString &);
    void recordCountChanged(uint count);
    void AppInfoChanged(const QString &id, uint item, QDBusVariant var);
    void SystemInfoChanged(uint item, QDBusVariant var);
    void AppAddedSignal(const QString &id);
    void AppRemovedSignal(const QString &id);

    // Using external bubbles
    void ShowBubble(const QString &, uint replacesId, const QString &, const QString &, const QString &, const QStringList &, const QVariantMap, int, const QVariantMap bubbleParams);

    // 旧接口之后废弃
    void appAdded(QString appName);
    void appRemoved(QString appName);
    void appSettingChanged(QString Settings);
    void systemSettingChanged(QString Settings);

public Q_SLOTS:
    // Standard Notifications dbus implementation
    /*!
     * \~chinese \name CloseNotification
     * \~chinese \brief 根据通知id关闭通知气泡
     * \~chinese \param 根据Notify函数处理后返回的ID来关闭气泡
     */
    void CloseNotification(uint);
    QStringList GetCapabilities();
    /*!
     * \~chinese \name GetServerInformation
     * \~chinese \brief 获取服务信息
     */
    QString GetServerInformation(QString &, QString &, QString &);
    // new notify will be received by this slot

    /*!
     * \~chinese \name Notify
     * \~chinese \brief 返回一个根据通知内容生成的通知ID
     * \~chinese \param in0:App名称; in1:ID; in2:App图标名称; in3:通知信息概要; in4:通知信息主体
     * \~chinese \param in5:行为信息; in6:提示信息 in7:多长时间超时过期,值为-1时不会超时
     * \~chinese \return 返回一个处理后的replacesId
     */
    uint Notify(const QString &, uint replacesId, const QString &, const QString &, const QString &, const QStringList &, const QVariantMap, int);

    // Extra DBus APIs

    /*!
     * \~chinese \name GetAllRecords
     * \~chinese \brief 返回所有通知的记录
     * \~chinese \return 返回一个json格式的字符串
     */
    QString GetAllRecords();
    /*!
     * \~chinese \name GetRecordById
     * \~chinese \brief 根据ID查询通知记录
     * \~chinese \param id
     * \~chinese \return 返回一个json格式的字符串
     */
    QString GetRecordById(const QString &id);
    QString GetRecordsFromId(int rowCount, const QString &offsetId);
    /*!
     * \~chinese \name RemoveRecord
     * \~chinese \brief 根据ID删除通知记录
     * \~chinese \param id
     */
    void RemoveRecord(const QString &id);
    /*!
     * \~chinese \name ClearRecords
     * \~chinese \brief 删除所有通知记录
     */
    void ClearRecords();
    /*!
     * \~chinese \name Toggle
     * \~chinese \brief 控制通知中心的显示和隐藏
     */
    void Toggle();
    /*!
     */
    void ReplaceBubble(bool replace);
    /*!
     * \brief HandleBubbleEnd
     * 响应bubble结束
     */
    void HandleBubbleEnd(uint type, uint id, const QVariantMap bubbleParams, const QVariantMap selectedHints);

    void Show();
    void Hide();
    uint recordCount();
    QStringList GetAppList();
    QDBusVariant GetAppInfo(const QString &id, uint item);
    QDBusVariant GetSystemInfo(uint item);
    void SetAppInfo(const QString &id, uint item, const QDBusVariant var);
    void SetSystemInfo(uint item, const QDBusVariant var);

    // 旧接口之后废弃
    QString getAllSetting();
    void setAllSetting(const QString &settings);
    QString getAppSetting(QString appName);
    void setAppSetting(const QString &settings);
    QString getSystemSetting();
    void setSystemSetting(const QString &settings);

private Q_SLOTS:
    /*!
     * \~chinese \name geometryChanged
     * \~chinese \brief 当屏幕或DOCK栏发生几何变化时,该函数被执行,更新通知中心的几何位置和大小
     */
    void geometryChanged();
    void onPrepareForSleep(bool);
    /*!
     * \~chinese \name bubbleExpired
     * \~chinese \brief 当气泡超时时发出信号,执行该函数,产生移除动画,动画结束删除通知气泡
     * \~chinese \param 超时的通知气泡
     */
    void bubbleExpired(Bubble *);
    /*!
     * \~chinese \name bubbleDismissed
     * \~chinese \brief 鼠标点击气泡,或点击关闭按钮,执行该函数,产生移除动画,动画结束删除通知气泡
     * \~chinese \param 需要关闭的通知
     */
    void bubbleDismissed(Bubble *);
    void bubbleReplacedByOther(Bubble *);
    /*!
     * \~chinese \name bubbleActionInvoked
     * \~chinese \brief 鼠标点击气泡,点击该通知后执行的动作
     * \~chinese \param in1:当前气泡 in:气泡执行动作的id
     */
    void bubbleActionInvoked(Bubble *, QString);
    /*!
     * \~chinese \name updateGeometry
     * \~chinese \brief 当主屏幕发生改变或几何大小发送改变,更新所有通知气泡的几何位置
     */
    void updateGeometry();
    void onOpacityChanged(double value);

private:
    void initConnections();                 //初始化信号槽连接
    void registerAsService();               //注册Dbus服务
    /*!
     * \~chinese \name calcReplaceId
     * \~chinese \brief 计算ReplaceId,并查找当前通知中是否有相同的ReplaceId,如果有相同的
     * \~chinese ReplaceId,将该ReplaceId标记的通知中的内容替换掉
     * \~chinese \return 有相同的ReplaceId返回true,没有返回false
     */
    bool calcReplaceId(EntityPtr notify);

    bool checkControlCenterExistence();

    Bubble *createBubble(EntityPtr notify, int index = 0);  //创建一个通知气泡
    void pushBubble(EntityPtr notify);                      //推入一个气泡
    void popBubble(Bubble *);                               //推出一个气泡
    void refreshBubble();

    void pushAnimation(Bubble *bubble);                     //推入一个气泡的动画
    void popAnimation(Bubble *bubble);                      //推出一个气泡的动画

    QRect getBubbleGeometry(int index);                     //根据索引获取气泡的矩形大小
    // Get the last unanimated bubble rect
    QRect getLastStableRect(int index);                     //得到最后一个没有动画的矩形气泡
    bool isDoNotDisturb();
    QRect calcDisplayRect();
    /**
     * @brief getBubbleHeightBefore 获取序号小于index的气泡的高度之和
     * @param index 当前的气泡序号
     * @return 气泡高度之和
     */
    int getBubbleHeightBefore(const int index);

    /**
     * @brief popAllBubbles 立即移除当前所有的气泡
     */
    void popAllBubblesImmediately();

    bool useBuiltinBubble() const;
    void emitRecordAdded(EntityPtr notify);
    void emitRecordAdded(const QString &id);
private:
    int m_replaceCount = 0;
    QString m_configFile;
    QRect m_currentDisplayRect;
    QRect m_currentDockRect;
    OSD::DockPosition m_dockPos;
    int m_dockMode;

    QList<EntityPtr> m_oldEntities;
    QList<QPointer<Bubble>> m_bubbleList;

    AbstractPersistence *m_persistence;
    Login1ManagerInterface *m_login1ManagerInterface;
    DisplayInter *m_displayInter;
    DockInter *m_dockDeamonInter;
    UserInter *m_userInter;
    SoundeffectInter *m_soundeffectInter;
    AbstractNotifySetting *m_notifySettings;
    NotifyCenterWidget *m_notifyCenter;
    Appearance *m_appearance;

    // 手指划入距离，任务栏在右侧时，需大于任务栏最大宽度100，其它情况没有设限大于0即可
    int m_slideWidth;
    GestureInter *m_gestureInter;
    DBusDockInterface *m_dockInter;
    QTimer* m_trickTimer; // 防止300ms内重复按键
    bool m_useBuiltinBubble = true;
};

#endif // BUBBLEMANAGER_H
