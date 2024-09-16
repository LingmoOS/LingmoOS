// SPDX-FileCopyrightText: 2014 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubblemanager.h"
#include "bubble.h"
#include "dbuslogin1manager.h"
#include "notification/bubbletool.h"
#include "notificationentity.h"
#include "persistence.h"
#include "constants.h"
#include "notifysettings.h"
#include "notification-center/notifycenterwidget.h"
#include "dbusdockinterface.h"
#include "signalbridge.h"

#include <DDesktopServices>
#include <DSGApplication>

#include <QStringList>
#include <QVariantMap>
#include <QTimer>
#include <QDebug>
#include <QScreen>
#include <QDBusContext>
#include <QDateTime>
#include <QGSettings>
#include <QLoggingCategory>

#include "org_deepin_dde_display1.h"
#include "org_deepin_dde_display1_monitor.h"

Q_LOGGING_CATEGORY(notifiyBubbleLog, "dde.notifycation.bubblemanger")

using DisplayInter = org::deepin::dde::Display1;
using MonitorInter = org::deepin::dde::display1::Monitor;

BubbleManager::BubbleManager(AbstractPersistence *persistence, AbstractNotifySetting *setting, QObject *parent)
    : QObject(parent)
    , m_persistence(persistence)
    , m_login1ManagerInterface(new Login1ManagerInterface(Login1DBusService, Login1DBusPath,
                                                          QDBusConnection::systemBus(), this))
    , m_userInter(new UserInter(SessionDBusServie, SessionDaemonDBusPath,
                                 QDBusConnection::sessionBus(), this))
    , m_notifySettings(setting)
    , m_notifyCenter(new NotifyCenterWidget(m_persistence))
    , m_trickTimer(new QTimer(this))
{
    if (!useBuiltinBubble()) {
        qCDebug(notifiyBubbleLog) << "Default does not use built-in bubble.";
    }

    if (useBuiltinBubble()) {
        m_displayInter = new DisplayInter(DisplayDaemonDBusServie, DisplayDaemonDBusPath,
                                           QDBusConnection::sessionBus(), this);
        m_dockDeamonInter = new DockInter(DockDaemonDBusServie, DockDaemonDBusPath,
                                           QDBusConnection::sessionBus(), this);
        m_soundeffectInter = new SoundeffectInter(SoundEffectDaemonDBusServie, SoundEffectDaemonDBusPath,
                                                   QDBusConnection::sessionBus(), this);
        m_appearance = new Appearance("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", QDBusConnection::sessionBus(), this);
        m_dockInter = new DBusDockInterface(this);
        m_gestureInter = new GestureInter("org.deepin.dde.Gesture1"
                                          , "/org/deepin/dde/Gesture1"
                                          , QDBusConnection::systemBus()
                                          , this);
    }
    m_trickTimer->setInterval(300);
    m_trickTimer->setSingleShot(true);

    initConnections();
    geometryChanged();

    if (useBuiltinBubble()) {
        m_notifyCenter->setMaskAlpha(static_cast<quint8>(m_appearance->opacity() * 255));
    }
    m_notifyCenter->hide();
    registerAsService();

    if (useBuiltinBubble()) {
        // 任务栏在左侧时，触屏划入距离需要超过100
        m_slideWidth = (m_dockDeamonInter->position() == OSD::DockPosition::Right) ? 100 : 0;
        m_dockInter->setSync(false);
        connect(m_userInter, &UserInter::LockedChanged, this, [ this ] {
            // 当锁屏状态发生变化时立即隐藏所有通知并插入到通知中心（根据通知的实际情况决定），桌面和锁屏的通知不交叉显示
            popAllBubblesImmediately();
        });
    }
}

BubbleManager::~BubbleManager()
{
    if (!m_bubbleList.isEmpty()) qDeleteAll(m_bubbleList);

    m_oldEntities.clear();
    delete m_notifyCenter;
    m_notifyCenter = nullptr;
}

void BubbleManager::CloseNotification(uint id)
{
#if defined (QT_DEBUG)
    if (calledFromDBus()) {
        QDBusReply<uint> reply = connection().interface()->servicePid(message().service());
        qDebug() << "PID:" << reply.value();//关闭通知的进程
    }
#endif

    QString str_id = QString::number(id);
    foreach (auto bubble, m_bubbleList) {
        if (bubble->entity()->replacesId() == str_id) {
            //m_persistence->addOne(bubble->entity());
            bubble->close();
            m_bubbleList.removeOne(bubble);
            qDebug() << "CloseNotification : id" << str_id;
        }
    }

    foreach (auto notify, m_oldEntities) {
        if (notify->replacesId() == str_id) {
            //m_persistence->addOne(notify);
            m_oldEntities.removeOne(notify);
            qDebug() << "CloseNotification : id" << str_id;
        }
    }
}

