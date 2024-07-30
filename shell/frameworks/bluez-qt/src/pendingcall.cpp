/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "pendingcall.h"
#include "bluezqt_dbustypes.h"
#include "debug.h"
#include "obexfiletransferentry.h"
#include "obextransfer.h"
#include "obextransfer_p.h"

#include <QDBusPendingCallWatcher>
#include <QTimer>

namespace BluezQt
{
static PendingCall::Error nameToError(const QString &name)
{
    if (name.startsWith(QLatin1String("org.freedesktop.DBus.Error"))) {
        return PendingCall::DBusError;
    }

    if (!name.startsWith(QLatin1String("org.bluez.Error"))) {
        return PendingCall::UnknownError;
    }

#define FROM_BLUEZ_ERROR(string, value)                                                                                                                        \
    if (errorName == QLatin1String(string)) {                                                                                                                  \
        return value;                                                                                                                                          \
    }

    const QString &errorName = name.mid(16);
    FROM_BLUEZ_ERROR("NotReady", PendingCall::NotReady);
    FROM_BLUEZ_ERROR("Failed", PendingCall::Failed);
    FROM_BLUEZ_ERROR("Rejected", PendingCall::Rejected);
    FROM_BLUEZ_ERROR("Canceled", PendingCall::Canceled);
    FROM_BLUEZ_ERROR("InvalidArguments", PendingCall::InvalidArguments);
    FROM_BLUEZ_ERROR("AlreadyExists", PendingCall::AlreadyExists);
    FROM_BLUEZ_ERROR("DoesNotExist", PendingCall::DoesNotExist);
    FROM_BLUEZ_ERROR("AlreadyConnected", PendingCall::AlreadyConnected);
    FROM_BLUEZ_ERROR("ConnectFailed", PendingCall::ConnectFailed);
    FROM_BLUEZ_ERROR("NotConnected", PendingCall::NotConnected);
    FROM_BLUEZ_ERROR("NotSupported", PendingCall::NotSupported);
    FROM_BLUEZ_ERROR("NotAuthorized", PendingCall::NotAuthorized);
    FROM_BLUEZ_ERROR("AuthenticationCanceled", PendingCall::AuthenticationCanceled);
    FROM_BLUEZ_ERROR("AuthenticationFailed", PendingCall::AuthenticationFailed);
    FROM_BLUEZ_ERROR("AuthenticationRejected", PendingCall::AuthenticationRejected);
    FROM_BLUEZ_ERROR("AuthenticationTimeout", PendingCall::AuthenticationTimeout);
    FROM_BLUEZ_ERROR("ConnectionAttemptFailed", PendingCall::ConnectionAttemptFailed);
    FROM_BLUEZ_ERROR("InvalidLength", PendingCall::InvalidLength);
    FROM_BLUEZ_ERROR("NotPermitted", PendingCall::NotPermitted);
#undef FROM_BLUEZ_ERROR

    return PendingCall::UnknownError;
}

class PendingCallPrivate : public QObject
{
public:
    explicit PendingCallPrivate(PendingCall *parent);

    void processReply(QDBusPendingCallWatcher *call);
    void processVoidReply(const QDBusPendingReply<> &reply);
    void processUint32Reply(const QDBusPendingReply<quint32> &reply);
    void processStringReply(const QDBusPendingReply<QString> &reply);
    void processStringListReply(const QDBusPendingReply<QStringList> &reply);
    void processObjectPathReply(const QDBusPendingReply<QDBusObjectPath> &reply);
    void processFileTransferListReply(const QDBusPendingReply<QVariantMapList> &reply);
    void processTransferWithPropertiesReply(const QDBusPendingReply<QDBusObjectPath, QVariantMap> &reply);
    void processByteArrayReply(const QDBusPendingReply<QByteArray> &reply);
    void processError(const QDBusError &m_error);

    void emitFinished();
    void emitDelayedFinished();
    void emitInternalError(const QString &errorText);
    void pendingCallFinished(QDBusPendingCallWatcher *m_watcher);

    PendingCall *q;
    int m_error;
    QString m_errorText;
    QVariant m_userData;
    QVariantList m_value;
    PendingCall::ReturnType m_type;
    QDBusPendingCallWatcher *m_watcher;
};

PendingCallPrivate::PendingCallPrivate(PendingCall *parent)
    : QObject(parent)
    , q(parent)
    , m_error(PendingCall::NoError)
    , m_type(PendingCall::ReturnVoid)
    , m_watcher(nullptr)
{
}

void PendingCallPrivate::processReply(QDBusPendingCallWatcher *call)
{
    switch (m_type) {
    case PendingCall::ReturnVoid:
        processVoidReply(*call);
        break;

    case PendingCall::ReturnUint32:
        processUint32Reply(*call);
        break;

    case PendingCall::ReturnString:
        processStringReply(*call);
        break;

    case PendingCall::ReturnStringList:
        processStringListReply(*call);
        break;

    case PendingCall::ReturnObjectPath:
        processObjectPathReply(*call);
        break;

    case PendingCall::ReturnFileTransferList:
        processFileTransferListReply(*call);
        break;

    case PendingCall::ReturnTransferWithProperties:
        processTransferWithPropertiesReply(*call);
        break;

    case PendingCall::ReturnByteArray:
        processByteArrayReply(*call);
        break;

    default:
        break;
    }
}

void PendingCallPrivate::processVoidReply(const QDBusPendingReply<> &reply)
{
    processError(reply.error());
}

void PendingCallPrivate::processUint32Reply(const QDBusPendingReply<quint32> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        m_value.append(reply.value());
    }
}

