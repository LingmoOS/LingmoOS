// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationmanager.h"
#include "notificationsetting.h"
#include "notifyentity.h"
#include "dbaccessor.h"

#include <QDateTime>
#include <DDesktopServices>
#include <DSGApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <DConfig>

#include <applet.h>
#include <appletbridge.h>

DCORE_USE_NAMESPACE
DS_USE_NAMESPACE

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notification {

static const uint NoReplacesId = 0;
static const int DefaultTimeOutMSecs = 5000;
static const QString NotificationsDBusService = "org.freedesktop.Notifications";
static const QString NotificationsDBusPath = "/org/freedesktop/Notifications";
static const QString OCEANNotifyDBusServer = "org.lingmo.ocean.Notification1";
static const QString OCEANNotifyDBusPath = "/org/lingmo/ocean/Notification1";
static const QString SessionDBusService = "org.lingmo.ocean.SessionManager1";
static const QString SessionDaemonDBusPath = "/org/lingmo/ocean/SessionManager1";

NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent)
    , m_persistence(new DBAccessor("Manager"))
    , m_setting(new NotificationSetting(this))
    , m_userSessionManager(new UserSessionManager(SessionDBusService, SessionDaemonDBusPath, QDBusConnection::sessionBus(), this))
    , m_pendingTimeout(new QTimer(this))
{
    m_pendingTimeout->setSingleShot(true);
    connect(m_pendingTimeout, &QTimer::timeout, this, &NotificationManager::onHandingPendingEntities);

    DAppletBridge bridge("org.lingmo.ds.ocean-apps");
    if (auto apps = bridge.applet()) {
        if (auto model = apps->property("appModel").value<QAbstractItemModel *>()) {
            m_setting->setAppAccessor(model);
        }
    }
    if (!m_setting->appAccessor()) {
        qWarning(notifyLog) << "It's not exist appModel for the applet:" << bridge.pluginId();
    }
    connect(m_setting, &NotificationSetting::appAoceand, this, &NotificationManager::AppAoceand);
    connect(m_setting, &NotificationSetting::appRemoved, this, &NotificationManager::AppRemoved);
    connect(m_setting, &NotificationSetting::appValueChanged, this, [this] (const QString &appId, uint configItem, const QVariant &value) {
        Q_EMIT AppInfoChanged(appId, configItem, QDBusVariant(value));
    });
    connect(m_setting, &NotificationSetting::systemValueChanged, this, [this] (uint configItem, const QVariant &value) {
        Q_EMIT SystemInfoChanged(configItem, QDBusVariant(value));
    });

    QScopedPointer<DConfig> config(DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell.notification"));
    m_systemApps = config->value("systemApps").toStringList();
    // TODO temporary fix for AppNamesMap
    m_appNamesMap = config->value("AppNamesMap").toMap();
}

NotificationManager::~NotificationManager()
{
    if (m_persistence) {
        delete m_persistence;
        m_persistence = nullptr;
    }
}

bool NotificationManager::registerDbusService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(NotificationsDBusService,
                                            QDBusConnectionInterface::ReplaceExistingService,
                                            QDBusConnectionInterface::AllowReplacement);
    if (!connection.registerObject(NotificationsDBusPath, this)) {
        return false;
    }

    QDBusConnection oceanNotifyConnect = QDBusConnection::sessionBus();
    oceanNotifyConnect.interface()->registerService(OCEANNotifyDBusServer,
                                                  QDBusConnectionInterface::ReplaceExistingService,
                                                  QDBusConnectionInterface::AllowReplacement);
    if (!oceanNotifyConnect.registerObject(OCEANNotifyDBusPath, this)) {
        return false;
    }

    return true;
}

uint NotificationManager::recordCount() const
{
    return m_persistence->fetchEntityCount(DataAccessor::AllApp(), NotifyEntity::Processed);
}

