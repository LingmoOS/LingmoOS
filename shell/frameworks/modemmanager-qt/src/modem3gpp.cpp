/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gpp.h"
#include "mmdebug_p.h"
#include "modem3gpp_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::Modem3gppPrivate::Modem3gppPrivate(const QString &path, Modem3gpp *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modem3gppIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modem3gppIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (modem3gppIface.isValid()) {
        imei = modem3gppIface.imei();
        registrationState = (MMModem3gppRegistrationState)modem3gppIface.registrationState();
        operatorCode = modem3gppIface.operatorCode();
        operatorName = modem3gppIface.operatorName();
        enabledFacilityLocks = (QFlags<MMModem3gppFacility>)modem3gppIface.enabledFacilityLocks();
#if MM_CHECK_VERSION(1, 2, 0)
        subscriptionState = (MMModem3gppSubscriptionState)modem3gppIface.subscriptionState();
#endif

        QStringView mcc(operatorCode);
        if (!operatorCode.isEmpty() && operatorCode.size() > 3) {
            mcc = mcc.sliced(0, 3);
        }
        QString cc = mobileCountryCodeToAlpha2CountryCode(mcc.toInt());
        if (cc != countryCode) {
            countryCode = cc;
        }
    }
}

ModemManager::Modem3gpp::Modem3gpp(const QString &path, QObject *parent)
    : Interface(*new Modem3gppPrivate(path, this), parent)
{
    Q_D(Modem3gpp);

    qRegisterMetaType<QFlags<MMModem3gppFacility>>();
    qRegisterMetaType<MMModem3gppRegistrationState>();
#if MM_CHECK_VERSION(1, 2, 0)
    qRegisterMetaType<MMModem3gppSubscriptionState>();
#endif
#ifdef MMQT_STATIC
    QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                          d->uni,
                                          QLatin1String(DBUS_INTERFACE_PROPS),
                                          QStringLiteral("PropertiesChanged"),
                                          d,
                                          SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#else
    QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                         d->uni,
                                         QLatin1String(DBUS_INTERFACE_PROPS),
                                         QStringLiteral("PropertiesChanged"),
                                         d,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#endif
}

ModemManager::Modem3gpp::~Modem3gpp()
{
}

QString ModemManager::Modem3gpp::imei() const
{
    Q_D(const Modem3gpp);
    return d->imei;
}

MMModem3gppRegistrationState ModemManager::Modem3gpp::registrationState() const
{
    Q_D(const Modem3gpp);
    return d->registrationState;
}

QString ModemManager::Modem3gpp::operatorCode() const
{
    Q_D(const Modem3gpp);
    return d->operatorCode;
}

QString ModemManager::Modem3gpp::operatorName() const
{
    Q_D(const Modem3gpp);
    return d->operatorName;
}

QString ModemManager::Modem3gpp::countryCode() const
{
    Q_D(const Modem3gpp);
    return d->countryCode;
}

ModemManager::Modem3gpp::FacilityLocks ModemManager::Modem3gpp::enabledFacilityLocks() const
{
    Q_D(const Modem3gpp);
    return d->enabledFacilityLocks;
}

#if MM_CHECK_VERSION(1, 2, 0)
MMModem3gppSubscriptionState ModemManager::Modem3gpp::subscriptionState() const
{
    Q_D(const Modem3gpp);
    return d->subscriptionState;
}
#endif

void ModemManager::Modem3gpp::registerToNetwork(const QString &networkId)
{
    Q_D(Modem3gpp);
    d->modem3gppIface.Register(networkId);
}

QDBusPendingReply<ModemManager::QVariantMapList> ModemManager::Modem3gpp::scan()
{
    Q_D(Modem3gpp);
    return d->modem3gppIface.Scan();
}

void ModemManager::Modem3gpp::setTimeout(int timeout)
{
    Q_D(Modem3gpp);
    d->modem3gppIface.setTimeout(timeout);
}

int ModemManager::Modem3gpp::timeout() const
{
    Q_D(const Modem3gpp);
    return d->modem3gppIface.timeout();
}

void ModemManager::Modem3gppPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_Q(Modem3gpp);
    Q_UNUSED(invalidatedProps);
    qCDebug(MMQT) << interface << properties.keys();

    if (interface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP)) {
        QVariantMap::const_iterator it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_IMEI));
        if (it != properties.constEnd()) {
            imei = it->toString();
            Q_EMIT q->imeiChanged(imei);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_REGISTRATIONSTATE));
        if (it != properties.constEnd()) {
            registrationState = (MMModem3gppRegistrationState)it->toUInt();
            Q_EMIT q->registrationStateChanged(registrationState);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_OPERATORCODE));
        if (it != properties.constEnd()) {
            operatorCode = it->toString();
            Q_EMIT q->operatorCodeChanged(operatorCode);
            QStringView mcc(operatorCode);
            if (!operatorCode.isEmpty() && operatorCode.size() > 3) {
                mcc = mcc.sliced(0, 3);
            }
            QString cc = mobileCountryCodeToAlpha2CountryCode(mcc.toInt());
            if (cc != countryCode) {
                countryCode = cc;
                Q_EMIT q->countryCodeChanged(countryCode);
            }
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_OPERATORNAME));
        if (it != properties.constEnd()) {
            operatorName = it->toString();
            Q_EMIT q->operatorNameChanged(operatorName);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_ENABLEDFACILITYLOCKS));
        if (it != properties.constEnd()) {
            enabledFacilityLocks = (QFlags<MMModem3gppFacility>)it->toUInt();
            Q_EMIT q->enabledFacilityLocksChanged(enabledFacilityLocks);
        }
#if MM_CHECK_VERSION(1, 2, 0)
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_SUBSCRIPTIONSTATE));
        if (it != properties.constEnd()) {
            subscriptionState = (MMModem3gppSubscriptionState)it->toUInt();
            Q_EMIT q->subscriptionStateChanged(subscriptionState);
        }
#endif
    }
}

// The logic ported from the Qt wrapper for ofono from Jolla's Sailfish OS
// https://github.com/sailfishos/libqofono/blob/94e793860debe9c73c70de94cc3510e7609137b9/src/qofono.cpp#L2379

QString ModemManager::Modem3gppPrivate::mobileCountryCodeToAlpha2CountryCode(int mcc) const
{
    const int n = sizeof(mccList) / sizeof(mccList[0]);

    int low = 0;
    int high = n;

    while (low < high) {
        const int mid = (low + high) / 2;
        if (mccList[mid].mcc >= mcc) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    if (high < n && mccList[high].mcc == mcc) {
        return QString::fromLatin1(mccList[high].cc);
    } else {
        qCWarning(MMQT) << "Unknown Mobile Country Code:" << mcc;
        return QString();
    }
}

#include "moc_modem3gpp.cpp"