QStringList BubbleManager::GetCapabilities()
{
    QStringList result;
    result << "action-icons" << "actions" << "body" << "body-hyperlinks" << "body-markup";

    return result;
}

QString BubbleManager::GetServerInformation(QString &name, QString &vender, QString &version)
{
    name = QString("DeepinNotifications");
    vender = QString("Deepin");
    version = QString("2.0");

    return QString("1.2");
}

uint BubbleManager::Notify(const QString &appName, uint replacesId,
                           const QString &appIcon, const QString &summary,
                           const QString &body, const QStringList &actions,
                           const QVariantMap hints, int expireTimeout)
{
DCORE_USE_NAMESPACE

    QString servicePid, appId;
    if (calledFromDBus()) {
        QDBusReply<uint> reply = connection().interface()->servicePid(message().service());
        servicePid = QString::number(reply.value());
        appId = DSGApplication::getId(reply.value());
    }
    if (appId.isEmpty())
        appId = appName;

    if (calledFromDBus()) {
        QGSettings oem_setting("com.deepin.dde.notifications", "/com/deepin/dde/notifications/");
        if (oem_setting.keys().contains("notifycationClosed") && oem_setting.get("notifycationClosed").toBool())
            return 0;

        QGSettings setting("com.deepin.dde.osd", "/com/deepin/dde/osd/");
        if (setting.keys().contains("bubbleDebugPrivacy") && setting.get("bubble-debug-privacy").toBool()) {
            qDebug() << "Notify:" << "appName:" + appName << "replaceID:" + QString::number(replacesId)
                     << "appIcon:" + appIcon << "summary:" + summary << "body:" + body
                     << "actions:" << actions << "hints:" << hints << "expireTimeout:" << expireTimeout;

            qDebug() << "servicePid" << servicePid << "appid" << appId;

            if (!servicePid.isEmpty()) {
                // 记录通知发送方
                QString cmd = QString("grep \"Name:\" /proc/%1/status").arg(servicePid);
                QProcess process;
                QStringList args;
                args << "-c";
                args << cmd;
                process.start("sh", args);
                process.waitForReadyRead();
                qDebug() << "cmd :" << cmd;
                QString result = QString::fromUtf8(process.readAllStandardOutput());
                qDebug() << "notify called by :" << result;
                process.close();
            }
        }
    }

    if (useBuiltinBubble()) {
        // 如果display服务无效，无法获取显示器大小，不能正确计算显示位置，则不显示消息通知
        if (!m_displayInter->isValid()) {
            qWarning() << "The name org.deepin.dde.Display1 is invalid";
            return 0;
        }
    }

    // 应用通知功能未开启不做处理
    bool enableNotificaion = m_notifySettings->getAppSetting(appId, NotifySettings::ENABELNOTIFICATION).toBool();

    if (!enableNotificaion && !IgnoreList.contains(appId)) {
        return 0;
    }

    QString strBody = body;
    strBody.replace(QLatin1String("\\\\"), QLatin1String("\\"), Qt::CaseInsensitive);
    QString icon = appIcon;
    if (icon.isEmpty())
        icon = m_notifySettings->getAppSetting(appId, NotifySettings::APPICON).toString();

    EntityPtr notification = std::make_shared<NotificationEntity>(appName, QString(), icon,
                                                                  summary, strBody, actions, hints,
                                                                  QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                                  QString::number(replacesId),
                                                                  QString::number(expireTimeout));

    bool enablePreview = true;
    bool showInNotifyCenter = true;
    bool playsound = true;
    bool lockscreeshow = true;
    bool dndmode = isDoNotDisturb();
    bool systemNotification = IgnoreList.contains(notification->appName());
    bool lockscree = m_userInter->locked();

    if (!systemNotification) {
        enablePreview = m_notifySettings->getAppSetting(appId, NotifySettings::ENABELPREVIEW).toBool();
        showInNotifyCenter = m_notifySettings->getAppSetting(appId, NotifySettings::SHOWINNOTIFICATIONCENTER).toBool();
        playsound = m_notifySettings->getAppSetting(appId, NotifySettings::ENABELSOUND).toBool();
        lockscreeshow = m_notifySettings->getAppSetting(appId, NotifySettings::LOCKSCREENSHOWNOTIFICATION).toBool();
    }

    notification->setShowPreview(enablePreview);
    notification->setShowInNotifyCenter(showInNotifyCenter);

    if (playsound && !dndmode) {
        QString action;
        //接收蓝牙文件时，只在发送完成后才有提示音,"cancel"表示正在发送文件
        if (actions.contains("cancel")) {
            if (hints.contains("x-deepin-action-_view")) {
                action = hints["x-deepin-action-_view"].toString();
                if (action.contains("xdg-open"))
                    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Notifications);
            }
        } else {
            DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Notifications);
        }
    }

    if (systemNotification && dndmode) {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Notifications);
    }

    if (!calcReplaceId(notification)) {
        QVariantMap params;
        params["id"] = notification->id();
        params["isShowPreview"] = enablePreview;
        params["isShowInNotifyCenter"] = showInNotifyCenter;
        if (systemNotification) { // 系统通知
            if (showInNotifyCenter) { // 开启在通知中心显示才加入通知中心的数据库
                m_persistence->addOne(notification);
                params["storageId"] = notification->storageId();
            }
            if (useBuiltinBubble()) {
                pushBubble(notification);
            } else {
                qCDebug(notifiyBubbleLog) << "Publish ShowBubble, id:" << notification->id();
                Q_EMIT ShowBubble(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout, params);
            }
        } else if (lockscree && !lockscreeshow) { // 锁屏不显示通知
            if (showInNotifyCenter) { // 开启在通知中心显示才加入通知中心
                m_persistence->addOne(notification);
                emitRecordAdded(notification);
            }
        } else { // 锁屏显示通知或者未锁屏状态
            if (!systemNotification && !dndmode && enableNotificaion) { // 普通应用非勿扰模式并且开启通知选项
                if (showInNotifyCenter) { // 开启在通知中心显示才加入通知中心的数据库
                    m_persistence->addOne(notification);
                    params["storageId"] = notification->storageId();
                }
                if (useBuiltinBubble()) {
                    pushBubble(notification);
                } else {
                    qCDebug(notifiyBubbleLog) << "Publish ShowBubble, id:" << notification->id();
                    Q_EMIT ShowBubble(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout, params);
                }
            } else if (showInNotifyCenter) {
                m_persistence->addOne(notification);
                emitRecordAdded(notification);
            }
        }
    } else {
        if (!useBuiltinBubble()) {
            QVariantMap params;
            params["id"] = notification->id();
            params["isShowPreview"] = enablePreview;
            params["isShowInNotifyCenter"] = showInNotifyCenter;
            if (showInNotifyCenter) { // 开启在通知中心显示才加入通知中心
                m_persistence->addOne(notification);
                params["storageId"] = notification->storageId();
            }
            qCDebug(notifiyBubbleLog) << "Publish ShowBubble, replaceId:" << notification->replacesId();
            Q_EMIT ShowBubble(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout, params);
        }
    }

    // If replaces_id is 0, the return value is a UINT32 that represent the notification.
    // If replaces_id is not 0, the returned value is the same value as replaces_id.
    return replacesId == 0 ? notification->id() : replacesId;
}

