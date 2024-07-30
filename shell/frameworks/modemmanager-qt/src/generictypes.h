/*
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_GENERIC_TYPES_H
#define MODEMMANAGERQT_GENERIC_TYPES_H

#include <ModemManager/ModemManager.h>
#include <modemmanagerqt_export.h>

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QMetaType>

// Enums from ModemManager-enums.h which needs to be defined all the time,
// because it's not possible to use MM_CHECK_VERSION for signals
#if !MM_CHECK_VERSION(1, 2, 0)
typedef enum { /*< underscore_name=mm_modem_3gpp_subscription_state >*/
               MM_MODEM_3GPP_SUBSCRIPTION_STATE_UNKNOWN = 0,
               MM_MODEM_3GPP_SUBSCRIPTION_STATE_UNPROVISIONED = 1,
               MM_MODEM_3GPP_SUBSCRIPTION_STATE_PROVISIONED = 2,
               MM_MODEM_3GPP_SUBSCRIPTION_STATE_OUT_OF_DATA = 3,
} MMModem3gppSubscriptionState;

typedef enum { /*< underscore_name=mm_sms_cdma_service_category >*/
               MM_SMS_CDMA_SERVICE_CATEGORY_UNKNOWN = 0x0000,
               MM_SMS_CDMA_SERVICE_CATEGORY_EMERGENCY_BROADCAST = 0x0001,
               MM_SMS_CDMA_SERVICE_CATEGORY_ADMINISTRATIVE = 0x0002,
               MM_SMS_CDMA_SERVICE_CATEGORY_MAINTENANCE = 0x0003,
               MM_SMS_CDMA_SERVICE_CATEGORY_GENERAL_NEWS_LOCAL = 0x0004,
               MM_SMS_CDMA_SERVICE_CATEGORY_GENERAL_NEWS_REGIONAL = 0x0005,
               MM_SMS_CDMA_SERVICE_CATEGORY_GENERAL_NEWS_NATIONAL = 0x0006,
               MM_SMS_CDMA_SERVICE_CATEGORY_GENERAL_NEWS_INTERNATIONAL = 0x0007,
               MM_SMS_CDMA_SERVICE_CATEGORY_BUSINESS_NEWS_LOCAL = 0x0008,
               MM_SMS_CDMA_SERVICE_CATEGORY_BUSINESS_NEWS_REGIONAL = 0x0009,
               MM_SMS_CDMA_SERVICE_CATEGORY_BUSINESS_NEWS_NATIONAL = 0x000A,
               MM_SMS_CDMA_SERVICE_CATEGORY_BUSINESS_NEWS_INTERNATIONAL = 0x000B,
               MM_SMS_CDMA_SERVICE_CATEGORY_SPORTS_NEWS_LOCAL = 0x000C,
               MM_SMS_CDMA_SERVICE_CATEGORY_SPORTS_NEWS_REGIONAL = 0x000D,
               MM_SMS_CDMA_SERVICE_CATEGORY_SPORTS_NEWS_NATIONAL = 0x000E,
               MM_SMS_CDMA_SERVICE_CATEGORY_SPORTS_NEWS_INTERNATIONAL = 0x000F,
               MM_SMS_CDMA_SERVICE_CATEGORY_ENTERTAINMENT_NEWS_LOCAL = 0x0010,
               MM_SMS_CDMA_SERVICE_CATEGORY_ENTERTAINMENT_NEWS_REGIONAL = 0x0011,
               MM_SMS_CDMA_SERVICE_CATEGORY_ENTERTAINMENT_NEWS_NATIONAL = 0x0012,
               MM_SMS_CDMA_SERVICE_CATEGORY_ENTERTAINMENT_NEWS_INTERNATIONAL = 0x0013,
               MM_SMS_CDMA_SERVICE_CATEGORY_LOCAL_WEATHER = 0x0014,
               MM_SMS_CDMA_SERVICE_CATEGORY_TRAFFIC_REPORT = 0x0015,
               MM_SMS_CDMA_SERVICE_CATEGORY_FLIGHT_SCHEDULES = 0x0016,
               MM_SMS_CDMA_SERVICE_CATEGORY_RESTAURANTS = 0x0017,
               MM_SMS_CDMA_SERVICE_CATEGORY_LODGINGS = 0x0018,
               MM_SMS_CDMA_SERVICE_CATEGORY_RETAIL_DIRECTORY = 0x0019,
               MM_SMS_CDMA_SERVICE_CATEGORY_ADVERTISEMENTS = 0x001A,
               MM_SMS_CDMA_SERVICE_CATEGORY_STOCK_QUOTES = 0x001B,
               MM_SMS_CDMA_SERVICE_CATEGORY_EMPLOYMENT = 0x001C,
               MM_SMS_CDMA_SERVICE_CATEGORY_HOSPITALS = 0x001D,
               MM_SMS_CDMA_SERVICE_CATEGORY_TECHNOLOGY_NEWS = 0x001E,
               MM_SMS_CDMA_SERVICE_CATEGORY_MULTICATEGORY = 0x001F,
               MM_SMS_CDMA_SERVICE_CATEGORY_CMAS_PRESIDENTIAL_ALERT = 0x1000,
               MM_SMS_CDMA_SERVICE_CATEGORY_CMAS_EXTREME_THREAT = 0x1001,
               MM_SMS_CDMA_SERVICE_CATEGORY_CMAS_SEVERE_THREAT = 0x1002,
               MM_SMS_CDMA_SERVICE_CATEGORY_CMAS_CHILD_ABDUCTION_EMERGENCY = 0x1003,
               MM_SMS_CDMA_SERVICE_CATEGORY_CMAS_TEST = 0x1004,
} MMSmsCdmaServiceCategory;

