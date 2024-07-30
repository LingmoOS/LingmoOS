/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemvoice.h"
#include "modemvoice_p.h"

#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif
#include "mmdebug_p.h"

#include <ModemManager/ModemManager.h>

ModemManager::ModemVoicePrivate::ModemVoicePrivate(const QString &path, ModemVoice *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemVoiceIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemVoiceIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (modemVoiceIface.isValid()) {
        connect(&modemVoiceIface, &OrgFreedesktopModemManager1ModemVoiceInterface::CallAdded, this, &ModemVoicePrivate::onCallAdded);
        connect(&modemVoiceIface, &OrgFreedesktopModemManager1ModemVoiceInterface::CallDeleted, this, &ModemVoicePrivate::onCallDeleted);
    }
}

ModemManager::ModemVoice::ModemVoice(const QString &path, QObject *parent)
    : Interface(*new ModemVoicePrivate(path, this), parent)
{
    Q_D(ModemVoice);

    // Note: no need to listen for PropertiesChanged signals. Additions and
    // removals of calls are already notified via CallAdded and CallRemoved

    QList<QDBusObjectPath> calls = d->modemVoiceIface.calls();
    Q_FOREACH (const QDBusObjectPath &op, calls) {
        const QString path = op.path();
        d->callList.insert(path, ModemManager::Call::Ptr());
        Q_EMIT callAdded(path);
    }
}

ModemManager::ModemVoice::~ModemVoice()
{
}

ModemManager::Call::Ptr ModemManager::ModemVoicePrivate::findCall(const QString &uni)
{
    ModemManager::Call::Ptr call;
    if (callList.contains(uni)) {
        if (callList.value(uni)) {
            call = callList.value(uni);
        } else {
            call = ModemManager::Call::Ptr(new ModemManager::Call(uni), &QObject::deleteLater);
            callList[uni] = call;
        }
    }
    return call;
}

ModemManager::Call::List ModemManager::ModemVoicePrivate::ModemVoicePrivate::calls()
{
    ModemManager::Call::List list;

    QMap<QString, ModemManager::Call::Ptr>::const_iterator i;
    for (i = callList.constBegin(); i != callList.constEnd(); ++i) {
        ModemManager::Call::Ptr call = findCall(i.key());
        if (call) {
            list.append(call);
        } else {
            qCWarning(MMQT) << "warning: null call for" << i.key();
        }
    }

    return list;
}

void ModemManager::ModemVoicePrivate::onCallAdded(const QDBusObjectPath &path)
{
    Q_Q(ModemVoice);
    callList.insert(path.path(), ModemManager::Call::Ptr());
    Q_EMIT q->callAdded(path.path());
}

void ModemManager::ModemVoicePrivate::onCallDeleted(const QDBusObjectPath &path)
{
    Q_Q(ModemVoice);
    callList.remove(path.path());
    Q_EMIT q->callDeleted(path.path());
}

ModemManager::Call::List ModemManager::ModemVoice::calls() const
{
    Q_D(const ModemVoice);
    return const_cast<ModemVoicePrivate *>(d)->calls();
}

QDBusPendingReply<QDBusObjectPath> ModemManager::ModemVoice::createCall(const QString &number)
{
    QVariantMap map;
    map.insert(QStringLiteral("number"), number);
    return createCall(map);
}

QDBusPendingReply<QDBusObjectPath> ModemManager::ModemVoice::createCall(const QVariantMap &call)
{
    Q_D(ModemVoice);

    if (!call.contains(QLatin1String("number"))) {
        qCDebug(MMQT) << "Unable to create call, missing some property";
        return QDBusPendingReply<QDBusObjectPath>();
    }

    return d->modemVoiceIface.CreateCall(call);
}

QDBusPendingReply<void> ModemManager::ModemVoice::deleteCall(const QString &uni)
{
    Q_D(ModemVoice);
    return d->modemVoiceIface.DeleteCall(QDBusObjectPath(uni));
}

ModemManager::Call::Ptr ModemManager::ModemVoice::findCall(const QString &uni)
{
    Q_D(ModemVoice);
    return d->findCall(uni);
}

void ModemManager::ModemVoice::setTimeout(int timeout)
{
    Q_D(ModemVoice);
    d->modemVoiceIface.setTimeout(timeout);
}

int ModemManager::ModemVoice::timeout() const
{
    Q_D(const ModemVoice);
    return d->modemVoiceIface.timeout();
}

#include "moc_modemvoice.cpp"