void BubbleManager::pushBubble(EntityPtr notify)
{
    if (notify == nullptr) return;

    Bubble *bubble = createBubble(notify);
    if (!bubble)
        return;

    if (m_bubbleList.size() == BubbleEntities + BubbleOverLap) {
        m_oldEntities.push_front(m_bubbleList.last()->entity());
        m_bubbleList.last()->setVisible(false);
        // emit RecordAdded for oldEntities entity
        emitRecordAdded(m_bubbleList.last()->entity());
        m_bubbleList.last()->deleteLater();
        m_bubbleList.removeLast();
    }

    m_bubbleList.push_front(bubble);
    pushAnimation(bubble);
}

void BubbleManager::popBubble(Bubble *bubble)
{
    // bubble delete itself when aniamtion finished
    refreshBubble();
    popAnimation(bubble);
    m_bubbleList.removeOne(bubble);
}

void BubbleManager::popAllBubblesImmediately()
{
    for (QPointer<Bubble> &bubble : m_bubbleList) {
        if (bubble) {
            if (bubble->entity()->isShowInNotifyCenter()) {
                m_persistence->addOne(bubble->entity());
                emitRecordAdded(bubble->entity());
            }

            bubble->hide();
            bubble->close();
        }
    }

    m_bubbleList.clear();
}

