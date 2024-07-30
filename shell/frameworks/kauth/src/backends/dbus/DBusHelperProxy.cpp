/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "DBusHelperProxy.h"
#include "BackendsManager.h"
#include "kauthdebug.h"
#include "kf6authadaptor.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusUnixFileDescriptor>
#include <QMap>
#include <QMetaMethod>
#include <QObject>
#include <QTimer>
#include <qplugin.h>

extern Q_CORE_EXPORT const QtPrivate::QMetaTypeInterface *qMetaTypeGuiHelper;

namespace KAuth
{
static void debugMessageReceived(int t, const QString &message);

DBusHelperProxy::DBusHelperProxy()
    : responder(nullptr)
    , m_stopRequest(false)
    , m_busConnection(QDBusConnection::systemBus())
{
    qDBusRegisterMetaType<QMap<QString, QDBusUnixFileDescriptor>>();
}

DBusHelperProxy::DBusHelperProxy(const QDBusConnection &busConnection)
    : responder(nullptr)
    , m_stopRequest(false)
    , m_busConnection(busConnection)
{
    qDBusRegisterMetaType<QMap<QString, QDBusUnixFileDescriptor>>();
}

DBusHelperProxy::~DBusHelperProxy()
{
}

void DBusHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf6auth"), QLatin1String("stopAction"));

    QList<QVariant> args;
    args << action;
    message.setArguments(args);

    m_busConnection.asyncCall(message);
}

void DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const DetailsMap &details, const QVariantMap &arguments, int timeout)
{
    QMap<QString, QDBusUnixFileDescriptor> fds;
    QVariantMap nonFds;
    for (auto [key, value] : arguments.asKeyValueRange()) {
        if (value.metaType() == QMetaType::fromType<QDBusUnixFileDescriptor>()) {
            fds.insert(key, value.value<QDBusUnixFileDescriptor>());
        } else {
            nonFds.insert(key, value);
        }
    }

    QByteArray blob;
    {
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << nonFds;
    }

    // on unit tests we won't have a service, but the service will already be running
    const auto reply = m_busConnection.interface()->startService(helperID);
    if (!reply.isValid() && !m_busConnection.interface()->isServiceRegistered(helperID)) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: service start %1 failed: %2").arg(helperID, reply.error().message()));
        Q_EMIT actionPerformed(action, errorReply);
        return;
    }

    const bool connected = m_busConnection.connect(helperID,
                                                   QLatin1String("/"),
                                                   QLatin1String("org.kde.kf6auth"),
                                                   QLatin1String("remoteSignal"),
                                                   this,
                                                   SLOT(remoteSignalReceived(int, QString, QByteArray)));

    // if already connected reply will be false but we won't have an error or a reason to fail
    if (!connected && m_busConnection.lastError().isValid()) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: connection to helper failed. %1\n(application: %2 helper: %3)")
                                           .arg(m_busConnection.lastError().message(), qApp->applicationName(), helperID));
        Q_EMIT actionPerformed(action, errorReply);
        return;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf6auth"), QLatin1String("performAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << BackendsManager::authBackend()->backendDetails(details) << blob << QVariant::fromValue(fds);
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QDBusPendingCall pendingCall = m_busConnection.asyncCall(message, timeout);

    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, action, args, message, watcher, timeout]() mutable {
        watcher->deleteLater();

        QDBusMessage reply = watcher->reply();

        if (reply.type() == QDBusMessage::ErrorMessage) {
            if (watcher->error().type() == QDBusError::InvalidArgs) {
                // For backwards compatibility if helper binary was built with older KAuth version.
                args.removeAt(args.count() - 2); // remove backend details
                message.setArguments(args);
                reply = m_busConnection.call(message, QDBus::Block, timeout);
                if (reply.type() != QDBusMessage::ErrorMessage) {
                    return;
                }
            }
            ActionReply r = ActionReply::DBusErrorReply();
            r.setErrorDescription(tr("DBus Backend error: could not contact the helper. "
                                     "Connection error: %1. Message error: %2")
                                      .arg(reply.errorMessage(), m_busConnection.lastError().message()));
            qCWarning(KAUTH) << reply.errorMessage();

            Q_EMIT actionPerformed(action, r);
        }
    });
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new Kf6authAdaptor(this);

    if (!m_busConnection.registerService(name)) {
        qCWarning(KAUTH) << "Error registering helper DBus service" << name << m_busConnection.lastError().message();
        return false;
    }

    if (!m_busConnection.registerObject(QLatin1String("/"), this)) {
        qCWarning(KAUTH) << "Error registering helper DBus object:" << m_busConnection.lastError().message();
        return false;
    }

    m_name = name;

    return true;
}

void DBusHelperProxy::setHelperResponder(QObject *o)
{
    responder = o;
}

void DBusHelperProxy::remoteSignalReceived(int t, const QString &action, QByteArray blob)
{
    SignalType type = static_cast<SignalType>(t);
    QDataStream stream(&blob, QIODevice::ReadOnly);

    if (type == ActionStarted) {
        Q_EMIT actionStarted(action);
    } else if (type == ActionPerformed) {
        ActionReply reply = ActionReply::deserialize(blob);

        m_actionsInProgress.removeOne(action);
        Q_EMIT actionPerformed(action, reply);
    } else if (type == DebugMessage) {
        int level;
        QString message;

        stream >> level >> message;

        debugMessageReceived(level, message);
    } else if (type == ProgressStepIndicator) {
        int step;
        stream >> step;

        Q_EMIT progressStep(action, step);
    } else if (type == ProgressStepData) {
        QVariantMap data;
        stream >> data;
        Q_EMIT progressStepData(action, data);
    }
}