typedef enum { /*< underscore_name=mm_sms_cdma_teleservice_id >*/
               MM_SMS_CDMA_TELESERVICE_ID_UNKNOWN = 0x0000,
               MM_SMS_CDMA_TELESERVICE_ID_CMT91 = 0x1000,
               MM_SMS_CDMA_TELESERVICE_ID_WPT = 0x1001,
               MM_SMS_CDMA_TELESERVICE_ID_WMT = 0x1002,
               MM_SMS_CDMA_TELESERVICE_ID_VMN = 0x1003,
               MM_SMS_CDMA_TELESERVICE_ID_WAP = 0x1004,
               MM_SMS_CDMA_TELESERVICE_ID_WEMT = 0x1005,
               MM_SMS_CDMA_TELESERVICE_ID_SCPT = 0x1006,
               MM_SMS_CDMA_TELESERVICE_ID_CATPT = 0x1007,
} MMSmsCdmaTeleserviceId;
#endif

namespace ModemManager
{
typedef QMap<QString, QVariantMap> MMVariantMapMap;
typedef QMap<QDBusObjectPath, MMVariantMapMap> DBUSManagerStruct;
typedef QList<uint> UIntList;
typedef QList<QList<uint>> UIntListList;

typedef struct {
    QString name;
    MMModemPortType type;
} Port;
typedef QList<Port> PortList;

typedef struct {
    uint signal;
    bool recent;
} SignalQualityPair;

typedef struct {
    uint allowed; // bitfield
    MMModemMode preferred;
} CurrentModesType;
typedef QList<CurrentModesType> SupportedModesType;

#if MM_CHECK_VERSION(1, 2, 0)
typedef struct {
    MMOmaSessionType type;
    uint id;
} OmaSessionType;
typedef QList<OmaSessionType> OmaSessionTypes;
#endif
typedef QMap<MMModemLock, uint> UnlockRetriesMap;
typedef QList<QVariantMap> QVariantMapList;
typedef QMap<MMModemLocationSource, QVariant> LocationInformationMap;
typedef struct {
    MMSmsValidityType validity;
    uint value;
} ValidityPair;
}

// Flags
Q_DECLARE_METATYPE(QFlags<MMModemAccessTechnology>)
Q_DECLARE_METATYPE(QFlags<MMModemCapability>)
Q_DECLARE_METATYPE(QFlags<MMModemMode>)
Q_DECLARE_METATYPE(QFlags<MMBearerIpFamily>)
Q_DECLARE_METATYPE(QFlags<MMModem3gppFacility>)
Q_DECLARE_METATYPE(QFlags<MMModemLocationSource>)
#if MM_CHECK_VERSION(1, 2, 0)
Q_DECLARE_METATYPE(QFlags<MMOmaFeature>)
#endif