bool BubbleManager::useBuiltinBubble() const
{
    static bool isTreeLand = qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == QString("TreeLand");
    // some dbus service is unavailable in treeland.
    if (isTreeLand)
        return false;

    return m_useBuiltinBubble;
}

void BubbleManager::emitRecordAdded(EntityPtr notify)
{
    if (!notify->isShowInNotifyCenter())
        return;
    emitRecordAdded(notify->storageId());
}

void BubbleManager::emitRecordAdded(const QString &id)
{
    if (!id.isEmpty())
        Q_EMIT RecordAdded(id);
}

void BubbleManager::refreshBubble()
{
    if (m_bubbleList.size() < BubbleEntities + BubbleOverLap + 1 && !m_oldEntities.isEmpty()) {
        auto notify = m_oldEntities.takeFirst();
        Bubble *bubble = createBubble(notify, BubbleEntities + BubbleOverLap - 1);
        if (bubble)
            m_bubbleList.push_back(bubble);
    }
}

void BubbleManager::pushAnimation(Bubble *bubble)
{
    int index = m_bubbleList.indexOf(bubble);
    if (index == -1)
        return;

    while (index < m_bubbleList.size() - 1) {
        index ++;
        QRect startRect = getLastStableRect(index - 1);
        QRect endRect = getBubbleGeometry(index);
        QPointer<Bubble> item = m_bubbleList.at(index);
        if (item->geometry() != endRect) { //动画中
            startRect = item->geometry();
        }
        if (bubble != nullptr) {
            item->setBubbleIndex(index);
            item->startMove(startRect, endRect);
        }
    }
}

void BubbleManager::popAnimation(Bubble *bubble)
{
    int index = m_bubbleList.indexOf(bubble);
    if (index == -1)
        return;

    QRect startRect = getBubbleGeometry(index);
    QRect endRect = getBubbleGeometry(0);

    if (bubble)
        bubble->startMove(startRect, endRect, true); // delete itself

    while (index < m_bubbleList.size() - 1) {
        index ++;
        startRect = getBubbleGeometry(index);
        endRect = getBubbleGeometry(index - 1);
        QPointer<Bubble> item = m_bubbleList.at(index);
        if (index == BubbleEntities + BubbleOverLap) {
            item->show();
        }
        if (item->geometry() != endRect) { //动画中
            startRect = item->geometry();
        }
        if (bubble != nullptr) {
            item->setBubbleIndex(index);
            item->startMove(startRect, endRect);
        }
    }

    //确定层次关系
    for (int i = m_bubbleList.size() - 1; i >= 0 ; --i) {
        Bubble *b = m_bubbleList[i];
        if (b) {
            b->raise();
        }
    }
}

QRect BubbleManager::getBubbleGeometry(int index)
{
    Q_ASSERT(index >= 0 && index <= BubbleEntities + BubbleOverLap);

    QRect rect;
    if (index >= 0 && index <= BubbleEntities - 1) {
        int y = (m_dockPos == OSD::Top ? m_currentDockRect.bottom() : m_currentDisplayRect.y()); // 多屏时屏幕设置为上下错位，主屏的top可能不是0
        rect.setX(m_currentDisplayRect.x() + (m_currentDisplayRect.width() - OSD::BubbleWidth(OSD::BUBBLEWINDOW)) / 2);
        rect.setY(y + ScreenPadding + index * BubbleMargin + getBubbleHeightBefore(index));
        rect.setWidth(OSD::BubbleWidth(OSD::BUBBLEWINDOW));
        rect.setHeight(OSD::BubbleHeight(OSD::BUBBLEWINDOW));
    } else if (index >= BubbleEntities && index <= BubbleEntities + BubbleOverLap) {
        rect = getBubbleGeometry(index - 1);

        int x = rect.x() + rect.width() / 20;
        int y = rect.y() + rect.height() / 3;
        int width = rect.width() * 18 / 20;
        int height = rect.height() * 19 / 20;

        rect.setX(x);
        rect.setY(y);
        rect.setWidth(width);
        rect.setHeight(height);
    }

    return rect;
}

int BubbleManager::getBubbleHeightBefore(const int index)
{
    int totalHeight = 0;
    for (int i = 0; i < index; i++) {
        if (m_bubbleList[i]) {
            totalHeight += m_bubbleList[i]->height();
        }
    }

    return totalHeight;
}