void DBusHelperProxy::stopAction(const QString &action)
{
    Q_UNUSED(action)
    //#warning FIXME: The stop request should be action-specific rather than global
    m_stopRequest = true;
}

bool DBusHelperProxy::hasToStopAction()
{
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);

    return m_stopRequest;
}

bool DBusHelperProxy::isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details)
{
    Q_UNUSED(callerID); // this only exists for the benefit of the mac backend. We obtain our callerID from dbus!
    return BackendsManager::authBackend()->isCallerAuthorized(action, message().service().toUtf8(), details);
}

QByteArray DBusHelperProxy::performAction(const QString &action,
                                          const QByteArray &callerID,
                                          const QVariantMap &details,
                                          QByteArray arguments,
                                          const QMap<QString, QDBusUnixFileDescriptor> &fdArguments)
{
    if (!responder) {
        return ActionReply::NoResponderReply().serialized();
    }

    if (!m_currentAction.isEmpty()) {
        return ActionReply::HelperBusyReply().serialized();
    }

    // Make sure we don't try restoring gui variants, in particular QImage/QPixmap/QIcon are super dangerous
    // since they end up calling the image loaders and thus are a vector for crashing → executing code
    auto origMetaTypeGuiHelper = qMetaTypeGuiHelper;
    qMetaTypeGuiHelper = nullptr;

    QVariantMap args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;

    for (auto [key, value] : fdArguments.asKeyValueRange()) {
        args.insert(key, QVariant::fromValue(value));
    }

    qMetaTypeGuiHelper = origMetaTypeGuiHelper;

    m_currentAction = action;
    Q_EMIT remoteSignal(ActionStarted, action, QByteArray());
    QEventLoop e;
    e.processEvents(QEventLoop::AllEvents);

    ActionReply retVal;

    QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer *>();
    timer->stop();

    if (isCallerAuthorized(action, callerID, details)) {
        QString slotname = action;
        if (slotname.startsWith(m_name + QLatin1Char('.'))) {
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        }

        slotname.replace(QLatin1Char('.'), QLatin1Char('_'));

        // For legacy reasons we could be dealing with ActionReply types (i.e.
        // `using namespace KAuth`). Since Qt type names are verbatim this would
        // mismatch a return type that is called 'KAuth::ActionReply' and
        // vice versa. This effectively required client code to always 'use' the
        // namespace as otherwise we'd not be able to call into it.
        // To support both scenarios we now dynamically determine what kind of return type
        // we deal with and call Q_RETURN_ARG either with or without namespace.
        const auto metaObj = responder->metaObject();
        const QString slotSignature(slotname + QStringLiteral("(QVariantMap)"));
        const QMetaMethod method = metaObj->method(metaObj->indexOfMethod(qPrintable(slotSignature)));
        if (method.isValid()) {
            const auto needle = "KAuth::";
            bool success = false;
            if (strncmp(needle, method.typeName(), strlen(needle)) == 0) {
                success = method.invoke(responder, Qt::DirectConnection, Q_RETURN_ARG(KAuth::ActionReply, retVal), Q_ARG(QVariantMap, args));
            } else {
                success = method.invoke(responder, Qt::DirectConnection, Q_RETURN_ARG(ActionReply, retVal), Q_ARG(QVariantMap, args));
            }
            if (!success) {
                retVal = ActionReply::NoSuchActionReply();
            }
        } else {
            retVal = ActionReply::NoSuchActionReply();
        }
    } else {
        retVal = ActionReply::AuthorizationDeniedReply();
    }

    timer->start();

    Q_EMIT remoteSignal(ActionPerformed, action, retVal.serialized());
    e.processEvents(QEventLoop::AllEvents);
    m_currentAction.clear();
    m_stopRequest = false;

    return retVal.serialized();
}

void DBusHelperProxy::sendDebugMessage(int level, const char *msg)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << level << QString::fromLocal8Bit(msg);

    Q_EMIT remoteSignal(DebugMessage, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(int step)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << step;

    Q_EMIT remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStepData(const QVariantMap &data)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << data;

    Q_EMIT remoteSignal(ProgressStepData, m_currentAction, blob);
}

void debugMessageReceived(int t, const QString &message)
{
    QtMsgType type = static_cast<QtMsgType>(t);
    switch (type) {
    case QtDebugMsg:
        qDebug("Debug message from helper: %s", message.toLatin1().data());
        break;
    case QtInfoMsg:
        qInfo("Info message from helper: %s", message.toLatin1().data());
        break;
    case QtWarningMsg:
        qWarning("Warning from helper: %s", message.toLatin1().data());
        break;
    case QtCriticalMsg:
        qCritical("Critical warning from helper: %s", message.toLatin1().data());
        break;
    case QtFatalMsg:
        qFatal("Fatal error from helper: %s", message.toLatin1().data());
        break;
    }
}

int DBusHelperProxy::callerUid() const
{
    QDBusConnectionInterface *iface = connection().interface();
    if (!iface) {
        return -1;
    }
    return iface->serviceUid(message().service());
}

} // namespace KAuth

#include "moc_DBusHelperProxy.cpp"