void NotificationManager::actionInvoked(qint64 id, uint bubbleId, const QString &actionKey)
{
    qInfo(notifyLog) << "Action invoked, bubbleId:" << bubbleId << ", id:" << id << ", actionKey" << actionKey;
    auto entity = m_persistence->fetchEntity(id);
    if (entity.isValid()) {
        doActionInvoked(entity, actionKey);

        entity.setProcessedType(NotifyEntity::Removed);
        updateEntityProcessed(entity);
    }

    Q_EMIT ActionInvoked(bubbleId, actionKey);
    Q_EMIT NotificationClosed(bubbleId, NotifyEntity::Closed);
}

void NotificationManager::notificationClosed(qint64 id, uint bubbleId, uint reason)
{
    qDebug(notifyLog) << "Close notification id" << id << ", reason" << reason;
    updateEntityProcessed(id, reason);

    Q_EMIT NotificationClosed(bubbleId, reason);
}

void NotificationManager::notificationReplaced(qint64 id)
{
    updateEntityProcessed(id, NotifyEntity::Closed);
}

void NotificationManager::removeNotification(qint64 id)
{
    m_persistence->removeEntity(id);

    emitRecordCountChanged();
}

void NotificationManager::removeNotifications(const QString &appName)
{
    m_persistence->removeEntityByApp(appName);

    emitRecordCountChanged();
}

void NotificationManager::removeNotifications()
{
    m_persistence->clear();

    emitRecordCountChanged();
}

QStringList NotificationManager::GetCapabilities()
{
    QStringList result;
    result << "action-icons" << "actions" << "body" << "body-hyperlinks" << "body-markup" << "body-image" << "enable-sound" << "persistence";

    return result;
}