QRect BubbleManager::getLastStableRect(int index)
{
    QRect rect = getBubbleGeometry(0);
    for (int i = index - 1; i > 0; --i) {
        if (i >= m_bubbleList.size() || m_bubbleList.at(i)->geometry() != getBubbleGeometry(i)) {
            continue;
        }
        rect = getBubbleGeometry(i);
    }

    return rect;
}

bool BubbleManager::isDoNotDisturb()
{
    if (!m_notifySettings->getSystemSetting(NotifySettings::DNDMODE).toBool())
        return false;

    // 未点击按钮  任何时候都勿扰模式
    if (!m_notifySettings->getSystemSetting(NotifySettings::OPENBYTIMEINTERVAL).toBool() && !m_notifySettings->getSystemSetting(NotifySettings::LOCKSCREENOPENDNDMODE).toBool()) {
        return true;
    }

    bool lockScreen = m_userInter->locked();
    // 点击锁屏时 并且 锁屏状态 任何时候都勿扰模式
    if (m_notifySettings->getSystemSetting(NotifySettings::LOCKSCREENOPENDNDMODE).toBool() && lockScreen)
        return true;

    QTime currentTime = QTime::fromString(QDateTime::currentDateTime().toString("hh:mm"));
    QTime startTime = QTime::fromString(m_notifySettings->getSystemSetting(NotifySettings::STARTTIME).toString());
    QTime endTime = QTime::fromString(m_notifySettings->getSystemSetting(NotifySettings::ENDTIME).toString());

    bool dndMode = false;
    if (startTime < endTime) {
        dndMode = startTime <= currentTime && endTime >= currentTime ? true : false;
    } else if (startTime > endTime) {
        dndMode = startTime <= currentTime || endTime >= currentTime ? true : false;
    } else {
        dndMode = true;
    }

    if (dndMode && m_notifySettings->getSystemSetting(NotifySettings::OPENBYTIMEINTERVAL).toBool()) {
        return dndMode;
    } else {
        return false;
    }
}

QRect BubbleManager::calcDisplayRect()
{
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QRect displayRect = m_displayInter->primaryRect();
    QList<QDBusObjectPath> screenList = m_displayInter->monitors();

    QRect dockRect(m_dockInter->geometry());
    for (const auto &screen : screenList) {
        MonitorInter monitor("org.deepin.dde.Display1", screen.path(), QDBusConnection::sessionBus());
        QRect monitorRect(monitor.x(), monitor.y(), monitor.width(), monitor.height());
        if (monitor.enabled() && monitorRect.contains(dockRect.center())) {
            displayRect = QRect(monitorRect.x(), monitorRect.y(),
                                monitorRect.width() / ratio, monitorRect.height() / ratio);
            break;
        }
    }
    return displayRect;
}

QString BubbleManager::GetAllRecords()
{
    return m_persistence->getAll();
}

QString BubbleManager::GetRecordById(const QString &id)
{
    return m_persistence->getById(id);
}

QString BubbleManager::GetRecordsFromId(int rowCount, const QString &offsetId)
{
    return m_persistence->getFrom(rowCount, offsetId);
}

void BubbleManager::RemoveRecord(const QString &id)
{
    m_persistence->removeOne(id);

    QFile file(CachePath + id + ".png");
    file.remove();
}

void BubbleManager::ClearRecords()
{
    m_persistence->removeAll();

    QDir dir(CachePath);
    dir.removeRecursively();
}

void BubbleManager::Toggle()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    geometryChanged();
    m_notifyCenter->showWidget();
}

void BubbleManager::ReplaceBubble(bool replace)
{
    if (m_useBuiltinBubble == !replace)
        return;

    m_useBuiltinBubble = !replace;
    if (!m_useBuiltinBubble) {
        popAllBubblesImmediately();
    }
}