void PendingCallPrivate::processStringReply(const QDBusPendingReply<QString> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        m_value.append(reply.value());
    }
}

void PendingCallPrivate::processStringListReply(const QDBusPendingReply<QStringList> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        m_value.append(reply.value());
    }
}

void PendingCallPrivate::processObjectPathReply(const QDBusPendingReply<QDBusObjectPath> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        m_value.append(QVariant::fromValue(reply.value()));
    }
}

void PendingCallPrivate::processFileTransferListReply(const QDBusPendingReply<QVariantMapList> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        QList<ObexFileTransferEntry> items;
        items.reserve(reply.value().size());
        const auto maps = reply.value();
        for (const QVariantMap &map : maps) {
            items.append(ObexFileTransferEntry(map));
        }
        m_value.append(QVariant::fromValue(items));
    }
}

void PendingCallPrivate::processTransferWithPropertiesReply(const QDBusPendingReply<QDBusObjectPath, QVariantMap> &reply)
{
    processError(reply.error());
    if (reply.isError()) {
        return;
    }

    ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(reply.argumentAt<0>().path(), reply.argumentAt<1>()));
    transfer->d->q = transfer.toWeakRef();
    transfer->d->m_suspendable = true;
    m_value.append(QVariant::fromValue(transfer));
}

void PendingCallPrivate::processByteArrayReply(const QDBusPendingReply<QByteArray> &reply)
{
    processError(reply.error());
    if (!reply.isError()) {
        m_value.append(QVariant::fromValue(reply.value()));
    }
}

void PendingCallPrivate::processError(const QDBusError &error)
{
    if (error.isValid()) {
        qCWarning(BLUEZQT) << "PendingCall Error:" << error.message();
        m_error = nameToError(error.name());
        m_errorText = error.message();
    }
}

void PendingCallPrivate::emitFinished()
{
    m_watcher->deleteLater();
    m_watcher = nullptr;
    Q_EMIT q->finished(q);
    q->deleteLater();
}

void PendingCallPrivate::emitDelayedFinished()
{
    Q_ASSERT(qobject_cast<QTimer *>(sender()));

    Q_EMIT q->finished(q);
    static_cast<QTimer *>(sender())->deleteLater();
}

void PendingCallPrivate::emitInternalError(const QString &errorText)
{
    qCWarning(BLUEZQT) << "PendingCall Internal error:" << errorText;
    m_error = PendingCall::InternalError;
    m_errorText = errorText;
    emitFinished();
}

void PendingCallPrivate::pendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    processReply(watcher);
    emitFinished();
}

PendingCall::PendingCall(const QDBusPendingCall &call, ReturnType type, QObject *parent)
    : QObject(parent)
    , d(new PendingCallPrivate(this))
{
    qDBusRegisterMetaType<QVariantMapList>();

    d->m_type = type;
    d->m_watcher = new QDBusPendingCallWatcher(call, this);

    connect(d->m_watcher, &QDBusPendingCallWatcher::finished, d.get(), &PendingCallPrivate::pendingCallFinished);
}

PendingCall::PendingCall(PendingCall::Error error, const QString &errorText, QObject *parent)
    : QObject(parent)
    , d(new PendingCallPrivate(this))
{
    d->m_error = error;
    d->m_errorText = errorText;

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(0);
    connect(timer, &QTimer::timeout, d.get(), &PendingCallPrivate::emitDelayedFinished);
}

PendingCall::PendingCall(const QDBusPendingCall &call, ExternalProcessor externalProcessor, QObject *parent)
    : QObject(parent)
    , d(new PendingCallPrivate(this))
{
    qDBusRegisterMetaType<QVariantMapList>();

    d->m_watcher = new QDBusPendingCallWatcher(call, this);
    connect(d->m_watcher, &QDBusPendingCallWatcher::finished, [externalProcessor, this](QDBusPendingCallWatcher *watcher) {
        externalProcessor(watcher, std::bind(&PendingCallPrivate::processError, d.get(), std::placeholders::_1), &d->m_value);
        d->emitFinished();
    });
}

PendingCall::~PendingCall() = default;

QVariant PendingCall::value() const
{
    if (d->m_value.isEmpty()) {
        return QVariant();
    }
    return d->m_value.first();
}

QVariantList PendingCall::values() const
{
    return d->m_value;
}

int PendingCall::error() const
{
    return d->m_error;
}

QString PendingCall::errorText() const
{
    return d->m_errorText;
}

bool PendingCall::isFinished() const
{
    if (d->m_watcher) {
        return d->m_watcher->isFinished();
    }
    return true;
}

void PendingCall::waitForFinished()
{
    if (d->m_watcher) {
        d->m_watcher->waitForFinished();
    }
}

QVariant PendingCall::userData() const
{
    return d->m_userData;
}

void PendingCall::setUserData(const QVariant &userData)
{
    d->m_userData = userData;
}

} // namespace BluezQt

#include "moc_pendingcall.cpp"