uint NotificationManager::Notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary,
                                 const QString &body, const QStringList &actions, const QVariantMap &hints,
                                 int expireTimeout)
{
    qDebug(notifyLog) << "Notify"
            << ", appName:" << appName
            << ", summary:" << summary
            << ", appIcon:" << appIcon
            << ", body size:" << body.size()
            << ", actions:" << actions
            << ", hint: " << hints
            << ", replaceId:" << replacesId
            << ", expireTimeout:" << expireTimeout;

    if (calledFromDBus() && m_setting->systemValue(NotificationSetting::CloseNotification).toBool()) {
        qDebug(notifyLog) << "Notify has been disabled by CloseNotification setting.";
        return 0;
    }

    QString appId = appIdByAppName(appName);

    if (appId.isEmpty() && calledFromDBus()) {
        QDBusReply<uint> reply = connection().interface()->servicePid(message().service());
        appId = DSGApplication::getId(reply.value());
    }

    if (appId.isEmpty())
        appId = appName;

    bool enableAppNotification = m_setting->appValue(appId, NotificationSetting::EnableNotification).toBool();
    if (!enableAppNotification && !m_systemApps.contains(appId)) {
        return 0;
    }

    QString tsAppName = m_setting->appValue(appId, NotificationSetting::AppName).toString();

    QString strBody = body;
    strBody.replace(QLatin1String("\\\\"), QLatin1String("\\"), Qt::CaseInsensitive);

    QString strIcon = appIcon;
    if (strIcon.isEmpty())
        strIcon = m_setting->appValue(appId, NotificationSetting::AppIcon).toString();
    NotifyEntity entity(tsAppName, replacesId, strIcon, summary, strBody, actions, hints, expireTimeout);
    entity.setAppId(appId);
    entity.setProcessedType(NotifyEntity::None);
    entity.setReplacesId(replacesId);

    bool lockScreenShow = true;
    bool dndMode = isDoNotDisturb();
    bool systemNotification = m_systemApps.contains(appId);
    bool lockScreen = m_userSessionManager->locked();
    const bool desktopScreen = !lockScreen;

    if (!systemNotification) {
        lockScreenShow = m_setting->appValue(appId, NotificationSetting::ShowOnLockScreen).toBool();
    }
    const bool onDesktopShow = m_setting->appValue(appId, NotificationSetting::ShowOnDesktop).toBool();

    tryPlayNotificationSound(entity, appId, dndMode);

    // new one
    if (replacesId == NoReplacesId) {
        entity.setBubbleId(++m_replacesCount);

        if (systemNotification) { // 系统通知
            entity.setProcessedType(NotifyEntity::NotProcessed);
        } else if (lockScreen && !lockScreenShow) { // 锁屏不显示通知
            entity.setProcessedType(NotifyEntity::Processed);
        } else if (desktopScreen && !onDesktopShow) { // 桌面不显示通知
            entity.setProcessedType(NotifyEntity::Processed);
        } else { // 锁屏显示通知或者未锁屏状态
            if (!systemNotification && !dndMode && enableAppNotification) { // 普通应用非勿扰模式并且开启通知选项
                entity.setProcessedType(NotifyEntity::NotProcessed);
            } else {
                entity.setProcessedType(NotifyEntity::Processed);
            }
        }
    } else { // maybe replace one
        entity.setBubbleId(replacesId);
        entity.setProcessedType(NotifyEntity::NotProcessed);
    }

    if (entity.processedType() != NotifyEntity::None) {
        qint64 id = -1;
        if (entity.isReplace()) {
            auto lastEntity = m_persistence->fetchLastEntity(entity.bubbleId());
            if (lastEntity.isValid()) {
                removePendingEntity(entity);
                id = m_persistence->replaceEntity(lastEntity.id(), entity);
            } else {
                qWarning() << "Not exist notification to replace for the replaceId" << replacesId;
            }
        }
        if (id == -1) {
            id = m_persistence->addEntity(entity);
        }

        if (id == -1) {
            qWarning(notifyLog) << "Failed on saving DB, bubbleId:" << entity.bubbleId() << ", appName" << appName;
            return 0;
        }

        entity.setId(id);

        emitRecordCountChanged();

        Q_EMIT NotificationStateChanged(entity.id(), entity.processedType());

        bool critical = false;
        if (auto iter = hints.find("urgency"); iter != hints.end()) {
            critical = iter.value().toUInt() == NotifyEntity::Critical;
        }
        // 0: never expire. -1: DefaultTimeOutMSecs
        if (expireTimeout != 0 && !critical) {
            pushPendingEntity(entity, expireTimeout);
        }
    }

    qInfo(notifyLog) << "Notify done, bubbleId:" << entity.bubbleId() << ", id:" << entity.id() << ", type:" << entity.processedType();

    // If replaces_id is 0, the return value is a UINT32 that represent the notification.
    // If replaces_id is not 0, the returned value is the same value as replaces_id.
    return entity.bubbleId();
}
void NotificationManager::CloseNotification(uint id)
{
    auto entity = m_persistence->fetchLastEntity(id);
    if (entity.isValid()) {
        entity.setProcessedType(NotifyEntity::Removed);
        updateEntityProcessed(entity);
    }

    Q_EMIT NotificationClosed(id, NotifyEntity::Closed);
    qDebug(notifyLog) << "Close notify, bubbleId" << id << ", id:" << entity.id();
}

void NotificationManager::GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion)
{
    name = QString("LingmoNotifications");
    vendor = QString("Lingmo");
    version = QString("3.0");

    specVersion = QString("1.2");
}

QStringList NotificationManager::GetAppList()
{
    qDebug(notifyLog) << "Get appList";
    return m_setting->apps();
}

QVariant NotificationManager::GetAppInfo(const QString &appId, uint configItem)
{
    return m_setting->appValue(appId, static_cast<NotificationSetting::AppConfigItem>(configItem));
}

void NotificationManager::SetAppInfo(const QString &appId, uint configItem, const QVariant &value)
{
    qDebug(notifyLog) << "Set appInfo" << appId << configItem << value;
    return m_setting->setAppValue(appId, static_cast<NotificationSetting::AppConfigItem>(configItem), value);
}

void NotificationManager::SetSystemInfo(uint configItem, const QVariant &value)
{
    qDebug(notifyLog) << "Set systemInfo" << configItem << value;
    return m_setting->setSystemValue(static_cast<NotificationSetting::SystemConfigItem>(configItem), value);
}