void BubbleManager::HandleBubbleEnd(uint type, uint id, const QVariantMap bubbleParams, const QVariantMap selectedHints)
{
    qCDebug(notifiyBubbleLog) << "HandleBubbleEnd, type:" << type << ", bubbleId:" << id
                              << bubbleParams << selectedHints;
    switch (type) {
    case BubbleManager::Expired:
    case BubbleManager::Unknown:
    case BubbleManager::Dismissed: {
        Q_EMIT NotificationClosed(id, type);
    } break;
    case BubbleManager::NotProcessedYet: {
        const auto extraParams = qdbus_cast<QVariantMap>(bubbleParams["extraParams"]);
        const auto isShowInNotifyCenter = extraParams["isShowInNotifyCenter"].toBool();
        const auto storageId = extraParams["storageId"].toString();
        if (!isShowInNotifyCenter) {
            return;
        }
        emitRecordAdded(storageId);
    } break;
    case BubbleManager::Action: {
        const auto hints = qdbus_cast<QVariantMap>(selectedHints);
        const auto actionId = hints["actionId"].toString();
        const auto extraParams = qdbus_cast<QVariantMap>(bubbleParams["extraParams"]);
        const auto storageId = extraParams["storageId"].toString();
        EntityPtr entity = m_persistence->getNotifyById(storageId);
        if (!entity) {
            qWarning() << QString("it can't find dbhd:%1 in store ").arg(storageId);
            return; 
        }
        const auto replaceId = entity->replacesId().toUInt();
        if (actionId == "default") {
            BubbleTool::actionInvoke(actionId, entity);
        }
        Q_EMIT ActionInvoked(replaceId == 0 ? id : replaceId, actionId);
        Q_EMIT NotificationClosed(id, BubbleManager::Closed);
    } break;
    case BubbleManager::Processed: {
        const auto extraParams = qdbus_cast<QVariantMap>(bubbleParams["extraParams"]);
        const auto storageId = extraParams["storageId"].toString();
        if (storageId.isEmpty()) {
            return;
        }
        m_persistence->removeOne(storageId);
    }
    }
}

void BubbleManager::Show()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    geometryChanged();

    m_notifyCenter->onlyShowWidget();
}

void BubbleManager::Hide()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    geometryChanged();

    m_notifyCenter->onlyHideWidget();
}

uint BubbleManager::recordCount()
{
    return m_persistence->recordCount();
}

QStringList BubbleManager::GetAppList()
{
    return m_notifySettings->getAppLists();
}

QDBusVariant BubbleManager::GetAppInfo(const QString &id, const uint item)
{
    const auto &tmp = m_notifySettings->getAppSetting(id, static_cast<NotifySettings::AppConfigurationItem>(item));
    if (!tmp.isValid()) {
        sendErrorReply(QDBusError::NotSupported, QString("GetAppInfo() failed for the app: [%1] configuration item: [%2].").arg(id).arg(item));
        return QDBusVariant();
    }

    return QDBusVariant(tmp);
}

QDBusVariant BubbleManager::GetSystemInfo(uint item)
{
    const auto &tmp = m_notifySettings->getSystemSetting(static_cast<NotifySettings::SystemConfigurationItem>(item));
    if (!tmp.isValid()) {
        sendErrorReply(QDBusError::NotSupported, QString("GetSystemInfo() failed for the configuration item: [%1].").arg(item));
        return QDBusVariant();
    }

    return QDBusVariant(tmp);
}

void BubbleManager::SetAppInfo(const QString &id, const uint item, const QDBusVariant var)
{
    m_notifySettings->setAppSetting(id, static_cast<NotifySettings::AppConfigurationItem>(item), var.variant());
}

void BubbleManager::SetSystemInfo(uint item, const QDBusVariant var)
{
    m_notifySettings->setSystemSetting(static_cast<NotifySettings::SystemConfigurationItem>(item), var.variant());
    Q_EMIT systemSettingChanged(m_notifySettings->getSystemSetings_v1());
}


void BubbleManager::onOpacityChanged(double value)
{
    m_notifyCenter->setMaskAlpha(static_cast<quint8>(value * 255));
}

QString BubbleManager::getAllSetting()
{
    return m_notifySettings->getAllSetings_v1();
}

void BubbleManager::setAllSetting(const QString &settings)
{
    m_notifySettings->setAllSetting_v1(settings);
}

QString BubbleManager::getAppSetting(QString appName)
{
    return m_notifySettings->getAppSettings_v1(appName);
}

void BubbleManager::setAppSetting(const QString &settings)
{
    QJsonObject currentObj = QJsonDocument::fromJson(settings.toUtf8()).object();
    m_notifySettings->setAppSetting_v1(settings);
    Q_EMIT systemSettingChanged(currentObj.begin().key());
}

QString BubbleManager::getSystemSetting()
{
    return m_notifySettings->getSystemSetings_v1();
}

void BubbleManager::setSystemSetting(const QString &settings)
{
    m_notifySettings->setSystemSetting_v1(settings);
    Q_EMIT systemSettingChanged(m_notifySettings->getSystemSetings_v1());
}

