// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifyaccessor.h"

#include <QQmlEngine>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QProcess>
#include <QDBusReply>

#include <DConfig>

#include "dataaccessor.h"

DCORE_USE_NAMESPACE

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notifycenter {

static const uint ShowNotificationTop = 7;
static const QString InvalidApp {"DS-Invalid-Apps"};
static const QStringList InvalidPinnedApps {InvalidApp};

static QDBusInterface controlCenterInterface()
{
    return QDBusInterface("org.lingmo.ocean.ControlCenter1",
                          "/org/lingmo/ocean/ControlCenter1",
                          "org.lingmo.ocean.ControlCenter1");
}

class EventFilter : public QObject
{
    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut
            || event->type() == QEvent::KeyPress  || event->type() == QEvent::KeyRelease) {
            qDebug(notifyLog) << "Focus and Key event" << event->type() << watched << qApp->focusObject();
        }
        return false;
    }
};

NotifyAccessor::NotifyAccessor(QObject *parent)
    : m_pinnedApps(InvalidPinnedApps)
{
    if (!qEnvironmentVariableIsEmpty("DS_NOTIFICATION_DEBUG")) {
        const int value = qEnvironmentVariableIntValue("DS_NOTIFICATION_DEBUG");
        m_debugging = value;
    }
    if (m_debugging) {
        qApp->installEventFilter(new EventFilter());
    }
}

NotifyAccessor *NotifyAccessor::instance()
{
    static NotifyAccessor *instance = nullptr;

    if (!instance) {
        instance = new NotifyAccessor(qGuiApp);
        instance->setDataAccessor(new DataAccessor());
    }
    return instance;
}

NotifyAccessor *NotifyAccessor::create(QQmlEngine *, QJSEngine *)
{
    auto helper = NotifyAccessor::instance();
    QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
    return helper;
}

void NotifyAccessor::setDataAccessor(DataAccessor *accessor)
{
    if (m_accessor) {
        delete m_accessor;
    }
    m_accessor = accessor;
}

void NotifyAccessor::setDataUpdater(QObject *updater)
{
    m_dataUpdater = updater;
}

bool NotifyAccessor::enabled() const
{
    return m_enabled;
}

void NotifyAccessor::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

NotifyEntity NotifyAccessor::fetchEntity(qint64 id) const
{
    qDebug(notifyLog) << "Fetch entity" << id;
    auto ret = m_accessor->fetchEntity(id);
    return ret;
}

int NotifyAccessor::fetchEntityCount(const QString &appName) const
{
    qDebug(notifyLog) << "Fetch entity count for the app" << appName;
    auto ret = m_accessor->fetchEntityCount(appName, NotifyEntity::Processed);
    return ret;
}

NotifyEntity NotifyAccessor::fetchLastEntity(const QString &appName) const
{
    qDebug(notifyLog) << "Fetch last entity for the app" << appName;
    auto ret = m_accessor->fetchLastEntity(appName, NotifyEntity::Processed);
    return ret;
}

QList<NotifyEntity> NotifyAccessor::fetchEntities(const QString &appName, int maxCount)
{
    qDebug(notifyLog) << "Fetch entities for the app" << appName;
    auto ret = m_accessor->fetchEntities(appName, NotifyEntity::Processed, maxCount);
    return ret;
}

QStringList NotifyAccessor::fetchApps(int maxCount) const
{
    qDebug(notifyLog) << "Fetch apps count" << maxCount;
    auto ret = m_accessor->fetchApps(maxCount);
    return ret;
}

void NotifyAccessor::removeEntity(qint64 id)
{
    qDebug(notifyLog) << "Remove notify" << id;
    if (m_dataUpdater) {
        QMetaObject::invokeMethod(m_dataUpdater, "removeNotification", Qt::DirectConnection,
                                  Q_ARG(qint64, id));
    } else {
        m_accessor->removeEntity(id);
    }
}

void NotifyAccessor::removeEntityByApp(const QString &appName)
{
    qDebug(notifyLog) << "Remove notifies for the application" << appName;

    if (m_dataUpdater) {
        QMetaObject::invokeMethod(m_dataUpdater, "removeNotifications", Qt::DirectConnection,
                                  Q_ARG(const QString &, appName));
    } else {
        m_accessor->removeEntityByApp(appName);
    }
}

void NotifyAccessor::clear()
{
    qDebug(notifyLog) << "Remove all notify";

    if (m_dataUpdater) {
        QMetaObject::invokeMethod(m_dataUpdater, "removeNotifications", Qt::DirectConnection);
    } else {
        m_accessor->clear();
    }
}