QVariant NotificationManager::GetSystemInfo(uint configItem)
{
    qDebug(notifyLog) << "Get systemInfo" << configItem;
    return m_setting->systemValue(static_cast<NotificationSetting::SystemConfigItem>(configItem));
}

bool NotificationManager::isDoNotDisturb() const
{
    if (!m_setting->systemValue(NotificationSetting::DNDMode).toBool())
        return false;

    // 未点击按钮  任何时候都勿扰模式
    if (!m_setting->systemValue(NotificationSetting::OpenByTimeInterval).toBool() &&
        !m_setting->systemValue(NotificationSetting::LockScreenOpenDNDMode).toBool()) {
        return true;
    }

    bool lockScreen = m_userSessionManager->locked();
    // 点击锁屏时 并且 锁屏状态 任何时候都勿扰模式
    if (m_setting->systemValue(NotificationSetting::LockScreenOpenDNDMode).toBool() && lockScreen)
        return true;

    QTime currentTime = QTime::fromString(QDateTime::currentDateTime().toString("hh:mm"));
    QTime startTime = QTime::fromString(m_setting->systemValue(NotificationSetting::StartTime).toString());
    QTime endTime = QTime::fromString(m_setting->systemValue(NotificationSetting::EndTime).toString());

    bool dndMode = true;
    if (startTime < endTime) {
        dndMode = startTime <= currentTime && endTime >= currentTime;
    } else if (startTime > endTime) {
        dndMode = startTime <= currentTime || endTime >= currentTime;
    }

    return dndMode && m_setting->systemValue(NotificationSetting::OpenByTimeInterval).toBool();
}

void NotificationManager::tryPlayNotificationSound(const NotifyEntity &entity, const QString &appId, bool dndMode) const
{
    const auto hints = entity.hints();
    if (!hints.isEmpty() && (!hints.value("enable-sound", true).toBool() ||
        !hints.value("x-lingmo-PlaySound", true).toBool())) {
        return;
    }

    bool playSoundTip = false;
    bool playSound = true;
    bool systemNotification = m_systemApps.contains(appId);
    if (!systemNotification)
        playSound = m_setting->appValue(appId, NotificationSetting::EnableSound).toBool();

    if (playSound && !dndMode) {
        const auto actions = entity.actions();
        //接收蓝牙文件时，只在发送完成后才有提示音,"cancel"表示正在发送文件
        if (actions.contains("cancel")) {
            if (auto iter = hints.find("x-lingmo-action-_view"); iter != hints.end()) {
                const auto action = iter.value().toString();
                if (action.contains("xdg-open"))
                    playSoundTip = true;
            }
        } else {
            playSoundTip = true;
        }
    } else if (systemNotification && dndMode) {
        playSoundTip = true;
    }

    if (playSoundTip) {
        Dtk::Gui::DDesktopServices::playSystemSoundEffect(Dtk::Gui::DDesktopServices::SSE_Notifications);
    }
}

void NotificationManager::emitRecordCountChanged()
{
    const auto count = m_persistence->fetchEntityCount(DataAccessor::AllApp(), NotifyEntity::Processed);
    emit RecordCountChanged(count);
}

void NotificationManager::pushPendingEntity(const NotifyEntity &entity, int expireTimeout)
{
    const int interval = expireTimeout == -1 ? DefaultTimeOutMSecs : expireTimeout;

    qint64 point = QDateTime::currentMSecsSinceEpoch() + interval;
    m_pendingTimeoutEntities.insert(point, entity);

    if (m_lastTimeoutPoint > point) {
        m_lastTimeoutPoint = point;
        auto newInterval = m_lastTimeoutPoint - QDateTime::currentMSecsSinceEpoch();
        m_pendingTimeout->setInterval(newInterval);
        m_pendingTimeout->start();
    }
}