void BubbleManager::registerAsService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(NotificationsDBusService,
                                            QDBusConnectionInterface::ReplaceExistingService,
                                            QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);

    QDBusConnection ddenotifyConnect = QDBusConnection::sessionBus();
    ddenotifyConnect.interface()->registerService(DDENotifyDBusServer,
                                                  QDBusConnectionInterface::ReplaceExistingService,
                                                  QDBusConnectionInterface::AllowReplacement);
    ddenotifyConnect.registerObject(DDENotifyDBusPath, this);
}


void BubbleManager::bubbleExpired(Bubble *bubble)
{
    popBubble(bubble);
    Q_EMIT NotificationClosed(bubble->entity()->id(), BubbleManager::Expired);
}

void BubbleManager::bubbleDismissed(Bubble *bubble)
{
    popBubble(bubble);
    Q_EMIT NotificationClosed(bubble->entity()->id(), BubbleManager::Dismissed);
}

void BubbleManager::bubbleReplacedByOther(Bubble *bubble)
{
    Q_EMIT NotificationClosed(bubble->entity()->id(), BubbleManager::Unknown);
}

void BubbleManager::bubbleActionInvoked(Bubble *bubble, QString actionId)
{
    popBubble(bubble);
    uint id = bubble->entity()->id();
    uint replacesId = bubble->entity()->replacesId().toUInt();
    Q_EMIT ActionInvoked(replacesId == 0 ? id : replacesId, actionId);
    Q_EMIT NotificationClosed(bubble->entity()->id(), BubbleManager::Closed);
}

void BubbleManager::updateGeometry()
{
    for (int index = 0; index < m_bubbleList.count(); index++) {
        auto item = m_bubbleList[index];
        if (!item.isNull()) {
            item->setGeometry(getBubbleGeometry(index));
            item->updateGeometry();
        }
    }
}

void BubbleManager::initConnections()
{
    connect(m_login1ManagerInterface, SIGNAL(PrepareForSleep(bool)),
            this, SLOT(onPrepareForSleep(bool)));

    if (useBuiltinBubble()) {
        connect(m_displayInter, &DisplayInter::PrimaryRectChanged, this, &BubbleManager::geometryChanged, Qt::QueuedConnection);
        connect(m_dockDeamonInter, &DockInter::FrontendWindowRectChanged, this, &BubbleManager::geometryChanged, Qt::UniqueConnection);
        connect(m_dockDeamonInter, &DockInter::serviceValidChanged, this, &BubbleManager::geometryChanged, Qt::UniqueConnection);

        connect(qApp, &QApplication::primaryScreenChanged, this, [ = ] {
            updateGeometry();
        });

        connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, [ = ] {
            updateGeometry();
        });
    }

    connect(m_notifySettings, &NotifySettings::appSettingChanged, this, [ = ] (const QString &id, const uint &item, QVariant var) {
        Q_EMIT AppInfoChanged(id, item, QDBusVariant(var));
    });
    connect(m_notifySettings, &NotifySettings::systemSettingChanged, this, [ = ] (const uint &item, QVariant var) {
        Q_EMIT SystemInfoChanged(item, QDBusVariant(var));
    });
    connect(m_notifySettings, &NotifySettings::appAddedSignal, this, [ = ] (const QString &id) {
        Q_EMIT AppAddedSignal(id);
        Q_EMIT appAdded(m_notifySettings->getAppSettings_v1(id));
    });
    connect(m_notifySettings, &NotifySettings::appRemovedSignal, this, [ = ] (const QString &id) {
        Q_EMIT AppRemovedSignal(id);
        Q_EMIT appRemoved(id);
    });

    if (useBuiltinBubble()) {
        // 响应任务栏方向改变信号，更新额外触屏划入距离
        connect(m_dockDeamonInter, &DockInter::PositionChanged, this, [ this ] {
            m_slideWidth = (m_dockDeamonInter->position() == OSD::DockPosition::Right) ? 100 : 0;
        });

        connect(m_appearance, &Appearance::OpacityChanged, this,  &BubbleManager::onOpacityChanged);
    }

    connect(&SignalBridge::ref(), &SignalBridge::actionInvoked, this, &BubbleManager::ActionInvoked);
    connect(m_persistence, &AbstractPersistence::recordCountChanged, this, &BubbleManager::recordCountChanged);
}

void BubbleManager::onPrepareForSleep(bool sleep)
{
    // workaround to avoid the "About to suspend..." notifications still
    // hanging there on restoring from sleep confusing users.
    if (!sleep) {
        qDebug() << "Quit on restoring from sleep.";
        qApp->quit();
    }
}

