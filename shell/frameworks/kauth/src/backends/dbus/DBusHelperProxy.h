/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DBUS_HELPER_PROXY_H
#define DBUS_HELPER_PROXY_H

#include "HelperProxy.h"
#include "actionreply.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>
#include <QVariant>

namespace KAuth
{
class DBusHelperProxy : public HelperProxy, protected QDBusContext
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.DBusHelperProxy")
    Q_INTERFACES(KAuth::HelperProxy)

    QObject *responder;
    QString m_name;
    QString m_currentAction;
    bool m_stopRequest;
    QList<QString> m_actionsInProgress;
    QDBusConnection m_busConnection;

    enum SignalType {
        ActionStarted, // The blob argument is empty
        ActionPerformed, // The blob argument contains the ActionReply
        DebugMessage, // The blob argument contains the debug level and the message (in this order)
        ProgressStepIndicator, // The blob argument contains the step indicator
        ProgressStepData, // The blob argument contains the QVariantMap
    };

public:
    DBusHelperProxy();
    DBusHelperProxy(const QDBusConnection &busConnection);

    ~DBusHelperProxy() override;

    virtual void
    executeAction(const QString &action, const QString &helperID, const DetailsMap &details, const QVariantMap &arguments, int timeout = -1) override;
    void stopAction(const QString &action, const QString &helperID) override;

    bool initHelper(const QString &name) override;
    void setHelperResponder(QObject *o) override;
    bool hasToStopAction() override;
    void sendDebugMessage(int level, const char *msg) override;
    void sendProgressStep(int step) override;
    void sendProgressStepData(const QVariantMap &data) override;

    int callerUid() const override;

public Q_SLOTS:
    void stopAction(const QString &action);
    QByteArray performAction(const QString &action,
                             const QByteArray &callerID,
                             const QVariantMap &details,
                             QByteArray arguments,
                             const QMap<QString, QDBusUnixFileDescriptor> &fdArguments);

Q_SIGNALS:
    void remoteSignal(int type, const QString &action, const QByteArray &blob); // This signal is sent from the helper to the app

private Q_SLOTS:
    void remoteSignalReceived(int type, const QString &action, QByteArray blob);

private:
    bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details);
};

} // namespace Auth

#endif