void NotificationManager::updateEntityProcessed(qint64 id, uint reason)
{
    auto entity = m_persistence->fetchEntity(id);
    if (entity.isValid()) {
        if (reason == NotifyEntity::Closed && entity.processedType() == NotifyEntity::NotProcessed) {
            entity.setProcessedType(NotifyEntity::Removed);
        } else {
            entity.setProcessedType(NotifyEntity::Processed);
        }
        updateEntityProcessed(entity);
    }
}

void NotificationManager::updateEntityProcessed(const NotifyEntity &entity)
{
    const auto id = entity.id();
    const bool removed = entity.processedType() == NotifyEntity::Removed;
    const auto showInCenter = m_setting->appValue(entity.appId(), NotificationSetting::ShowInCenter).toBool();
    // "cancel"表示正在发送蓝牙文件,不需要发送到通知中心
    const auto bluetooth = entity.body().contains("%") && entity.actions().contains("cancel");
    if (removed || !showInCenter || bluetooth) {
        m_persistence->removeEntity(id);
        removePendingEntity(entity);
    } else {
        m_persistence->updateEntityProcessedType(id, entity.processedType());
    }
    Q_EMIT NotificationStateChanged(id, entity.processedType());

    emitRecordCountChanged();
}

QString NotificationManager::appIdByAppName(const QString &appName) const
{
    const auto items = m_setting->appItems();
    for (const auto &item: items) {
        if (item.id == appName || item.appName == appName)
            return item.id;
    }

    if (m_appNamesMap.contains(appName)) {
        return m_appNamesMap.value(appName).toString();
    }

    return QString();
}

void NotificationManager::doActionInvoked(const NotifyEntity &entity, const QString &actionId)
{
    qDebug(notifyLog) << "Invoke the notification:" << entity.id() << entity.appName() << actionId;
    QMap<QString, QVariant> hints = entity.hints();
    QMap<QString, QVariant>::const_iterator i = hints.constBegin();
    while (i != hints.constEnd()) {
        QStringList args = i.value().toString().split(",");
        if (!args.isEmpty()) {
            QString cmd = args.takeFirst(); //命令
            if (i.key() == "x-lingmo-action-" + actionId) {
                QProcess::startDetached(cmd, args); //执行相关命令
            }
        }
        ++i;
    }
}

void NotificationManager::onHandingPendingEntities()
{
    QList<NotifyEntity> timeoutEntities;

    const auto current = QDateTime::currentMSecsSinceEpoch();
    for (auto iter = m_pendingTimeoutEntities.begin(); iter != m_pendingTimeoutEntities.end();) {
        const auto point = iter.key();
        if (point > current) {
            iter++;
            continue;
        }

        const auto entity = iter.value();;
        timeoutEntities << entity;
        iter = m_pendingTimeoutEntities.erase(iter);
    }

    // update pendingTimeout to deal with m_pendingTimeoutEntities
    if (!m_pendingTimeoutEntities.isEmpty()) {
        auto points = m_pendingTimeoutEntities.keys();
        std::sort(points.begin(), points.end());
        // find last point to restart pendingTimeout
        m_lastTimeoutPoint = points.first();
        auto newInterval = m_lastTimeoutPoint - current;
        m_pendingTimeout->setInterval(newInterval);
        m_pendingTimeout->start();
    } else {
        // reset m_lastTimeoutPoint
        m_lastTimeoutPoint = std::numeric_limits<qint64>::max();
    }

    for (const auto &item : timeoutEntities) {
        qDebug(notifyLog) << "Expired for the notification " << item.id() << item.appName();
        notificationClosed(item.id(), item.bubbleId(), NotifyEntity::Expired);
    }
}

void NotificationManager::removePendingEntity(const NotifyEntity &entity)
{
    for (auto iter = m_pendingTimeoutEntities.begin(); iter != m_pendingTimeoutEntities.end();) {
        const auto item = iter.value();
        if (item != entity) {
            iter++;
            continue;
        }
        m_pendingTimeoutEntities.erase(iter);
        onHandingPendingEntities();
        break;
    }
}

} // notification
