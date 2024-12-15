// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationcenterpanel.h"

#include "notificationcenterproxy.h"
#include "notificationcenterdbusadaptor.h"
#include "notifyaccessor.h"
#include "dbaccessor.h"

#include <pluginfactory.h>
#include <pluginloader.h>
#include <applet.h>
#include <containment.h>
#include <appletbridge.h>

#include <QQueue>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QLoggingCategory>
#include <QDBusConnectionInterface>

DS_USE_NAMESPACE

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notification {

static const QString OCEANNotifyDBusServer = "org.lingmo.ocean.Notification1";
static const QString OCEANNotifyDBusInterface = "org.lingmo.ocean.Notification1";
static const QString OCEANNotifyDBusPath = "/org/lingmo/ocean/Notification1";

static QDBusInterface notifyCenterInterface()
{
    return QDBusInterface(OCEANNotifyDBusServer, OCEANNotifyDBusPath, OCEANNotifyDBusInterface);
}

NotificationCenterPanel::NotificationCenterPanel(QObject *parent)
    : DPanel(parent)
    , m_proxy(new NotificationCenterProxy(this))
{
}

NotificationCenterPanel::~NotificationCenterPanel()
{
}

bool NotificationCenterPanel::load()
{
    return DPanel::load();
}

bool NotificationCenterPanel::init()
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerObject("/org/lingmo/ocean/shell/notification/center",
                            "org.lingmo.ocean.shell.notification.center",
                            m_proxy,
                            QDBusConnection::ExportAllSlots)) {
        qWarning(notifyLog) << QString("Can't register to the D-Bus object.");
        return false;
    }

    // TODO compatible with old notification center
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService("org.lingmo.ocean.Widgets1",
                                            QDBusConnectionInterface::ReplaceExistingService,
                                            QDBusConnectionInterface::AllowReplacement);
    if (!connection.registerObject("/org/lingmo/ocean/Widgets1", m_proxy)) {
        return false;
    }
    new NotificationCenterDBusAdaptor(m_proxy);

    DPanel::init();

    auto accessor = notification::DBAccessor::instance();
    notifycenter::NotifyAccessor::instance()->setDataAccessor(accessor);

    bool valid = false;
    DAppletBridge bridge("org.lingmo.ds.notificationserver");
    if (auto server = bridge.applet()) {
        valid = QObject::connect(server,
                                 SIGNAL(notificationStateChanged(qint64, int)),
                                 notifycenter::NotifyAccessor::instance(),
                                 SLOT(onNotificationStateChanged(qint64, int)),
                                 Qt::QueuedConnection);
        notifycenter::NotifyAccessor::instance()->setDataUpdater(server);
        notifycenter::NotifyAccessor::instance()->setEnabled(visible());
    } else {
        // old interface by dbus
        auto connection = QDBusConnection::sessionBus();
        valid = connection.connect(OCEANNotifyDBusServer, OCEANNotifyDBusPath, OCEANNotifyDBusInterface,
                                   "RecordAoceand", this, SLOT(onReceivedRecord(const QString &)));
    }
    if (!valid) {
        qWarning(notifyLog) << "NotifyConnection is invalid, and can't receive RecordAoceand signal.";
    }

    return true;
}

bool NotificationCenterPanel::visible() const
{
    return m_visible;
}

void NotificationCenterPanel::setVisible(bool newVisible)
{
    if (m_visible == newVisible)
        return;
    m_visible = newVisible;
    notifycenter::NotifyAccessor::instance()->setEnabled(m_visible);
    setBubblePanelEnabled(!m_visible);
    emit visibleChanged();
}

void NotificationCenterPanel::close()
{
    if (m_proxy) {
        m_proxy->Hide();
    }
}

void NotificationCenterPanel::setBubblePanelEnabled(bool enabled)
{
    DAppletBridge bridge("org.lingmo.ds.notificationbubble");
    if (auto applet = bridge.applet())
        QMetaObject::invokeMethod(applet, "setEnabled", Qt::DirectConnection, Q_ARG(bool, enabled));
}

D_APPLET_CLASS(NotificationCenterPanel)
}

#include "notificationcenterpanel.moc"