void BubbleManager::geometryChanged()
{
    if (!useBuiltinBubble())
        return;

    m_currentDisplayRect = calcDisplayRect();
    // dock未启动时，不要调用其接口，会导致系统刚启动是任务栏被提前启动（比窗管还早），造成显示异常，后续应该改成通知中心显示时才调用任务栏的接口，否则不应调用
    if (m_dockInter->isValid()) {
        m_currentDockRect = m_dockInter->geometry();
    }

    m_dockPos = static_cast<OSD::DockPosition>(m_dockDeamonInter->position());
    m_dockMode = m_dockDeamonInter->displayMode();
    m_notifyCenter->updateGeometry(m_currentDisplayRect, m_currentDockRect, m_dockPos, m_dockMode);
    updateGeometry();
}

bool BubbleManager::calcReplaceId(EntityPtr notify)
{
    bool find = false;

    if (notify->replacesId() == NoReplaceId) {
        notify->setId(QString::number(++m_replaceCount));
        notify->setReplacesId(QString::number(m_replaceCount));
    } else {
        for (int i = 0; i < m_bubbleList.size(); ++i) {
            Bubble *bubble = m_bubbleList.at(i);
            if (bubble->entity()->replacesId() == notify->replacesId()
                    && bubble->entity()->appName() == notify->appName()) {
                // remove old one
                if (!bubble->entity()->storageId().isEmpty())
                    m_persistence->removeOne(m_bubbleList.at(i)->entity()->storageId());

                m_persistence->addOne(m_bubbleList.at(i)->entity());
                // emit RecordAdded for replaced entity
                emitRecordAdded(m_bubbleList.at(i)->entity());
                if (i != 0) {
                    bubble->setEntity(m_bubbleList.at(i)->entity());
                }
                m_bubbleList.at(i)->setEntity(notify);
                find = true;
            }
        }

        for (int i = 0; i < m_oldEntities.size(); ++i) {
            if (m_oldEntities.at(i)->replacesId() == notify->replacesId()
                    && m_oldEntities.at(i)->appName() == notify->appName()) {
                // emit RecordAdded for notProcessedYet entity
                emitRecordAdded(m_oldEntities.at(i));
                m_oldEntities.removeAt(i);
            }
        }
        if (find) {
            // Add persistence for Replace Entity.
            if (notify->isShowInNotifyCenter())
                m_persistence->addOne(notify);
        }
    }

    return find;
}

Bubble *BubbleManager::createBubble(EntityPtr notify, int index)
{
    Bubble *bubble = new Bubble(nullptr, notify);
    bubble->setMaskAlpha(static_cast<quint8>(m_appearance->opacity() * 255));
    connect(m_appearance, &Appearance::OpacityChanged, bubble, &Bubble::onOpacityChanged);
    connect(bubble, &Bubble::expired, this, &BubbleManager::bubbleExpired);
    connect(bubble, &Bubble::dismissed, this, &BubbleManager::bubbleDismissed);
    connect(bubble, &Bubble::actionInvoked, this, &BubbleManager::bubbleActionInvoked);
    connect(bubble, &Bubble::processed, this, [this](EntityPtr ptr){
        m_persistence->removeOne(ptr->storageId());
    });
    connect(bubble, &Bubble::notProcessedYet, this, [ this ](EntityPtr ptr) {
        emitRecordAdded(ptr);
    });

    if (index != 0) {
        QRect startRect = getBubbleGeometry(BubbleEntities + BubbleOverLap);
        QRect endRect = getBubbleGeometry(BubbleEntities + BubbleOverLap - 1);
        bubble->setBubbleIndex(BubbleEntities + BubbleOverLap - 1);
        bubble->startMove(startRect, endRect);
    } else {
        QRect endRect = getBubbleGeometry(0);
        QRect startRect = endRect;
        startRect.setHeight(1);

        bubble->setProperty("geometry",0);
        bubble->show();

        QPropertyAnimation *ani = new QPropertyAnimation(bubble, "geometry", this);
        ani->setStartValue(startRect);
        ani->setEndValue(endRect);

        int animationTime = int(endRect.height() * 1.0 / 72 * AnimationTime);
        ani->setDuration(animationTime);

        bubble->setBubbleIndex(0);
        ani->start(QPropertyAnimation::DeleteWhenStopped);
    }

    return bubble;
}