// ModemManager enums
Q_DECLARE_METATYPE(QList<MMModemBand>)
Q_DECLARE_METATYPE(QList<MMModemCapability>)
Q_DECLARE_METATYPE(MMModemLock)
Q_DECLARE_METATYPE(MMModemPowerState)
Q_DECLARE_METATYPE(MMModemState)
Q_DECLARE_METATYPE(MMModemStateChangeReason)
Q_DECLARE_METATYPE(MMModemStateFailedReason)
Q_DECLARE_METATYPE(MMModem3gppRegistrationState)
#if MM_CHECK_VERSION(1, 2, 0)
Q_DECLARE_METATYPE(MMModem3gppSubscriptionState)
#endif
Q_DECLARE_METATYPE(MMModem3gppUssdSessionState)
Q_DECLARE_METATYPE(MMModemCdmaActivationState)
Q_DECLARE_METATYPE(MMCdmaActivationError)
Q_DECLARE_METATYPE(MMModemCdmaRegistrationState)
Q_DECLARE_METATYPE(MMModemLocationSource)
#if MM_CHECK_VERSION(1, 2, 0)
Q_DECLARE_METATYPE(MMOmaSessionType)
Q_DECLARE_METATYPE(MMOmaSessionState)
Q_DECLARE_METATYPE(MMOmaSessionStateFailedReason)
#endif
Q_DECLARE_METATYPE(MMSmsDeliveryState)
Q_DECLARE_METATYPE(MMSmsPduType)
Q_DECLARE_METATYPE(MMSmsState)
Q_DECLARE_METATYPE(MMSmsStorage)
#if MM_CHECK_VERSION(1, 2, 0)
Q_DECLARE_METATYPE(MMSmsCdmaServiceCategory)
Q_DECLARE_METATYPE(MMSmsCdmaTeleserviceId)
#endif
#if MM_CHECK_VERSION(1, 6, 0)
Q_DECLARE_METATYPE(MMCallState)
Q_DECLARE_METATYPE(MMCallStateReason)
Q_DECLARE_METATYPE(MMCallDirection)
#endif
#if MM_CHECK_VERSION(1, 20, 0)
Q_DECLARE_METATYPE(MMSimType)
Q_DECLARE_METATYPE(MMSimEsimStatus)
Q_DECLARE_METATYPE(MMSimRemovability)
#endif

// Others
Q_DECLARE_METATYPE(ModemManager::DBUSManagerStruct)
Q_DECLARE_METATYPE(ModemManager::Port)
Q_DECLARE_METATYPE(ModemManager::PortList)
Q_DECLARE_METATYPE(ModemManager::SignalQualityPair)
Q_DECLARE_METATYPE(ModemManager::CurrentModesType)
Q_DECLARE_METATYPE(ModemManager::SupportedModesType)
#if MM_CHECK_VERSION(1, 2, 0)
Q_DECLARE_METATYPE(ModemManager::OmaSessionType)
Q_DECLARE_METATYPE(ModemManager::OmaSessionTypes)
#endif
Q_DECLARE_METATYPE(ModemManager::UnlockRetriesMap)
Q_DECLARE_METATYPE(ModemManager::LocationInformationMap)
Q_DECLARE_METATYPE(ModemManager::ValidityPair)

// Port
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &arg, const ModemManager::Port &port);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &arg, ModemManager::Port &port);

// CurrentModesType
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &arg, const ModemManager::CurrentModesType &mode);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &arg, ModemManager::CurrentModesType &mode);

// SignalQualityPair
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &arg, const ModemManager::SignalQualityPair &sqp);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &arg, ModemManager::SignalQualityPair &sqp);

// ValidityPair
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &arg, const ModemManager::ValidityPair &vp);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &arg, ModemManager::ValidityPair &vp);

#if MM_CHECK_VERSION(1, 2, 0)
// OmaSessionType
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &arg, const ModemManager::OmaSessionType &sessionType);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &arg, ModemManager::OmaSessionType &sessionType);
#endif

// UnlockRetriesMap
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const ModemManager::UnlockRetriesMap &unlockRetriesMap);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, ModemManager::UnlockRetriesMap &unlockRetriesMap);

// QVariantMapList
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const ModemManager::QVariantMapList &variantMapList);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, ModemManager::QVariantMapList &variantMapList);

// LocationInformationMap
MODEMMANAGERQT_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const ModemManager::LocationInformationMap &locationMap);
MODEMMANAGERQT_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, ModemManager::LocationInformationMap &locationMap);

MODEMMANAGERQT_EXPORT void registerModemManagerTypes();

#endif // MODEMMANAGERQT_GENERIC_TYPES_P_H