void NotifyAccessor::closeNotify(const NotifyEntity &entity)
{
    if (!m_dataUpdater)
        return;
    const auto id = entity.id();
    const auto bubbleId = entity.bubbleId();
    QMetaObject::invokeMethod(m_dataUpdater, "notificationClosed", Qt::DirectConnection,
                              Q_ARG(qint64, id), Q_ARG(uint, bubbleId), Q_ARG(uint, NotifyEntity::Closed));
}

void NotifyAccessor::invokeNotify(const NotifyEntity &entity, const QString &actionId)
{
    if (!m_dataUpdater)
        return;
    const auto id = entity.id();
    const auto bubbleId = entity.bubbleId();
    qDebug(notifyLog) << "Invoke notify" << id << actionId;
    QMetaObject::invokeMethod(m_dataUpdater, "actionInvoked", Qt::DirectConnection,
                              Q_ARG(qint64, id), Q_ARG(uint, bubbleId), Q_ARG(const QString&, actionId));

}

// don't need to emit ActionInvoked of protocol.
void NotifyAccessor::invokeAction(const NotifyEntity &entity, const QString &actionId)
{
    qDebug(notifyLog) << "Invoke action for the notify" << entity.id() << actionId;

    QMap<QString, QVariant> hints = entity.hints();
    if (hints.isEmpty())
        return;
    QMap<QString, QVariant>::const_iterator i = hints.constBegin();
    while (i != hints.constEnd()) {
        QStringList args = i.value().toString().split(",");
        if (!args.isEmpty()) {
            QString cmd = args.first();
            args.removeFirst();
            if (i.key() == "x-lingmo-action-" + actionId) {
                qDebug(notifyLog) << "Invoke action" << cmd;
                QProcess::startDetached(cmd, args);
            }
        }
        ++i;
    }
}

void NotifyAccessor::pinApplication(const QString &appId, bool pin)
{
    qDebug(notifyLog) << "Pin the application" << appId << pin;

    if (!pin) {
        m_pinnedApps.removeOne(appId);
    } else {
        if (!m_pinnedApps.contains(appId))
            m_pinnedApps.append(appId);
    }
    QScopedPointer<DConfig> config(DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell.notification"));
    config->setValue("pinnedApps", m_pinnedApps);
}

bool NotifyAccessor::applicationPin(const QString &appId) const
{
    if (m_pinnedApps.contains(appId))
        return true;

    if (m_pinnedApps.contains(InvalidApp)) {
        QScopedPointer<DConfig> config(DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell.notification"));
        const_cast<NotifyAccessor*>(this)->m_pinnedApps = config->value("pinnedApps").toStringList();
    }

    return m_pinnedApps.contains(appId);
}

void NotifyAccessor::openNotificationSetting()
{
    qDebug(notifyLog) << "Open notification setting";
    QDBusReply<void> reply = controlCenterInterface().call("ShowPage",
                                                           "notification");
    if (reply.error().isValid()) {
        qWarning(notifyLog) << "Failed to Open notifycation setting" << reply.error().message();
        return;
    }
}

void NotifyAccessor::addNotify(const QString &appName, const QString &content)
{
    qDebug(notifyLog) << "Add notify" << appName;
    static int id = 10000;
    NotifyEntity entity(id++, appName);
    entity.setBody(content);
    m_accessor->addEntity(entity);

    if (auto entity = fetchLastEntity(appName); entity.isValid()) {
        entityReceived(entity.id());
    }
}

void NotifyAccessor::fetchDataInfo()
{
    QStringList info;
    auto entityCount = fetchEntityCount(DataAccessor::AllApp());
    auto apps = fetchApps();
    info.append(QString("notifyCount: %1, appCount: %2").arg(entityCount).arg(apps.size()));
    for (auto item : apps) {
        info.append(QString("%1 -> %2").arg(item).arg(fetchEntityCount(item)));
    }
    QString ret = info.join("\n");
    m_dataInfo = ret;
    dataInfoChanged();
    appsChanged();
}

void NotifyAccessor::onNotificationStateChanged(qint64 id, int processedType)
{
    if (!enabled())
        return;
    if (processedType == NotifyEntity::Processed) {
        emit entityReceived(id);
        emit stagingEntityClosed(id);
    } else if (processedType == NotifyEntity::NotProcessed) {
        emit stagingEntityReceived(id);
    }
}

void NotifyAccessor::onReceivedRecord(const QString &id)
{
    emit entityReceived(id.toLongLong());
}

QString NotifyAccessor::dataInfo() const
{
    return m_dataInfo;
}

QStringList NotifyAccessor::apps() const
{
    return m_apps;
}

bool NotifyAccessor::debugging() const
{
    return m_debugging;
}

}
