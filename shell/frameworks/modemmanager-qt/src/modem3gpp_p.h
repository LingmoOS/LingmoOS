/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_MODEM3GPP_P_H
#define MODEMMANAGER_MODEM3GPP_P_H

#include "dbus/modem3gppinterface.h"
#include "interface_p.h"
#include "modem3gpp.h"

namespace ModemManager
{
class Modem3gppPrivate : public InterfacePrivate
{
public:
    explicit Modem3gppPrivate(const QString &path, Modem3gpp *q);
    OrgFreedesktopModemManager1ModemModem3gppInterface modem3gppIface;

    QString imei;
    MMModem3gppRegistrationState registrationState;
    QString operatorCode;
    QString operatorName;
    QString countryCode;
    QFlags<MMModem3gppFacility> enabledFacilityLocks;
#if MM_CHECK_VERSION(1, 2, 0)
    MMModem3gppSubscriptionState subscriptionState;
#endif

    Q_DECLARE_PUBLIC(Modem3gpp)
    Modem3gpp *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;

private:
    QString mobileCountryCodeToAlpha2CountryCode(int mcc) const;
};

// The logic ported from the Qt wrapper for ofono from Jolla's Sailfish OS:
// https://github.com/sailfishos/libqofono

// For MCC to country code mapping crawling, see aslo:
// https://github.com/sailfishos/tzdata-timed/blob/master/scripts/create-mcc-country-mappings.sh

typedef struct _MccListEntry {
    int mcc;
    char mnc[4];
    char cc[4];
} MccListEntry;

// Updated from Wikipedia on June 15, 2017
// http://en.wikipedia.org/wiki/Mobile_country_code
//
// The list can also be produced from tzdata/data/MCC file,
// like this:
//
// sed -e 's/$/ *\//'
//     -e 's/ /"\}, \/* /3'
//     -e 's/ /", "/2'
//     -e 's/ /, "/1'
//     -e 's/^/    {/'
//
static const MccListEntry mccList[] = {
    {202, "01", "GR"}, /* Greece */
    {202, "02", "GR"}, /* Greece */
    {202, "03", "GR"}, /* Greece */
    {202, "04", "GR"}, /* Greece */
    {202, "05", "GR"}, /* Greece */
    {202, "06", "GR"}, /* Greece */
    {202, "07", "GR"}, /* Greece */
    {202, "09", "GR"}, /* Greece */
    {202, "10", "GR"}, /* Greece */
    {202, "11", "GR"}, /* Greece */
    {202, "12", "GR"}, /* Greece */
    {202, "13", "GR"}, /* Greece */
    {202, "15", "GR"}, /* Greece */
    {202, "16", "GR"}, /* Greece */
    {204, "01", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "02", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "03", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "04", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "05", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "06", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "07", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "08", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "09", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "10", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "11", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "12", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "13", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "14", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "15", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "16", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "17", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "18", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "19", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "20", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "21", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "22", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "23", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "24", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "25", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "26", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "27", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "28", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "29", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "60", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "61", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "62", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "64", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "65", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "66", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "67", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "68", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {204, "69", "NL"}, /* Netherlands (Kingdom of the Netherlands) */
    {206, "00", "BE"}, /* Belgium */
    {206, "01", "BE"}, /* Belgium */
    {206, "02", "BE"}, /* Belgium */
    {206, "05", "BE"}, /* Belgium */
    {206, "06", "BE"}, /* Belgium */
    {206, "07", "BE"}, /* Belgium */
    {206, "09", "BE"}, /* Belgium */
    {206, "10", "BE"}, /* Belgium */
    {206, "15", "BE"}, /* Belgium */
    {206, "16", "BE"}, /* Belgium */
    {206, "20", "BE"}, /* Belgium */
    {206, "25", "BE"}, /* Belgium */
    {206, "40", "BE"}, /* Belgium */
    {208, "01", "FR"}, /* France */
    {208, "02", "FR"}, /* France */
    {208, "03", "FR"}, /* France */
    {208, "04", "FR"}, /* France */
    {208, "05", "FR"}, /* France */
    {208, "06", "FR"}, /* France */
    {208, "07", "FR"}, /* France */
    {208, "08", "FR"}, /* France */
    {208, "09", "FR"}, /* France */
    {208, "10", "FR"}, /* France */
    {208, "11", "FR"}, /* France */
    {208, "12", "FR"}, /* France */
    {208, "13", "FR"}, /* France */
    {208, "14", "FR"}, /* France */
    {208, "15", "FR"}, /* France */
    {208, "16", "FR"}, /* France */
    {208, "17", "FR"}, /* France */
    {208, "18", "FR"}, /* France */
    {208, "19", "FR"}, /* France */
    {208, "20", "FR"}, /* France */
    {208, "21", "FR"}, /* France */
    {208, "22", "FR"}, /* France */
    {208, "23", "FR"}, /* France */
    {208, "24", "FR"}, /* France */
    {208, "25", "FR"}, /* France */
    {208, "26", "FR"}, /* France */
    {208, "27", "FR"}, /* France */
    {208, "28", "FR"}, /* France */
    {208, "29", "FR"}, /* France */
    {208, "30", "FR"}, /* France */
    {208, "31", "FR"}, /* France */
    {208, "88", "FR"}, /* France */
    {208, "89", "FR"}, /* France */
    {208, "90", "FR"}, /* France */
    {208, "91", "FR"}, /* France */
    {208, "92", "FR"}, /* France */
    {208, "93", "FR"}, /* France */
    {208, "94", "FR"}, /* France */
    {208, "95", "FR"}, /* France */
    {208, "96", "FR"}, /* France */
    {208, "97", "FR"}, /* France */
    {208, "98", "FR"}, /* France */
    {212, "10", "MC"}, /* Monaco */
    {214, "01", "ES"}, /* Spain */
    {214, "03", "ES"}, /* Spain */
    {214, "04", "ES"}, /* Spain */
    {214, "05", "ES"}, /* Spain */
    {214, "06", "ES"}, /* Spain */
    {214, "07", "ES"}, /* Spain */
    {214, "08", "ES"}, /* Spain */
    {214, "09", "ES"}, /* Spain */
    {214, "10", "ES"}, /* Spain */
    {214, "11", "ES"}, /* Spain */
    {214, "12", "ES"}, /* Spain */
    {214, "13", "ES"}, /* Spain */
    {214, "14", "ES"}, /* Spain */
    {214, "15", "ES"}, /* Spain */
    {214, "16", "ES"}, /* Spain */
    {214, "17", "ES"}, /* Spain */
    {214, "18", "ES"}, /* Spain */
    {214, "19", "ES"}, /* Spain */
    {214, "20", "ES"}, /* Spain */
    {214, "21", "ES"}, /* Spain */
    {214, "22", "ES"}, /* Spain */
    {214, "23", "ES"}, /* Spain */
    {214, "24", "ES"}, /* Spain */
    {214, "25", "ES"}, /* Spain */
    {214, "26", "ES"}, /* Spain */
    {214, "27", "ES"}, /* Spain */
    {214, "28", "ES"}, /* Spain */
    {214, "29", "ES"}, /* Spain */
    {214, "30", "ES"}, /* Spain */
    {214, "31", "ES"}, /* Spain */
    {214, "32", "ES"}, /* Spain */
    {214, "33", "ES"}, /* Spain */
    {214, "34", "ES"}, /* Spain */
    {214, "35", "ES"}, /* Spain */
    {214, "36", "ES"}, /* Spain */
    {216, "01", "HU"}, /* Hungary */
    {216, "02", "HU"}, /* Hungary */
    {216, "03", "HU"}, /* Hungary */
    {216, "04", "HU"}, /* Hungary */
    {216, "30", "HU"}, /* Hungary */
    {216, "70", "HU"}, /* Hungary */
    {216, "71", "HU"}, /* Hungary */
    {216, "99", "HU"}, /* Hungary */
    {218, "03", "BA"}, /* Bosnia and Herzegovina */
    {218, "05", "BA"}, /* Bosnia and Herzegovina */
    {218, "90", "BA"}, /* Bosnia and Herzegovina */
    {219, "01", "HR"}, /* Croatia */
    {219, "02", "HR"}, /* Croatia */
    {219, "10", "HR"}, /* Croatia */
    {219, "12", "HR"}, /* Croatia */
    {220, "01", "RS"}, /* Serbia */
    {220, "02", "RS"}, /* Serbia */
    {220, "03", "RS"}, /* Serbia */
    {220, "04", "RS"}, /* Serbia */
    {220, "05", "RS"}, /* Serbia */
    {220, "07", "RS"}, /* Serbia */
    {220, "09", "RS"}, /* Serbia */
    {220, "11", "RS"}, /* Serbia */
    {221, "01", "XK"}, /* Kosovo */
    {221, "02", "XK"}, /* Kosovo */
    {221, "06", "XK"}, /* Kosovo */
    {222, "01", "IT"}, /* Italy */
    {222, "02", "IT"}, /* Italy */
    {222, "04", "IT"}, /* Italy */
    {222, "05", "IT"}, /* Italy */
    {222, "06", "IT"}, /* Italy */
    {222, "07", "IT"}, /* Italy */
    {222, "08", "IT"}, /* Italy */
    {222, "10", "IT"}, /* Italy */
    {222, "30", "IT"}, /* Italy */
    {222, "33", "IT"}, /* Italy */
    {222, "34", "IT"}, /* Italy */
    {222, "35", "IT"}, /* Italy */
    {222, "36", "IT"}, /* Italy */
    {222, "37", "IT"}, /* Italy */
    {222, "38", "IT"}, /* Italy */
    {222, "39", "IT"}, /* Italy */
    {222, "43", "IT"}, /* Italy */
    {222, "48", "IT"}, /* Italy */
    {222, "50", "IT"}, /* Italy */
    {222, "77", "IT"}, /* Italy */
    {222, "88", "IT"}, /* Italy */
    {222, "98", "IT"}, /* Italy */
    {222, "99", "IT"}, /* Italy */
    {226, "01", "RO"}, /* Romania */
    {226, "02", "RO"}, /* Romania */
    {226, "03", "RO"}, /* Romania */
    {226, "04", "RO"}, /* Romania */
    {226, "05", "RO"}, /* Romania */
    {226, "06", "RO"}, /* Romania */
    {226, "10", "RO"}, /* Romania */
    {226, "11", "RO"}, /* Romania */
    {226, "15", "RO"}, /* Romania */
    {226, "16", "RO"}, /* Romania */
    {228, "01", "CH"}, /* Switzerland */
    {228, "02", "CH"}, /* Switzerland */
    {228, "03", "CH"}, /* Switzerland */
    {228, "05", "CH"}, /* Switzerland */
    {228, "06", "CH"}, /* Switzerland */
    {228, "07", "CH"}, /* Switzerland */
    {228, "08", "CH"}, /* Switzerland */
    {228, "09", "CH"}, /* Switzerland */
    {228, "12", "CH"}, /* Switzerland */
    {228, "50", "CH"}, /* Switzerland */
    {228, "51", "CH"}, /* Switzerland */
    {228, "52", "CH"}, /* Switzerland */
    {228, "53", "CH"}, /* Switzerland */
    {228, "54", "CH"}, /* Switzerland */
    {228, "55", "CH"}, /* Switzerland */
    {228, "56", "CH"}, /* Switzerland */
    {228, "57", "CH"}, /* Switzerland */
    {228, "58", "CH"}, /* Switzerland */
    {228, "59", "CH"}, /* Switzerland */
    {228, "60", "CH"}, /* Switzerland */
    {228, "61", "CH"}, /* Switzerland */
    {228, "99", "CH"}, /* Switzerland */
    {230, "01", "CZ"}, /* Czech Republic */
    {230, "02", "CZ"}, /* Czech Republic */
    {230, "03", "CZ"}, /* Czech Republic */
    {230, "04", "CZ"}, /* Czech Republic */
    {230, "05", "CZ"}, /* Czech Republic */
    {230, "06", "CZ"}, /* Czech Republic */
    {230, "07", "CZ"}, /* Czech Republic */
    {230, "08", "CZ"}, /* Czech Republic */
    {230, "09", "CZ"}, /* Czech Republic */
    {230, "98", "CZ"}, /* Czech Republic */
    {230, "99", "CZ"}, /* Czech Republic */
    {231, "01", "SK"}, /* Slovakia */
    {231, "02", "SK"}, /* Slovakia */
    {231, "03", "SK"}, /* Slovakia */
    {231, "04", "SK"}, /* Slovakia */
    {231, "05", "SK"}, /* Slovakia */
    {231, "06", "SK"}, /* Slovakia */
    {231, "99", "SK"}, /* Slovakia */
    {232, "02", "AT"}, /* Austria */
    {232, "03", "AT"}, /* Austria */
    {232, "04", "AT"}, /* Austria */
    {232, "05", "AT"}, /* Austria */
    {232, "06", "AT"}, /* Austria */
    {232, "07", "AT"}, /* Austria */
    {232, "08", "AT"}, /* Austria */
    {232, "09", "AT"}, /* Austria */
    {232, "10", "AT"}, /* Austria */
    {232, "11", "AT"}, /* Austria */
    {232, "12", "AT"}, /* Austria */
    {232, "13", "AT"}, /* Austria */
    {232, "14", "AT"}, /* Austria */
    {232, "15", "AT"}, /* Austria */
    {232, "16", "AT"}, /* Austria */
    {232, "17", "AT"}, /* Austria */
    {232, "18", "AT"}, /* Austria */
    {232, "19", "AT"}, /* Austria */
    {232, "20", "AT"}, /* Austria */
    {232, "21", "AT"}, /* Austria */
    {232, "22", "AT"}, /* Austria */
    {232, "91", "AT"}, /* Austria */
    {232, "92", "AT"}, /* Austria */
    {234, "00", "GB"}, /* United Kingdom */
    {234, "01", "GB"}, /* United Kingdom */
    {234, "02", "GB"}, /* United Kingdom */
    {234, "03", "GB"}, /* United Kingdom */
    {234, "03", "GG"}, /* Guernsey (United Kingdom) */
    {234, "03", "JE"}, /* Jersey (United Kingdom) */
    {234, "04", "GB"}, /* United Kingdom */
    {234, "05", "GB"}, /* United Kingdom */
    {234, "06", "GB"}, /* United Kingdom */
    {234, "07", "GB"}, /* United Kingdom */
    {234, "08", "GB"}, /* United Kingdom */
    {234, "09", "GB"}, /* United Kingdom */
    {234, "10", "GB"}, /* United Kingdom */
    {234, "11", "GB"}, /* United Kingdom */
    {234, "12", "GB"}, /* United Kingdom */
    {234, "13", "GB"}, /* United Kingdom */
    {234, "14", "GB"}, /* United Kingdom */
    {234, "15", "GB"}, /* United Kingdom */
    {234, "16", "GB"}, /* United Kingdom */
    {234, "17", "GB"}, /* United Kingdom */
    {234, "18", "GB"}, /* United Kingdom */
    {234, "18", "IM"}, /* Isle of Man (United Kingdom) */
    {234, "19", "GB"}, /* United Kingdom */
    {234, "20", "GB"}, /* United Kingdom */
    {234, "21", "GB"}, /* United Kingdom */
    {234, "22", "GB"}, /* United Kingdom */
    {234, "23", "GB"}, /* United Kingdom */
    {234, "24", "GB"}, /* United Kingdom */
    {234, "25", "GB"}, /* United Kingdom */
    {234, "26", "GB"}, /* United Kingdom */
    {234, "27", "GB"}, /* United Kingdom */
    {234, "28", "GB"}, /* United Kingdom */
    {234, "28", "JE"}, /* Jersey (United Kingdom) */
    {234, "29", "GB"}, /* United Kingdom */
    {234, "31", "GB"}, /* United Kingdom */
    {234, "32", "GB"}, /* United Kingdom */
    {234, "33", "GB"}, /* United Kingdom */
    {234, "34", "GB"}, /* United Kingdom */
    {234, "35", "GB"}, /* United Kingdom */
    {234, "36", "GB"}, /* United Kingdom */
    {234, "36", "IM"}, /* Isle of Man (United Kingdom) */
    {234, "37", "GB"}, /* United Kingdom */
    {234, "38", "GB"}, /* United Kingdom */
    {234, "39", "GB"}, /* United Kingdom */
    {234, "50", "GB"}, /* United Kingdom */
    {234, "50", "GG"}, /* Guernsey (United Kingdom) */
    {234, "50", "JE"}, /* Jersey (United Kingdom) */
    {234, "51", "GB"}, /* United Kingdom */
    {234, "52", "GB"}, /* United Kingdom */
    {234, "53", "GB"}, /* United Kingdom */
    {234, "54", "GB"}, /* United Kingdom */
    {234, "55", "GB"}, /* United Kingdom */
    {234, "55", "GG"}, /* Guernsey (United Kingdom) */
    {234, "55", "JE"}, /* Jersey (United Kingdom) */
    {234, "56", "GB"}, /* United Kingdom */
    {234, "57", "GB"}, /* United Kingdom */
    {234, "58", "GB"}, /* United Kingdom */
    {234, "58", "IM"}, /* Isle of Man (United Kingdom) */
    {234, "59", "GB"}, /* United Kingdom */
    {234, "70", "GB"}, /* United Kingdom */
    {234, "71", "GB"}, /* United Kingdom */
    {234, "72", "GB"}, /* United Kingdom */
    {234, "76", "GB"}, /* United Kingdom */
    {234, "78", "GB"}, /* United Kingdom */
    {234, "86", "GB"}, /* United Kingdom */
    {235, "00", "GB"}, /* United Kingdom */
    {235, "01", "GB"}, /* United Kingdom */
    {235, "02", "GB"}, /* United Kingdom */
    {235, "03", "GB"}, /* United Kingdom */
    {235, "77", "GB"}, /* United Kingdom */
    {235, "91", "GB"}, /* United Kingdom */
    {235, "92", "GB"}, /* United Kingdom */
    {235, "94", "GB"}, /* United Kingdom */
    {235, "95", "GB"}, /* United Kingdom */
    {238, "01", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "02", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "03", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "04", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "05", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "06", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "07", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "08", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "09", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "10", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "11", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "12", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "13", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "14", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "15", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "16", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "17", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "18", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "20", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "23", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "25", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "28", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "30", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "40", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "42", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "43", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "66", "DK"}, /* Denmark (Kingdom of Denmark) */
    {238, "77", "DK"}, /* Denmark (Kingdom of Denmark) */
    {240, "01", "SE"}, /* Sweden */
    {240, "02", "SE"}, /* Sweden */
    {240, "03", "SE"}, /* Sweden */
    {240, "04", "SE"}, /* Sweden */
    {240, "05", "SE"}, /* Sweden */
    {240, "06", "SE"}, /* Sweden */
    {240, "07", "SE"}, /* Sweden */
    {240, "08", "SE"}, /* Sweden */
    {240, "09", "SE"}, /* Sweden */
    {240, "10", "SE"}, /* Sweden */
    {240, "11", "SE"}, /* Sweden */
    {240, "12", "SE"}, /* Sweden */
    {240, "13", "SE"}, /* Sweden */
    {240, "14", "SE"}, /* Sweden */
    {240, "15", "SE"}, /* Sweden */
    {240, "16", "SE"}, /* Sweden */
    {240, "17", "SE"}, /* Sweden */
    {240, "18", "SE"}, /* Sweden */
    {240, "19", "SE"}, /* Sweden */
    {240, "20", "SE"}, /* Sweden */
    {240, "21", "SE"}, /* Sweden */
    {240, "22", "SE"}, /* Sweden */
    {240, "23", "SE"}, /* Sweden */
    {240, "24", "SE"}, /* Sweden */
    {240, "25", "SE"}, /* Sweden */
    {240, "26", "SE"}, /* Sweden */
    {240, "27", "SE"}, /* Sweden */
    {240, "28", "SE"}, /* Sweden */
    {240, "29", "SE"}, /* Sweden */
    {240, "30", "SE"}, /* Sweden */
    {240, "31", "SE"}, /* Sweden */
    {240, "32", "SE"}, /* Sweden */
    {240, "33", "SE"}, /* Sweden */
    {240, "34", "SE"}, /* Sweden */
    {240, "35", "SE"}, /* Sweden */
    {240, "36", "SE"}, /* Sweden */
    {240, "37", "SE"}, /* Sweden */
    {240, "38", "SE"}, /* Sweden */
    {240, "39", "SE"}, /* Sweden */
    {240, "40", "SE"}, /* Sweden */
    {240, "41", "SE"}, /* Sweden */
    {240, "42", "SE"}, /* Sweden */
    {240, "43", "SE"}, /* Sweden */
    {240, "44", "SE"}, /* Sweden */
    {240, "45", "SE"}, /* Sweden */
    {240, "46", "SE"}, /* Sweden */
    {240, "60", "SE"}, /* Sweden */
    {242, "01", "NO"}, /* Norway */
    {242, "02", "NO"}, /* Norway */
    {242, "03", "NO"}, /* Norway */
    {242, "04", "NO"}, /* Norway */
    {242, "05", "NO"}, /* Norway */
    {242, "06", "NO"}, /* Norway */
    {242, "07", "NO"}, /* Norway */
    {242, "08", "NO"}, /* Norway */
    {242, "09", "NO"}, /* Norway */
    {242, "10", "NO"}, /* Norway */
    {242, "11", "NO"}, /* Norway */
    {242, "12", "NO"}, /* Norway */
    {242, "14", "NO"}, /* Norway */
    {242, "20", "NO"}, /* Norway */
    {242, "21", "NO"}, /* Norway */
    {242, "22", "NO"}, /* Norway */
    {242, "23", "NO"}, /* Norway */
    {242, "24", "NO"}, /* Norway */
    {242, "25", "NO"}, /* Norway */
    {242, "99", "NO"}, /* Norway */
    {244, "03", "FI"}, /* Finland */
    {244, "04", "FI"}, /* Finland */
    {244, "05", "FI"}, /* Finland */
    {244, "06", "FI"}, /* Finland */
    {244, "07", "FI"}, /* Finland */
    {244, "08", "FI"}, /* Finland */
    {244, "09", "FI"}, /* Finland */
    {244, "10", "FI"}, /* Finland */
    {244, "11", "FI"}, /* Finland */
    {244, "12", "FI"}, /* Finland */
    {244, "13", "FI"}, /* Finland */
    {244, "14", "FI"}, /* Finland */
    {244, "15", "FI"}, /* Finland */
    {244, "16", "FI"}, /* Finland */
    {244, "17", "FI"}, /* Finland */
    {244, "21", "FI"}, /* Finland */
    {244, "22", "FI"}, /* Finland */
    {244, "23", "FI"}, /* Finland */
    {244, "24", "FI"}, /* Finland */
    {244, "25", "FI"}, /* Finland */
    {244, "26", "FI"}, /* Finland */
    {244, "27", "FI"}, /* Finland */
    {244, "28", "FI"}, /* Finland */
    {244, "29", "FI"}, /* Finland */
    {244, "30", "FI"}, /* Finland */
    {244, "31", "FI"}, /* Finland */
    {244, "32", "FI"}, /* Finland */
    {244, "33", "FI"}, /* Finland */
    {244, "34", "FI"}, /* Finland */
    {244, "35", "FI"}, /* Finland */
    {244, "36", "FI"}, /* Finland */
    {244, "37", "FI"}, /* Finland */
    {244, "38", "FI"}, /* Finland */
    {244, "39", "FI"}, /* Finland */
    {244, "40", "FI"}, /* Finland */
    {244, "41", "FI"}, /* Finland */
    {244, "42", "FI"}, /* Finland */
    {244, "91", "FI"}, /* Finland */
    {244, "92", "FI"}, /* Finland */
    {246, "01", "LT"}, /* Lithuania */
    {246, "02", "LT"}, /* Lithuania */
    {246, "03", "LT"}, /* Lithuania */
    {246, "04", "LT"}, /* Lithuania */
    {246, "05", "LT"}, /* Lithuania */
    {246, "06", "LT"}, /* Lithuania */
    {246, "07", "LT"}, /* Lithuania */
    {246, "08", "LT"}, /* Lithuania */
    {246, "09", "LT"}, /* Lithuania */
    {247, "01", "LV"}, /* Latvia */
    {247, "02", "LV"}, /* Latvia */
    {247, "03", "LV"}, /* Latvia */
    {247, "04", "LV"}, /* Latvia */
    {247, "05", "LV"}, /* Latvia */
    {247, "06", "LV"}, /* Latvia */
    {247, "07", "LV"}, /* Latvia */
    {247, "08", "LV"}, /* Latvia */
    {247, "09", "LV"}, /* Latvia */
    {248, "01", "EE"}, /* Estonia */
    {248, "02", "EE"}, /* Estonia */
    {248, "03", "EE"}, /* Estonia */
    {248, "04", "EE"}, /* Estonia */
    {248, "05", "EE"}, /* Estonia */
    {248, "06", "EE"}, /* Estonia */
    {248, "07", "EE"}, /* Estonia */
    {248, "08", "EE"}, /* Estonia */
    {248, "09", "EE"}, /* Estonia */
    {248, "10", "EE"}, /* Estonia */
    {248, "71", "EE"}, /* Estonia */
    {250, "01", "RU"}, /* Russian Federation */
    {250, "02", "RU"}, /* Russian Federation */
    {250, "03", "RU"}, /* Russian Federation */
    {250, "04", "RU"}, /* Russian Federation */
    {250, "05", "RU"}, /* Russian Federation */
    {250, "06", "RU"}, /* Russian Federation */
    {250, "07", "RU"}, /* Russian Federation */
    {250, "09", "RU"}, /* Russian Federation */
    {250, "10", "RU"}, /* Russian Federation */
    {250, "11", "RU"}, /* Russian Federation */
    {250, "12", "RU"}, /* Russian Federation */
    {250, "13", "RU"}, /* Russian Federation */
    {250, "14", "RU"}, /* Russian Federation */
    {250, "15", "RU"}, /* Russian Federation */
    {250, "16", "RU"}, /* Russian Federation */
    {250, "17", "RU"}, /* Russian Federation */
    {250, "18", "RU"}, /* Russian Federation */
    {250, "19", "RU"}, /* Russian Federation */
    {250, "20", "RU"}, /* Russian Federation */
    {250, "22", "RU"}, /* Russian Federation */
    {250, "23", "RU"}, /* Russian Federation */
    {250, "28", "RU"}, /* Russian Federation */
    {250, "32", "RU"}, /* Russian Federation */
    {250, "33", "RU"}, /* Russian Federation */
    {250, "34", "RU"}, /* Russian Federation */
    {250, "35", "RU"}, /* Russian Federation */
    {250, "38", "RU"}, /* Russian Federation */
    {250, "39", "RU"}, /* Russian Federation */
    {250, "44", "RU"}, /* Russian Federation */
    {250, "50", "RU"}, /* Russian Federation */
    {250, "54", "RU"}, /* Russian Federation */
    {250, "60", "RU"}, /* Russian Federation */
    {250, "811", "RU"}, /* Russian Federation */
    {250, "91", "RU"}, /* Russian Federation */
    {250, "92", "RU"}, /* Russian Federation */
    {250, "93", "RU"}, /* Russian Federation */
    {250, "99", "RU"}, /* Russian Federation */
    {255, "01", "UA"}, /* Ukraine */
    {255, "02", "UA"}, /* Ukraine */
    {255, "03", "UA"}, /* Ukraine */
    {255, "04", "UA"}, /* Ukraine */
    {255, "05", "UA"}, /* Ukraine */
    {255, "06", "UA"}, /* Ukraine */
    {255, "07", "UA"}, /* Ukraine */
    {255, "21", "UA"}, /* Ukraine */
    {255, "23", "UA"}, /* Ukraine */
    {255, "25", "UA"}, /* Ukraine */
    {257, "01", "BY"}, /* Belarus */
    {257, "02", "BY"}, /* Belarus */
    {257, "03", "BY"}, /* Belarus */
    {257, "04", "BY"}, /* Belarus */
    {257, "05", "BY"}, /* Belarus */
    {257, "06", "BY"}, /* Belarus */
    {259, "01", "MD"}, /* Moldova */
    {259, "02", "MD"}, /* Moldova */
    {259, "03", "MD"}, /* Moldova */
    {259, "04", "MD"}, /* Moldova */
    {259, "15", "MD"}, /* Moldova */
    {259, "99", "MD"}, /* Moldova */
    {260, "01", "PL"}, /* Poland */
    {260, "02", "PL"}, /* Poland */
    {260, "03", "PL"}, /* Poland */
    {260, "04", "PL"}, /* Poland */
    {260, "05", "PL"}, /* Poland */
    {260, "06", "PL"}, /* Poland */
    {260, "07", "PL"}, /* Poland */
    {260, "08", "PL"}, /* Poland */
    {260, "09", "PL"}, /* Poland */
    {260, "10", "PL"}, /* Poland */
    {260, "11", "PL"}, /* Poland */
    {260, "12", "PL"}, /* Poland */
    {260, "13", "PL"}, /* Poland */
    {260, "14", "PL"}, /* Poland */
    {260, "15", "PL"}, /* Poland */
    {260, "16", "PL"}, /* Poland */
    {260, "17", "PL"}, /* Poland */
    {260, "18", "PL"}, /* Poland */
    {260, "19", "PL"}, /* Poland */
    {260, "20", "PL"}, /* Poland */
    {260, "21", "PL"}, /* Poland */
    {260, "22", "PL"}, /* Poland */
    {260, "23", "PL"}, /* Poland */
    {260, "24", "PL"}, /* Poland */
    {260, "25", "PL"}, /* Poland */
    {260, "26", "PL"}, /* Poland */
    {260, "27", "PL"}, /* Poland */
    {260, "28", "PL"}, /* Poland */
    {260, "29", "PL"}, /* Poland */
    {260, "30", "PL"}, /* Poland */
    {260, "31", "PL"}, /* Poland */
    {260, "32", "PL"}, /* Poland */
    {260, "33", "PL"}, /* Poland */
    {260, "34", "PL"}, /* Poland */
    {260, "35", "PL"}, /* Poland */
    {260, "36", "PL"}, /* Poland */
    {260, "37", "PL"}, /* Poland */
    {260, "38", "PL"}, /* Poland */
    {260, "39", "PL"}, /* Poland */
    {260, "40", "PL"}, /* Poland */
    {260, "41", "PL"}, /* Poland */
    {260, "42", "PL"}, /* Poland */
    {260, "43", "PL"}, /* Poland */
    {260, "44", "PL"}, /* Poland */
    {260, "45", "PL"}, /* Poland */
    {260, "46", "PL"}, /* Poland */
    {260, "47", "PL"}, /* Poland */
    {260, "48", "PL"}, /* Poland */
    {260, "49", "PL"}, /* Poland */
    {260, "98", "PL"}, /* Poland */
    {262, "01", "DE"}, /* Germany */
    {262, "02", "DE"}, /* Germany */
    {262, "03", "DE"}, /* Germany */
    {262, "04", "DE"}, /* Germany */
    {262, "05", "DE"}, /* Germany */
    {262, "06", "DE"}, /* Germany */
    {262, "07", "DE"}, /* Germany */
    {262, "08", "DE"}, /* Germany */
    {262, "09", "DE"}, /* Germany */
    {262, "10", "DE"}, /* Germany */
    {262, "11", "DE"}, /* Germany */
    {262, "12", "DE"}, /* Germany */
    {262, "13", "DE"}, /* Germany */
    {262, "14", "DE"}, /* Germany */
    {262, "15", "DE"}, /* Germany */
    {262, "16", "DE"}, /* Germany */
    {262, "17", "DE"}, /* Germany */
    {262, "18", "DE"}, /* Germany */
    {262, "19", "DE"}, /* Germany */
    {262, "20", "DE"}, /* Germany */
    {262, "21", "DE"}, /* Germany */
    {262, "22", "DE"}, /* Germany */
    {262, "23", "DE"}, /* Germany */
    {262, "33", "DE"}, /* Germany */
    {262, "41", "DE"}, /* Germany */
    {262, "42", "DE"}, /* Germany */
    {262, "43", "DE"}, /* Germany */
    {262, "60", "DE"}, /* Germany */
    {262, "72", "DE"}, /* Germany */
    {262, "73", "DE"}, /* Germany */
    {262, "74", "DE"}, /* Germany */
    {262, "75", "DE"}, /* Germany */
    {262, "76", "DE"}, /* Germany */
    {262, "77", "DE"}, /* Germany */
    {262, "78", "DE"}, /* Germany */
    {262, "79", "DE"}, /* Germany */
    {262, "92", "DE"}, /* Germany */
    {266, "01", "GI"}, /* Gibraltar (United Kingdom) */
    {266, "06", "GI"}, /* Gibraltar (United Kingdom) */
    {266, "09", "GI"}, /* Gibraltar (United Kingdom) */
    {268, "01", "PT"}, /* Portugal */
    {268, "02", "PT"}, /* Portugal */
    {268, "03", "PT"}, /* Portugal */
    {268, "04", "PT"}, /* Portugal */
    {268, "06", "PT"}, /* Portugal */
    {268, "07", "PT"}, /* Portugal */
    {268, "11", "PT"}, /* Portugal */
    {268, "12", "PT"}, /* Portugal */
    {268, "13", "PT"}, /* Portugal */
    {268, "21", "PT"}, /* Portugal */
    {268, "80", "PT"}, /* Portugal */
    {270, "01", "LU"}, /* Luxembourg */
    {270, "02", "LU"}, /* Luxembourg */
    {270, "10", "LU"}, /* Luxembourg */
    {270, "77", "LU"}, /* Luxembourg */
    {270, "78", "LU"}, /* Luxembourg */
    {270, "99", "LU"}, /* Luxembourg */
    {272, "01", "IE"}, /* Republic of Ireland|Ireland */
    {272, "02", "IE"}, /* Republic of Ireland|Ireland */
    {272, "03", "IE"}, /* Republic of Ireland|Ireland */
    {272, "04", "IE"}, /* Republic of Ireland|Ireland */
    {272, "05", "IE"}, /* Republic of Ireland|Ireland */
    {272, "07", "IE"}, /* Republic of Ireland|Ireland */
    {272, "09", "IE"}, /* Republic of Ireland|Ireland */
    {272, "11", "IE"}, /* Republic of Ireland|Ireland */
    {272, "13", "IE"}, /* Republic of Ireland|Ireland */
    {272, "15", "IE"}, /* Republic of Ireland|Ireland */
    {272, "16", "IE"}, /* Republic of Ireland|Ireland */
    {274, "01", "IS"}, /* Iceland */
    {274, "02", "IS"}, /* Iceland */
    {274, "03", "IS"}, /* Iceland */
    {274, "04", "IS"}, /* Iceland */
    {274, "05", "IS"}, /* Iceland */
    {274, "06", "IS"}, /* Iceland */
    {274, "07", "IS"}, /* Iceland */
    {274, "08", "IS"}, /* Iceland */
    {274, "11", "IS"}, /* Iceland */
    {274, "12", "IS"}, /* Iceland */
    {274, "16", "IS"}, /* Iceland */
    {274, "22", "IS"}, /* Iceland */
    {274, "31", "IS"}, /* Iceland */
    {276, "01", "AL"}, /* Albania */
    {276, "02", "AL"}, /* Albania */
    {276, "03", "AL"}, /* Albania */
    {276, "04", "AL"}, /* Albania */
    {278, "01", "MT"}, /* Malta */
    {278, "11", "MT"}, /* Malta */
    {278, "21", "MT"}, /* Malta */
    {278, "30", "MT"}, /* Malta */
    {278, "77", "MT"}, /* Malta */
    {280, "01", "CY"}, /* Cyprus */
    {280, "10", "CY"}, /* Cyprus */
    {280, "20", "CY"}, /* Cyprus */
    {280, "22", "CY"}, /* Cyprus */
    {280, "23", "CY"}, /* Cyprus */
    {282, "01", "GE"}, /* Georgia */
    {282, "02", "GE"}, /* Georgia */
    {282, "03", "GE"}, /* Georgia */
    {282, "04", "GE"}, /* Georgia */
    {282, "05", "GE"}, /* Georgia */
    {282, "06", "GE"}, /* Georgia */
    {282, "07", "GE"}, /* Georgia */
    {282, "08", "GE"}, /* Georgia */
    {282, "09", "GE"}, /* Georgia */
    {283, "01", "AM"}, /* Armenia */
    {283, "04", "AM"}, /* Armenia */
    {283, "05", "AM"}, /* Armenia */
    {283, "10", "AM"}, /* Armenia */
    {284, "01", "BG"}, /* Bulgaria */
    {284, "03", "BG"}, /* Bulgaria */
    {284, "07", "BG"}, /* Bulgaria */
    {284, "09", "BG"}, /* Bulgaria */
    {284, "11", "BG"}, /* Bulgaria */
    {284, "13", "BG"}, /* Bulgaria */
    {286, "01", "TR"}, /* Turkey */
    {286, "02", "TR"}, /* Turkey */
    {286, "03", "TR"}, /* Turkey */
    {286, "04", "TR"}, /* Turkey */
    {288, "01", "FO"}, /* Faroe Islands (Kingdom of Denmark) */
    {288, "02", "FO"}, /* Faroe Islands (Kingdom of Denmark) */
    {289, "67", "GE"}, /* Abkhazia */
    {289, "88", "GE"}, /* Abkhazia */
    {290, "01", "GL"}, /* Greenland (Kingdom of Denmark) */
    {290, "02", "GL"}, /* Greenland (Kingdom of Denmark) */
    {292, "01", "SM"}, /* San Marino */
    {293, "10", "SI"}, /* Slovenia */
    {293, "20", "SI"}, /* Slovenia */
    {293, "40", "SI"}, /* Slovenia */
    {293, "41", "SI"}, /* Slovenia */
    {293, "64", "SI"}, /* Slovenia */
    {293, "70", "SI"}, /* Slovenia */
    {294, "01", "MK"}, /* Macedonia */
    {294, "02", "MK"}, /* Macedonia */
    {294, "03", "MK"}, /* Macedonia */
    {294, "04", "MK"}, /* Macedonia */
    {294, "10", "MK"}, /* Macedonia */
    {294, "11", "MK"}, /* Macedonia */
    {295, "01", "LI"}, /* Liechtenstein */
    {295, "02", "LI"}, /* Liechtenstein */
    {295, "05", "LI"}, /* Liechtenstein */
    {295, "06", "LI"}, /* Liechtenstein */
    {295, "07", "LI"}, /* Liechtenstein */
    {295, "09", "LI"}, /* Liechtenstein */
    {295, "10", "LI"}, /* Liechtenstein */
    {297, "01", "ME"}, /* Montenegro */
    {297, "02", "ME"}, /* Montenegro */
    {297, "03", "ME"}, /* Montenegro */
    {302, "220", "CA"}, /* Canada */
    {302, "221", "CA"}, /* Canada */
    {302, "222", "CA"}, /* Canada */
    {302, "250", "CA"}, /* Canada */
    {302, "270", "CA"}, /* Canada */
    {302, "290", "CA"}, /* Canada */
    {302, "300", "CA"}, /* Canada */
    {302, "320", "CA"}, /* Canada */
    {302, "340", "CA"}, /* Canada */
    {302, "350", "CA"}, /* Canada */
    {302, "360", "CA"}, /* Canada */
    {302, "361", "CA"}, /* Canada */
    {302, "370", "CA"}, /* Canada */
    {302, "380", "CA"}, /* Canada */
    {302, "390", "CA"}, /* Canada */
    {302, "480", "CA"}, /* Canada */
    {302, "490", "CA"}, /* Canada */
    {302, "500", "CA"}, /* Canada */
    {302, "510", "CA"}, /* Canada */
    {302, "520", "CA"}, /* Canada */
    {302, "530", "CA"}, /* Canada */
    {302, "540", "CA"}, /* Canada */
    {302, "560", "CA"}, /* Canada */
    {302, "570", "CA"}, /* Canada */
    {302, "590", "CA"}, /* Canada */
    {302, "610", "CA"}, /* Canada */
    {302, "620", "CA"}, /* Canada */
    {302, "630", "CA"}, /* Canada */
    {302, "640", "CA"}, /* Canada */
    {302, "650", "CA"}, /* Canada */
    {302, "652", "CA"}, /* Canada */
    {302, "653", "CA"}, /* Canada */
    {302, "655", "CA"}, /* Canada */
    {302, "656", "CA"}, /* Canada */
    {302, "657", "CA"}, /* Canada */
    {302, "660", "CA"}, /* Canada */
    {302, "670", "CA"}, /* Canada */
    {302, "680", "CA"}, /* Canada */
    {302, "690", "CA"}, /* Canada */
    {302, "701", "CA"}, /* Canada */
    {302, "702", "CA"}, /* Canada */
    {302, "703", "CA"}, /* Canada */
    {302, "710", "CA"}, /* Canada */
    {302, "720", "CA"}, /* Canada */
    {302, "730", "CA"}, /* Canada */
    {302, "740", "CA"}, /* Canada */
    {302, "750", "CA"}, /* Canada */
    {302, "760", "CA"}, /* Canada */
    {302, "770", "CA"}, /* Canada */
    {302, "780", "CA"}, /* Canada */
    {302, "790", "CA"}, /* Canada */
    {302, "820", "CA"}, /* Canada */
    {302, "860", "CA"}, /* Canada */
    {302, "880", "CA"}, /* Canada */
    {302, "920", "CA"}, /* Canada */
    {302, "940", "CA"}, /* Canada */
    {302, "990", "CA"}, /* Canada */
    {308, "01", "PM"}, /* Saint Pierre and Miquelon (France) */
    {308, "02", "PM"}, /* Saint Pierre and Miquelon (France) */
    {310, "004", "US"}, /* United States of America */
    {310, "005", "US"}, /* United States of America */
    {310, "006", "US"}, /* United States of America */
    {310, "010", "US"}, /* United States of America */
    {310, "012", "US"}, /* United States of America */
    {310, "013", "US"}, /* United States of America */
    {310, "014", "US"}, /* United States of America */
    {310, "015", "US"}, /* United States of America */
    {310, "016", "US"}, /* United States of America */
    {310, "017", "US"}, /* United States of America */
    {310, "020", "US"}, /* United States of America */
    {310, "030", "US"}, /* United States of America */
    {310, "032", "GU"}, /* Guam (United States of America) */
    {310, "032", "US"}, /* United States of America */
    {310, "033", "GU"}, /* Guam (United States of America) */
    {310, "033", "US"}, /* United States of America */
    {310, "034", "US"}, /* United States of America */
    {310, "035", "US"}, /* United States of America */
    {310, "040", "US"}, /* United States of America */
    {310, "050", "US"}, /* United States of America */
    {310, "053", "US"}, /* United States of America */
    {310, "054", "US"}, /* United States of America */
    {310, "060", "US"}, /* United States of America */
    {310, "066", "US"}, /* United States of America */
    {310, "070", "US"}, /* United States of America */
    {310, "080", "US"}, /* United States of America */
    {310, "090", "US"}, /* United States of America */
    {310, "100", "US"}, /* United States of America */
    {310, "110", "MP"}, /* Northern Mariana Islands (United States of America) */
    {310, "110", "US"}, /* United States of America */
    {310, "120", "US"}, /* United States of America */
    {310, "130", "US"}, /* United States of America */
    {310, "140", "GU"}, /* Guam (United States of America) */
    {310, "140", "US"}, /* United States of America */
    {310, "150", "US"}, /* United States of America */
    {310, "160", "US"}, /* United States of America */
    {310, "170", "US"}, /* United States of America */
    {310, "180", "US"}, /* United States of America */
    {310, "190", "US"}, /* United States of America */
    {310, "200", "US"}, /* United States of America */
    {310, "210", "US"}, /* United States of America */
    {310, "220", "US"}, /* United States of America */
    {310, "230", "US"}, /* United States of America */
    {310, "240", "US"}, /* United States of America */
    {310, "250", "US"}, /* United States of America */
    {310, "260", "US"}, /* United States of America */
    {310, "270", "US"}, /* United States of America */
    {310, "280", "US"}, /* United States of America */
    {310, "290", "US"}, /* United States of America */
    {310, "300", "US"}, /* United States of America */
    {310, "310", "US"}, /* United States of America */
    {310, "311", "US"}, /* United States of America */
    {310, "320", "US"}, /* United States of America */
    {310, "330", "US"}, /* United States of America */
    {310, "340", "US"}, /* United States of America */
    {310, "350", "US"}, /* United States of America */
    {310, "360", "US"}, /* United States of America */
    {310, "370", "GU"}, /* Guam (United States of America) */
    {310, "370", "MP"}, /* Northern Mariana Islands (United States of America) */
    {310, "370", "US"}, /* United States of America */
    {310, "380", "US"}, /* United States of America */
    {310, "390", "US"}, /* United States of America */
    {310, "400", "GU"}, /* Guam (United States of America) */
    {310, "400", "US"}, /* United States of America */
    {310, "410", "US"}, /* United States of America */
    {310, "420", "US"}, /* United States of America */
    {310, "430", "US"}, /* United States of America */
    {310, "440", "US"}, /* United States of America */
    {310, "450", "US"}, /* United States of America */
    {310, "460", "US"}, /* United States of America */
    {310, "470", "US"}, /* United States of America */
    {310, "480", "GU"}, /* Guam (United States of America) */
    {310, "480", "US"}, /* United States of America */
    {310, "490", "US"}, /* United States of America */
    {310, "500", "US"}, /* United States of America */
    {310, "510", "US"}, /* United States of America */
    {310, "520", "US"}, /* United States of America */
    {310, "530", "US"}, /* United States of America */
    {310, "540", "US"}, /* United States of America */
    {310, "550", "US"}, /* United States of America */
    {310, "560", "US"}, /* United States of America */
    {310, "570", "US"}, /* United States of America */
    {310, "580", "US"}, /* United States of America */
    {310, "590", "US"}, /* United States of America */
    {310, "59", "BM"}, /* Bermuda */
    {310, "600", "US"}, /* United States of America */
    {310, "620", "US"}, /* United States of America */
    {310, "630", "US"}, /* United States of America */
    {310, "640", "US"}, /* United States of America */
    {310, "650", "US"}, /* United States of America */
    {310, "660", "US"}, /* United States of America */
    {310, "670", "US"}, /* United States of America */
    {310, "680", "US"}, /* United States of America */
    {310, "690", "US"}, /* United States of America */
    {310, "700", "US"}, /* United States of America */
    {310, "710", "US"}, /* United States of America */
    {310, "720", "US"}, /* United States of America */
    {310, "730", "US"}, /* United States of America */
    {310, "740", "US"}, /* United States of America */
    {310, "750", "US"}, /* United States of America */
    {310, "760", "US"}, /* United States of America */
    {310, "770", "US"}, /* United States of America */
    {310, "780", "US"}, /* United States of America */
    {310, "790", "US"}, /* United States of America */
    {310, "800", "US"}, /* United States of America */
    {310, "810", "US"}, /* United States of America */
    {310, "820", "US"}, /* United States of America */
    {310, "830", "US"}, /* United States of America */
    {310, "840", "US"}, /* United States of America */
    {310, "850", "US"}, /* United States of America */
    {310, "860", "US"}, /* United States of America */
    {310, "870", "US"}, /* United States of America */
    {310, "880", "US"}, /* United States of America */
    {310, "890", "US"}, /* United States of America */
    {310, "900", "US"}, /* United States of America */
    {310, "910", "US"}, /* United States of America */
    {310, "920", "US"}, /* United States of America */
    {310, "930", "US"}, /* United States of America */
    {310, "940", "US"}, /* United States of America */
    {310, "950", "US"}, /* United States of America */
    {310, "960", "US"}, /* United States of America */
    {310, "970", "US"}, /* United States of America */
    {310, "980", "US"}, /* United States of America */
    {310, "990", "US"}, /* United States of America */
    {311, "000", "US"}, /* United States of America */
    {311, "010", "US"}, /* United States of America */
    {311, "012", "US"}, /* United States of America */
    {311, "020", "US"}, /* United States of America */
    {311, "030", "US"}, /* United States of America */
    {311, "040", "US"}, /* United States of America */
    {311, "050", "US"}, /* United States of America */
    {311, "060", "US"}, /* United States of America */
    {311, "070", "US"}, /* United States of America */
    {311, "080", "US"}, /* United States of America */
    {311, "090", "US"}, /* United States of America */
    {311, "100", "US"}, /* United States of America */
    {311, "110", "US"}, /* United States of America */
    {311, "120", "GU"}, /* Guam (United States of America) */
    {311, "120", "US"}, /* United States of America */
    {311, "130", "US"}, /* United States of America */
    {311, "140", "US"}, /* United States of America */
    {311, "150", "US"}, /* United States of America */
    {311, "160", "US"}, /* United States of America */
    {311, "170", "US"}, /* United States of America */
    {311, "180", "US"}, /* United States of America */
    {311, "190", "US"}, /* United States of America */
    {311, "200", "US"}, /* United States of America */
    {311, "210", "US"}, /* United States of America */
    {311, "220", "US"}, /* United States of America */
    {311, "230", "US"}, /* United States of America */
    {311, "240", "US"}, /* United States of America */
    {311, "250", "GU"}, /* Guam (United States of America) */
    {311, "250", "US"}, /* United States of America */
    {311, "270", "US"}, /* United States of America */
    {311, "271", "US"}, /* United States of America */
    {311, "272", "US"}, /* United States of America */
    {311, "273", "US"}, /* United States of America */
    {311, "274", "US"}, /* United States of America */
    {311, "275", "US"}, /* United States of America */
    {311, "276", "US"}, /* United States of America */
    {311, "277", "US"}, /* United States of America */
    {311, "278", "US"}, /* United States of America */
    {311, "279", "US"}, /* United States of America */
    {311, "280", "US"}, /* United States of America */
    {311, "281", "US"}, /* United States of America */
    {311, "282", "US"}, /* United States of America */
    {311, "283", "US"}, /* United States of America */
    {311, "284", "US"}, /* United States of America */
    {311, "285", "US"}, /* United States of America */
    {311, "286", "US"}, /* United States of America */
    {311, "287", "US"}, /* United States of America */
    {311, "288", "US"}, /* United States of America */
    {311, "289", "US"}, /* United States of America */
    {311, "290", "US"}, /* United States of America */
    {311, "300", "US"}, /* United States of America */
    {311, "310", "US"}, /* United States of America */
    {311, "320", "US"}, /* United States of America */
    {311, "330", "US"}, /* United States of America */
    {311, "340", "US"}, /* United States of America */
    {311, "350", "US"}, /* United States of America */
    {311, "360", "US"}, /* United States of America */
    {311, "370", "US"}, /* United States of America */
    {311, "380", "US"}, /* United States of America */
    {311, "390", "US"}, /* United States of America */
    {311, "400", "US"}, /* United States of America */
    {311, "410", "US"}, /* United States of America */
    {311, "420", "US"}, /* United States of America */
    {311, "430", "US"}, /* United States of America */
    {311, "440", "US"}, /* United States of America */
    {311, "450", "US"}, /* United States of America */
    {311, "460", "US"}, /* United States of America */
    {311, "470", "US"}, /* United States of America */
    {311, "480", "US"}, /* United States of America */
    {311, "481", "US"}, /* United States of America */
    {311, "482", "US"}, /* United States of America */
    {311, "483", "US"}, /* United States of America */
    {311, "484", "US"}, /* United States of America */
    {311, "485", "US"}, /* United States of America */
    {311, "486", "US"}, /* United States of America */
    {311, "487", "US"}, /* United States of America */
    {311, "488", "US"}, /* United States of America */
    {311, "489", "US"}, /* United States of America */
    {311, "490", "US"}, /* United States of America */
    {311, "500", "US"}, /* United States of America */
    {311, "510", "US"}, /* United States of America */
    {311, "520", "US"}, /* United States of America */
    {311, "530", "US"}, /* United States of America */
    {311, "540", "US"}, /* United States of America */
    {311, "550", "US"}, /* United States of America */
    {311, "560", "US"}, /* United States of America */
    {311, "570", "US"}, /* United States of America */
    {311, "580", "US"}, /* United States of America */
    {311, "590", "US"}, /* United States of America */
    {311, "600", "US"}, /* United States of America */
    {311, "610", "US"}, /* United States of America */
    {311, "620", "US"}, /* United States of America */
    {311, "630", "US"}, /* United States of America */
    {311, "640", "US"}, /* United States of America */
    {311, "650", "US"}, /* United States of America */
    {311, "660", "US"}, /* United States of America */
    {311, "670", "US"}, /* United States of America */
    {311, "680", "US"}, /* United States of America */
    {311, "690", "US"}, /* United States of America */
    {311, "700", "US"}, /* United States of America */
    {311, "710", "US"}, /* United States of America */
    {311, "720", "US"}, /* United States of America */
    {311, "730", "US"}, /* United States of America */
    {311, "740", "US"}, /* United States of America */
    {311, "750", "US"}, /* United States of America */
    {311, "760", "US"}, /* United States of America */
    {311, "770", "US"}, /* United States of America */
    {311, "780", "US"}, /* United States of America */
    {311, "790", "US"}, /* United States of America */
    {311, "800", "US"}, /* United States of America */
    {311, "810", "US"}, /* United States of America */
    {311, "820", "US"}, /* United States of America */
    {311, "830", "US"}, /* United States of America */
    {311, "840", "US"}, /* United States of America */
    {311, "850", "US"}, /* United States of America */
    {311, "860", "US"}, /* United States of America */
    {311, "870", "US"}, /* United States of America */
    {311, "880", "US"}, /* United States of America */
    {311, "890", "US"}, /* United States of America */
    {311, "900", "US"}, /* United States of America */
    {311, "910", "US"}, /* United States of America */
    {311, "920", "US"}, /* United States of America */
    {311, "930", "US"}, /* United States of America */
    {311, "940", "US"}, /* United States of America */
    {311, "950", "US"}, /* United States of America */
    {311, "960", "US"}, /* United States of America */
    {311, "970", "US"}, /* United States of America */
    {311, "980", "US"}, /* United States of America */
    {311, "990", "US"}, /* United States of America */
    {312, "010", "US"}, /* United States of America */
    {312, "020", "US"}, /* United States of America */
    {312, "030", "US"}, /* United States of America */
    {312, "040", "US"}, /* United States of America */
    {312, "050", "US"}, /* United States of America */
    {312, "060", "US"}, /* United States of America */
    {312, "070", "US"}, /* United States of America */
    {312, "080", "US"}, /* United States of America */
    {312, "090", "US"}, /* United States of America */
    {312, "100", "US"}, /* United States of America */
    {312, "110", "US"}, /* United States of America */
    {312, "120", "US"}, /* United States of America */
    {312, "130", "US"}, /* United States of America */
    {312, "140", "US"}, /* United States of America */
    {312, "150", "US"}, /* United States of America */
    {312, "160", "US"}, /* United States of America */
    {312, "170", "US"}, /* United States of America */
    {312, "180", "US"}, /* United States of America */
    {312, "190", "US"}, /* United States of America */
    {312, "200", "US"}, /* United States of America */
    {312, "210", "US"}, /* United States of America */
    {312, "220", "US"}, /* United States of America */
    {312, "230", "US"}, /* United States of America */
    {312, "240", "US"}, /* United States of America */
    {312, "250", "US"}, /* United States of America */
    {312, "260", "US"}, /* United States of America */
    {312, "270", "US"}, /* United States of America */
    {312, "280", "US"}, /* United States of America */
    {312, "290", "US"}, /* United States of America */
    {312, "300", "US"}, /* United States of America */
    {312, "310", "US"}, /* United States of America */
    {312, "320", "US"}, /* United States of America */
    {312, "330", "US"}, /* United States of America */
    {312, "340", "US"}, /* United States of America */
    {312, "350", "US"}, /* United States of America */
    {312, "360", "US"}, /* United States of America */
    {312, "370", "US"}, /* United States of America */
    {312, "380", "US"}, /* United States of America */
    {312, "390", "US"}, /* United States of America */
    {312, "400", "US"}, /* United States of America */
    {312, "410", "US"}, /* United States of America */
    {312, "420", "US"}, /* United States of America */
    {312, "430", "US"}, /* United States of America */
    {312, "440", "US"}, /* United States of America */
    {312, "450", "US"}, /* United States of America */
    {312, "460", "US"}, /* United States of America */
    {312, "470", "US"}, /* United States of America */
    {312, "480", "US"}, /* United States of America */
    {312, "490", "US"}, /* United States of America */
    {312, "500", "US"}, /* United States of America */
    {312, "510", "US"}, /* United States of America */
    {312, "520", "US"}, /* United States of America */
    {312, "530", "US"}, /* United States of America */
    {312, "540", "US"}, /* United States of America */
    {312, "550", "US"}, /* United States of America */
    {312, "560", "US"}, /* United States of America */
    {312, "570", "US"}, /* United States of America */
    {312, "580", "US"}, /* United States of America */
    {312, "590", "US"}, /* United States of America */
    {312, "600", "US"}, /* United States of America */
    {312, "610", "US"}, /* United States of America */
    {312, "620", "US"}, /* United States of America */
    {312, "630", "US"}, /* United States of America */
    {312, "640", "US"}, /* United States of America */
    {312, "650", "US"}, /* United States of America */
    {312, "660", "US"}, /* United States of America */
    {312, "670", "US"}, /* United States of America */
    {312, "680", "US"}, /* United States of America */
    {312, "690", "US"}, /* United States of America */
    {312, "700", "US"}, /* United States of America */
    {312, "710", "US"}, /* United States of America */
    {312, "720", "US"}, /* United States of America */
    {312, "730", "US"}, /* United States of America */
    {312, "740", "US"}, /* United States of America */
    {312, "750", "US"}, /* United States of America */
    {312, "760", "US"}, /* United States of America */
    {312, "770", "US"}, /* United States of America */
    {312, "780", "US"}, /* United States of America */
    {312, "790", "US"}, /* United States of America */
    {312, "800", "US"}, /* United States of America */
    {312, "810", "US"}, /* United States of America */
    {312, "820", "US"}, /* United States of America */
    {312, "830", "US"}, /* United States of America */
    {312, "840", "US"}, /* United States of America */
    {312, "850", "US"}, /* United States of America */
    {312, "860", "US"}, /* United States of America */
    {312, "870", "US"}, /* United States of America */
    {312, "880", "US"}, /* United States of America */
    {312, "890", "US"}, /* United States of America */
    {312, "900", "US"}, /* United States of America */
    {312, "910", "US"}, /* United States of America */
    {312, "920", "US"}, /* United States of America */
    {312, "930", "US"}, /* United States of America */
    {312, "940", "US"}, /* United States of America */
    {312, "950", "US"}, /* United States of America */
    {312, "960", "US"}, /* United States of America */
    {312, "970", "US"}, /* United States of America */
    {312, "980", "US"}, /* United States of America */
    {312, "990", "US"}, /* United States of America */
    {313, "000", "US"}, /* United States of America */
    {313, "010", "US"}, /* United States of America */
    {313, "020", "US"}, /* United States of America */
    {313, "030", "US"}, /* United States of America */
    {313, "040", "US"}, /* United States of America */
    {313, "050", "US"}, /* United States of America */
    {313, "060", "US"}, /* United States of America */
    {313, "070", "US"}, /* United States of America */
    {313, "080", "US"}, /* United States of America */
    {313, "090", "US"}, /* United States of America */
    {313, "100", "US"}, /* United States of America */
    {313, "110", "US"}, /* United States of America */
    {313, "200", "US"}, /* United States of America */
    {313, "210", "US"}, /* United States of America */
    {316, "010", "US"}, /* United States of America */
    {316, "011", "US"}, /* United States of America */
    {330, "000", "PR"}, /* Puerto Rico */
    {330, "110", "PR"}, /* Puerto Rico */
    {330, "120", "PR"}, /* Puerto Rico */
    {334, "001", "MX"}, /* Mexico */
    {334, "010", "MX"}, /* Mexico */
    {334, "020", "MX"}, /* Mexico */
    {334, "030", "MX"}, /* Mexico */
    {334, "040", "MX"}, /* Mexico */
    {334, "050", "MX"}, /* Mexico */
    {334, "060", "MX"}, /* Mexico */
    {334, "066", "MX"}, /* Mexico */
    {334, "070", "MX"}, /* Mexico */
    {334, "080", "MX"}, /* Mexico */
    {334, "090", "MX"}, /* Mexico */
    {338, "020", "JM"}, /* Jamaica */
    {338, "040", "JM"}, /* Jamaica */
    {338, "050", "BM"}, /* Bermuda */
    {338, "050", "JM"}, /* Jamaica */
    {338, "050", "TC"}, /* Turks and Caicos Islands */
    {338, "070", "JM"}, /* Jamaica */
    {338, "110", "JM"}, /* Jamaica */
    {338, "180", "JM"}, /* Jamaica */
    {340, "01", "BL"}, /* French Antilles (France) */
    {340, "02", "BL"}, /* French Antilles (France) */
    {340, "03", "BL"}, /* French Antilles (France) */
    {340, "08", "BL"}, /* French Antilles (France) */
    {340, "10", "BL"}, /* French Antilles (France) */
    {340, "11", "BL"}, /* French Antilles (France) */
    {340, "12", "BL"}, /* French Antilles (France) */
    {340, "20", "BL"}, /* French Antilles (France) */
    {342, "600", "BB"}, /* Barbados */
    {342, "820", "BB"}, /* Barbados */
    {344, "030", "AG"}, /* Antigua and Barbuda */
    {344, "920", "AG"}, /* Antigua and Barbuda */
    {344, "930", "AG"}, /* Antigua and Barbuda */
    {346, "050", "KY"}, /* Cayman Islands (United Kingdom) */
    {346, "140", "KY"}, /* Cayman Islands (United Kingdom) */
    {348, "170", "VG"}, /* British Virgin Islands (United Kingdom) */
    {348, "370", "VG"}, /* British Virgin Islands (United Kingdom) */
    {348, "570", "VG"}, /* British Virgin Islands (United Kingdom) */
    {348, "770", "VG"}, /* British Virgin Islands (United Kingdom) */
    {350, "01", "BM"}, /* Bermuda */
    {350, "02", "BM"}, /* Bermuda */
    {352, "030", "GD"}, /* Grenada */
    {352, "110", "GD"}, /* Grenada */
    {354, "860", "MS"}, /* Montserrat (United Kingdom) */
    {356, "050", "KN"}, /* Saint Kitts and Nevis */
    {356, "070", "KN"}, /* Saint Kitts and Nevis */
    {356, "110", "KN"}, /* Saint Kitts and Nevis */
    {358, "050", "LC"}, /* Saint Lucia */
    {358, "110", "LC"}, /* Saint Lucia */
    {360, "050", "VC"}, /* Saint Vincent and the Grenadines */
    {360, "100", "VC"}, /* Saint Vincent and the Grenadines */
    {360, "110", "VC"}, /* Saint Vincent and the Grenadines */
    {362, "31", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "33", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "51", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "54", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "59", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "60", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "63", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "68", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "69", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "74", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "76", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "78", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "91", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "94", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {362, "95", "BQ"}, /* Netherlands Antilles (Kingdom of the Netherlands) */
    {363, "02", "AW"}, /* Aruba (Kingdom of the Netherlands) */
    {364, "49", "BS"}, /* Bahamas */
    {365, "010", "AI"}, /* Anguilla (United Kingdom) */
    {366, "020", "DM"}, /* Dominica */
    {366, "110", "DM"}, /* Dominica */
    {368, "01", "CU"}, /* Cuba */
    {370, "01", "DO"}, /* Dominican Republic */
    {370, "03", "DO"}, /* Dominican Republic */
    {370, "04", "DO"}, /* Dominican Republic */
    {370, "05", "DO"}, /* Dominican Republic */
    {372, "01", "HT"}, /* Haiti */
    {372, "02", "HT"}, /* Haiti */
    {372, "03", "HT"}, /* Haiti */
    {374, "12", "TT"}, /* Trinidad and Tobago */
    {374, "130", "TT"}, /* Trinidad and Tobago */
    {374, "140", "TT"}, /* Trinidad and Tobago */
    {376, "350", "TC"}, /* Turks and Caicos Islands */
    {376, "352", "TC"}, /* Turks and Caicos Islands */
    {376, "360", "TC"}, /* Turks and Caicos Islands */
    {400, "01", "AZ"}, /* Azerbaijan */
    {400, "02", "AZ"}, /* Azerbaijan */
    {400, "03", "AZ"}, /* Azerbaijan */
    {400, "04", "AZ"}, /* Azerbaijan */
    {400, "06", "AZ"}, /* Azerbaijan */
    {401, "01", "KZ"}, /* Kazakhstan */
    {401, "02", "KZ"}, /* Kazakhstan */
    {401, "07", "KZ"}, /* Kazakhstan */
    {401, "08", "KZ"}, /* Kazakhstan */
    {401, "77", "KZ"}, /* Kazakhstan */
    {402, "77", "BT"}, /* Bhutan */
    {404, "01", "IN"}, /* India */
    {404, "02", "IN"}, /* India */
    {404, "03", "IN"}, /* India */
    {404, "04", "IN"}, /* India */
    {404, "05", "IN"}, /* India */
    {404, "07", "IN"}, /* India */
    {404, "09", "IN"}, /* India */
    {404, "10", "IN"}, /* India */
    {404, "11", "IN"}, /* India */
    {404, "12", "IN"}, /* India */
    {404, "13", "IN"}, /* India */
    {404, "14", "IN"}, /* India */
    {404, "15", "IN"}, /* India */
    {404, "16", "IN"}, /* India */
    {404, "17", "IN"}, /* India */
    {404, "18", "IN"}, /* India */
    {404, "19", "IN"}, /* India */
    {404, "20", "IN"}, /* India */
    {404, "21", "IN"}, /* India */
    {404, "22", "IN"}, /* India */
    {404, "24", "IN"}, /* India */
    {404, "25", "IN"}, /* India */
    {404, "27", "IN"}, /* India */
    {404, "28", "IN"}, /* India */
    {404, "29", "IN"}, /* India */
    {404, "30", "IN"}, /* India */
    {404, "31", "IN"}, /* India */
    {404, "34", "IN"}, /* India */
    {404, "36", "IN"}, /* India */
    {404, "37", "IN"}, /* India */
    {404, "38", "IN"}, /* India */
    {404, "40", "IN"}, /* India */
    {404, "41", "IN"}, /* India */
    {404, "42", "IN"}, /* India */
    {404, "43", "IN"}, /* India */
    {404, "44", "IN"}, /* India */
    {404, "45", "IN"}, /* India */
    {404, "46", "IN"}, /* India */
    {404, "48", "IN"}, /* India */
    {404, "49", "IN"}, /* India */
    {404, "50", "IN"}, /* India */
    {404, "51", "IN"}, /* India */
    {404, "52", "IN"}, /* India */
    {404, "53", "IN"}, /* India */
    {404, "54", "IN"}, /* India */
    {404, "55", "IN"}, /* India */
    {404, "56", "IN"}, /* India */
    {404, "57", "IN"}, /* India */
    {404, "58", "IN"}, /* India */
    {404, "59", "IN"}, /* India */
    {404, "60", "IN"}, /* India */
    {404, "62", "IN"}, /* India */
    {404, "64", "IN"}, /* India */
    {404, "66", "IN"}, /* India */
    {404, "67", "IN"}, /* India */
    {404, "68", "IN"}, /* India */
    {404, "69", "IN"}, /* India */
    {404, "70", "IN"}, /* India */
    {404, "71", "IN"}, /* India */
    {404, "72", "IN"}, /* India */
    {404, "73", "IN"}, /* India */
    {404, "74", "IN"}, /* India */
    {404, "75", "IN"}, /* India */
    {404, "76", "IN"}, /* India */
    {404, "77", "IN"}, /* India */
    {404, "78", "IN"}, /* India */
    {404, "79", "IN"}, /* India */
    {404, "80", "IN"}, /* India */
    {404, "81", "IN"}, /* India */
    {404, "82", "IN"}, /* India */
    {404, "83", "IN"}, /* India */
    {404, "84", "IN"}, /* India */
    {404, "85", "IN"}, /* India */
    {404, "86", "IN"}, /* India */
    {404, "87", "IN"}, /* India */
    {404, "88", "IN"}, /* India */
    {404, "89", "IN"}, /* India */
    {404, "90", "IN"}, /* India */
    {404, "91", "IN"}, /* India */
    {404, "92", "IN"}, /* India */
    {404, "93", "IN"}, /* India */
    {404, "94", "IN"}, /* India */
    {404, "95", "IN"}, /* India */
    {404, "96", "IN"}, /* India */
    {404, "97", "IN"}, /* India */
    {404, "98", "IN"}, /* India */
    {405, "01", "IN"}, /* India */
    {405, "025", "IN"}, /* India */
    {405, "026", "IN"}, /* India */
    {405, "027", "IN"}, /* India */
    {405, "028", "IN"}, /* India */
    {405, "029", "IN"}, /* India */
    {405, "030", "IN"}, /* India */
    {405, "031", "IN"}, /* India */
    {405, "032", "IN"}, /* India */
    {405, "033", "IN"}, /* India */
    {405, "034", "IN"}, /* India */
    {405, "035", "IN"}, /* India */
    {405, "036", "IN"}, /* India */
    {405, "037", "IN"}, /* India */
    {405, "038", "IN"}, /* India */
    {405, "039", "IN"}, /* India */
    {405, "03", "IN"}, /* India */
    {405, "041", "IN"}, /* India */
    {405, "042", "IN"}, /* India */
    {405, "043", "IN"}, /* India */
    {405, "044", "IN"}, /* India */
    {405, "045", "IN"}, /* India */
    {405, "046", "IN"}, /* India */
    {405, "047", "IN"}, /* India */
    {405, "04", "IN"}, /* India */
    {405, "05", "IN"}, /* India */
    {405, "06", "IN"}, /* India */
    {405, "07", "IN"}, /* India */
    {405, "08", "IN"}, /* India */
    {405, "09", "IN"}, /* India */
    {405, "10", "IN"}, /* India */
    {405, "11", "IN"}, /* India */
    {405, "12", "IN"}, /* India */
    {405, "13", "IN"}, /* India */
    {405, "14", "IN"}, /* India */
    {405, "15", "IN"}, /* India */
    {405, "17", "IN"}, /* India */
    {405, "18", "IN"}, /* India */
    {405, "19", "IN"}, /* India */
    {405, "20", "IN"}, /* India */
    {405, "21", "IN"}, /* India */
    {405, "22", "IN"}, /* India */
    {405, "23", "IN"}, /* India */
    {405, "51", "IN"}, /* India */
    {405, "52", "IN"}, /* India */
    {405, "53", "IN"}, /* India */
    {405, "54", "IN"}, /* India */
    {405, "55", "IN"}, /* India */
    {405, "56", "IN"}, /* India */
    {405, "66", "IN"}, /* India */
    {405, "67", "IN"}, /* India */
    {405, "70", "IN"}, /* India */
    {405, "750", "IN"}, /* India */
    {405, "751", "IN"}, /* India */
    {405, "752", "IN"}, /* India */
    {405, "753", "IN"}, /* India */
    {405, "754", "IN"}, /* India */
    {405, "755", "IN"}, /* India */
    {405, "756", "IN"}, /* India */
    {405, "799", "IN"}, /* India */
    {405, "800", "IN"}, /* India */
    {405, "801", "IN"}, /* India */
    {405, "802", "IN"}, /* India */
    {405, "803", "IN"}, /* India */
    {405, "804", "IN"}, /* India */
    {405, "805", "IN"}, /* India */
    {405, "806", "IN"}, /* India */
    {405, "807", "IN"}, /* India */
    {405, "808", "IN"}, /* India */
    {405, "809", "IN"}, /* India */
    {405, "810", "IN"}, /* India */
    {405, "811", "IN"}, /* India */
    {405, "812", "IN"}, /* India */
    {405, "818", "IN"}, /* India */
    {405, "819", "IN"}, /* India */
    {405, "820", "IN"}, /* India */
    {405, "821", "IN"}, /* India */
    {405, "822", "IN"}, /* India */
    {405, "824", "IN"}, /* India */
    {405, "827", "IN"}, /* India */
    {405, "834", "IN"}, /* India */
    {405, "840", "IN"}, /* India */
    {405, "844", "IN"}, /* India */
    {405, "845", "IN"}, /* India */
    {405, "846", "IN"}, /* India */
    {405, "847", "IN"}, /* India */
    {405, "848", "IN"}, /* India */
    {405, "849", "IN"}, /* India */
    {405, "850", "IN"}, /* India */
    {405, "851", "IN"}, /* India */
    {405, "852", "IN"}, /* India */
    {405, "853", "IN"}, /* India */
    {405, "854", "IN"}, /* India */
    {405, "855", "IN"}, /* India */
    {405, "856", "IN"}, /* India */
    {405, "857", "IN"}, /* India */
    {405, "858", "IN"}, /* India */
    {405, "859", "IN"}, /* India */
    {405, "860", "IN"}, /* India */
    {405, "861", "IN"}, /* India */
    {405, "862", "IN"}, /* India */
    {405, "863", "IN"}, /* India */
    {405, "864", "IN"}, /* India */
    {405, "865", "IN"}, /* India */
    {405, "866", "IN"}, /* India */
    {405, "867", "IN"}, /* India */
    {405, "868", "IN"}, /* India */
    {405, "869", "IN"}, /* India */
    {405, "870", "IN"}, /* India */
    {405, "871", "IN"}, /* India */
    {405, "872", "IN"}, /* India */
    {405, "873", "IN"}, /* India */
    {405, "874", "IN"}, /* India */
    {405, "875", "IN"}, /* India */
    {405, "880", "IN"}, /* India */
    {405, "881", "IN"}, /* India */
    {405, "908", "IN"}, /* India */
    {405, "909", "IN"}, /* India */
    {405, "910", "IN"}, /* India */
    {405, "911", "IN"}, /* India */
    {405, "912", "IN"}, /* India */
    {405, "913", "IN"}, /* India */
    {405, "914", "IN"}, /* India */
    {405, "917", "IN"}, /* India */
    {405, "927", "IN"}, /* India */
    {405, "929", "IN"}, /* India */
    {410, "01", "PK"}, /* Pakistan */
    {410, "02", "PK"}, /* Pakistan */
    {410, "03", "PK"}, /* Pakistan */
    {410, "04", "PK"}, /* Pakistan */
    {410, "05", "PK"}, /* Pakistan */
    {410, "06", "PK"}, /* Pakistan */
    {410, "07", "PK"}, /* Pakistan */
    {410, "08", "PK"}, /* Pakistan */
    {412, "01", "AF"}, /* Afghanistan */
    {412, "20", "AF"}, /* Afghanistan */
    {412, "40", "AF"}, /* Afghanistan */
    {412, "50", "AF"}, /* Afghanistan */
    {412, "55", "AF"}, /* Afghanistan */
    {412, "88", "AF"}, /* Afghanistan */
    {413, "01", "LK"}, /* Sri Lanka */
    {413, "02", "LK"}, /* Sri Lanka */
    {413, "03", "LK"}, /* Sri Lanka */
    {413, "04", "LK"}, /* Sri Lanka */
    {413, "05", "LK"}, /* Sri Lanka */
    {413, "08", "LK"}, /* Sri Lanka */
    {414, "00", "MM"}, /* Myanmar */
    {414, "01", "MM"}, /* Myanmar */
    {414, "02", "MM"}, /* Myanmar */
    {414, "03", "MM"}, /* Myanmar */
    {414, "04", "MM"}, /* Myanmar */
    {414, "05", "MM"}, /* Myanmar */
    {414, "06", "MM"}, /* Myanmar */
    {414, "09", "MM"}, /* Myanmar */
    {415, "01", "LB"}, /* Lebanon */
    {415, "03", "LB"}, /* Lebanon */
    {415, "05", "LB"}, /* Lebanon */
    {416, "01", "JO"}, /* Jordan */
    {416, "02", "JO"}, /* Jordan */
    {416, "03", "JO"}, /* Jordan */
    {416, "77", "JO"}, /* Jordan */
    {417, "01", "SY"}, /* Syria */
    {417, "02", "SY"}, /* Syria */
    {417, "09", "SY"}, /* Syria */
    {418, "00", "IQ"}, /* Iraq */
    {418, "05", "IQ"}, /* Iraq */
    {418, "08", "IQ"}, /* Iraq */
    {418, "20", "IQ"}, /* Iraq */
    {418, "30", "IQ"}, /* Iraq */
    {418, "40", "IQ"}, /* Iraq */
    {418, "45", "IQ"}, /* Iraq */
    {418, "62", "IQ"}, /* Iraq */
    {418, "92", "IQ"}, /* Iraq */
    {419, "02", "KW"}, /* Kuwait */
    {419, "03", "KW"}, /* Kuwait */
    {419, "04", "KW"}, /* Kuwait */
    {420, "01", "SA"}, /* Saudi Arabia */
    {420, "03", "SA"}, /* Saudi Arabia */
    {420, "04", "SA"}, /* Saudi Arabia */
    {420, "05", "SA"}, /* Saudi Arabia */
    {420, "21", "SA"}, /* Saudi Arabia */
    {421, "01", "YE"}, /* Yemen */
    {421, "02", "YE"}, /* Yemen */
    {421, "03", "YE"}, /* Yemen */
    {421, "04", "YE"}, /* Yemen */
    {422, "02", "OM"}, /* Oman */
    {422, "03", "OM"}, /* Oman */
    {422, "04", "OM"}, /* Oman */
    {424, "02", "AE"}, /* United Arab Emirates */
    {424, "03", "AE"}, /* United Arab Emirates */
    {425, "01", "IL"}, /* Israel */
    {425, "02", "IL"}, /* Israel */
    {425, "03", "IL"}, /* Israel */
    {425, "04", "IL"}, /* Israel */
    {425, "05", "IL"}, /* Israel */
    {425, "05", "PS"}, /* State of Palestine|Palestine */
    {425, "06", "IL"}, /* Israel */
    {425, "06", "PS"}, /* State of Palestine|Palestine */
    {425, "07", "IL"}, /* Israel */
    {425, "08", "IL"}, /* Israel */
    {425, "09", "IL"}, /* Israel */
    {425, "11", "IL"}, /* Israel */
    {425, "12", "IL"}, /* Israel */
    {425, "13", "IL"}, /* Israel */
    {425, "14", "IL"}, /* Israel */
    {425, "15", "IL"}, /* Israel */
    {425, "16", "IL"}, /* Israel */
    {425, "17", "IL"}, /* Israel */
    {425, "18", "IL"}, /* Israel */
    {425, "19", "IL"}, /* Israel */
    {425, "20", "IL"}, /* Israel */
    {425, "21", "IL"}, /* Israel */
    {425, "23", "IL"}, /* Israel */
    {425, "24", "IL"}, /* Israel */
    {425, "25", "IL"}, /* Israel */
    {425, "28", "IL"}, /* Israel */
    {426, "02", "BH"}, /* Bahrain */
    {426, "03", "BH"}, /* Bahrain */
    {426, "04", "BH"}, /* Bahrain */
    {426, "05", "BH"}, /* Bahrain */
    {427, "01", "QA"}, /* Qatar */
    {427, "02", "QA"}, /* Qatar */
    {427, "05", "QA"}, /* Qatar */
    {427, "06", "QA"}, /* Qatar */
    {428, "88", "MN"}, /* Mongolia */
    {428, "91", "MN"}, /* Mongolia */
    {428, "98", "MN"}, /* Mongolia */
    {428, "99", "MN"}, /* Mongolia */
    {429, "01", "NP"}, /* Nepal */
    {429, "02", "NP"}, /* Nepal */
    {429, "03", "NP"}, /* Nepal */
    {429, "04", "NP"}, /* Nepal */
    {432, "11", "IR"}, /* Iran */
    {432, "12", "IR"}, /* Iran */
    {432, "14", "IR"}, /* Iran */
    {432, "19", "IR"}, /* Iran */
    {432, "20", "IR"}, /* Iran */
    {432, "32", "IR"}, /* Iran */
    {432, "70", "IR"}, /* Iran */
    {432, "93", "IR"}, /* Iran */
    {434, "01", "UZ"}, /* Uzbekistan */
    {434, "02", "UZ"}, /* Uzbekistan */
    {434, "03", "UZ"}, /* Uzbekistan */
    {434, "04", "UZ"}, /* Uzbekistan */
    {434, "06", "UZ"}, /* Uzbekistan */
    {434, "07", "UZ"}, /* Uzbekistan */
    {434, "08", "UZ"}, /* Uzbekistan */
    {436, "01", "TJ"}, /* Tajikistan */
    {436, "02", "TJ"}, /* Tajikistan */
    {436, "03", "TJ"}, /* Tajikistan */
    {436, "04", "TJ"}, /* Tajikistan */
    {436, "05", "TJ"}, /* Tajikistan */
    {436, "10", "TJ"}, /* Tajikistan */
    {436, "12", "TJ"}, /* Tajikistan */
    {437, "01", "KG"}, /* Kyrgyzstan */
    {437, "03", "KG"}, /* Kyrgyzstan */
    {437, "05", "KG"}, /* Kyrgyzstan */
    {437, "09", "KG"}, /* Kyrgyzstan */
    {438, "01", "TM"}, /* Turkmenistan */
    {438, "02", "TM"}, /* Turkmenistan */
    {438, "03", "TM"}, /* Turkmenistan */
    {440, "00", "JP"}, /* Japan */
    {440, "01", "JP"}, /* Japan */
    {440, "02", "JP"}, /* Japan */
    {440, "03", "JP"}, /* Japan */
    {440, "05", "JP"}, /* Japan */
    {440, "10", "JP"}, /* Japan */
    {440, "20", "JP"}, /* Japan */
    {440, "21", "JP"}, /* Japan */
    {440, "50", "JP"}, /* Japan */
    {440, "51", "JP"}, /* Japan */
    {440, "52", "JP"}, /* Japan */
    {440, "53", "JP"}, /* Japan */
    {440, "54", "JP"}, /* Japan */
    {440, "70", "JP"}, /* Japan */
    {440, "71", "JP"}, /* Japan */
    {440, "72", "JP"}, /* Japan */
    {440, "73", "JP"}, /* Japan */
    {440, "74", "JP"}, /* Japan */
    {440, "75", "JP"}, /* Japan */
    {440, "76", "JP"}, /* Japan */
    {440, "78", "JP"}, /* Japan */
    {441, "00", "JP"}, /* Japan */
    {441, "01", "JP"}, /* Japan */
    {441, "10", "JP"}, /* Japan */
    {450, "01", "KR"}, /* South Korea */
    {450, "02", "KR"}, /* South Korea */
    {450, "03", "KR"}, /* South Korea */
    {450, "04", "KR"}, /* South Korea */
    {450, "05", "KR"}, /* South Korea */
    {450, "06", "KR"}, /* South Korea */
    {450, "07", "KR"}, /* South Korea */
    {450, "08", "KR"}, /* South Korea */
    {450, "11", "KR"}, /* South Korea */
    {450, "12", "KR"}, /* South Korea */
    {452, "01", "VN"}, /* Vietnam */
    {452, "02", "VN"}, /* Vietnam */
    {452, "03", "VN"}, /* Vietnam */
    {452, "04", "VN"}, /* Vietnam */
    {452, "05", "VN"}, /* Vietnam */
    {452, "06", "VN"}, /* Vietnam */
    {452, "07", "VN"}, /* Vietnam */
    {452, "08", "VN"}, /* Vietnam */
    {454, "00", "HK"}, /* Hong Kong */
    {454, "01", "HK"}, /* Hong Kong */
    {454, "02", "HK"}, /* Hong Kong */
    {454, "03", "HK"}, /* Hong Kong */
    {454, "04", "HK"}, /* Hong Kong */
    {454, "05", "HK"}, /* Hong Kong */
    {454, "06", "HK"}, /* Hong Kong */
    {454, "07", "HK"}, /* Hong Kong */
    {454, "08", "HK"}, /* Hong Kong */
    {454, "09", "HK"}, /* Hong Kong */
    {454, "10", "HK"}, /* Hong Kong */
    {454, "11", "HK"}, /* Hong Kong */
    {454, "12", "HK"}, /* Hong Kong */
    {454, "13", "HK"}, /* Hong Kong */
    {454, "14", "HK"}, /* Hong Kong */
    {454, "15", "HK"}, /* Hong Kong */
    {454, "16", "HK"}, /* Hong Kong */
    {454, "17", "HK"}, /* Hong Kong */
    {454, "18", "HK"}, /* Hong Kong */
    {454, "19", "HK"}, /* Hong Kong */
    {454, "20", "HK"}, /* Hong Kong */
    {454, "21", "HK"}, /* Hong Kong */
    {454, "22", "HK"}, /* Hong Kong */
    {454, "23", "HK"}, /* Hong Kong */
    {454, "24", "HK"}, /* Hong Kong */
    {454, "25", "HK"}, /* Hong Kong */
    {454, "26", "HK"}, /* Hong Kong */
    {454, "29", "HK"}, /* Hong Kong */
    {454, "30", "HK"}, /* Hong Kong */
    {454, "31", "HK"}, /* Hong Kong */
    {454, "32", "HK"}, /* Hong Kong */
    {455, "00", "MO"}, /* Macau (People's Republic of China) */
    {455, "01", "MO"}, /* Macau (People's Republic of China) */
    {455, "02", "MO"}, /* Macau (People's Republic of China) */
    {455, "03", "MO"}, /* Macau (People's Republic of China) */
    {455, "04", "MO"}, /* Macau (People's Republic of China) */
    {455, "05", "MO"}, /* Macau (People's Republic of China) */
    {455, "06", "MO"}, /* Macau (People's Republic of China) */
    {455, "07", "MO"}, /* Macau (People's Republic of China) */
    {456, "01", "KH"}, /* Cambodia */
    {456, "02", "KH"}, /* Cambodia */
    {456, "03", "KH"}, /* Cambodia */
    {456, "04", "KH"}, /* Cambodia */
    {456, "05", "KH"}, /* Cambodia */
    {456, "06", "KH"}, /* Cambodia */
    {456, "08", "KH"}, /* Cambodia */
    {456, "09", "KH"}, /* Cambodia */
    {456, "11", "KH"}, /* Cambodia */
    {456, "18", "KH"}, /* Cambodia */
    {457, "01", "LA"}, /* Laos */
    {457, "02", "LA"}, /* Laos */
    {457, "03", "LA"}, /* Laos */
    {457, "08", "LA"}, /* Laos */
    {460, "00", "CN"}, /* China */
    {460, "01", "CN"}, /* China */
    {460, "02", "CN"}, /* China */
    {460, "03", "CN"}, /* China */
    {460, "04", "CN"}, /* China */
    {460, "05", "CN"}, /* China */
    {460, "06", "CN"}, /* China */
    {460, "07", "CN"}, /* China */
    {460, "08", "CN"}, /* China */
    {460, "09", "CN"}, /* China */
    {460, "11", "CN"}, /* China */
    {460, "20", "CN"}, /* China */
    {466, "01", "TW"}, /* Taiwan */
    {466, "02", "TW"}, /* Taiwan */
    {466, "03", "TW"}, /* Taiwan */
    {466, "05", "TW"}, /* Taiwan */
    {466, "06", "TW"}, /* Taiwan */
    {466, "07", "TW"}, /* Taiwan */
    {466, "09", "TW"}, /* Taiwan */
    {466, "10", "TW"}, /* Taiwan */
    {466, "11", "TW"}, /* Taiwan */
    {466, "12", "TW"}, /* Taiwan */
    {466, "56", "TW"}, /* Taiwan */
    {466, "68", "TW"}, /* Taiwan */
    {466, "88", "TW"}, /* Taiwan */
    {466, "89", "TW"}, /* Taiwan */
    {466, "90", "TW"}, /* Taiwan */
    {466, "92", "TW"}, /* Taiwan */
    {466, "93", "TW"}, /* Taiwan */
    {466, "97", "TW"}, /* Taiwan */
    {466, "99", "TW"}, /* Taiwan */
    {467, "05", "KP"}, /* North Korea */
    {467, "06", "KP"}, /* North Korea */
    {467, "193", "KP"}, /* North Korea */
    {470, "01", "BD"}, /* Bangladesh */
    {470, "02", "BD"}, /* Bangladesh */
    {470, "03", "BD"}, /* Bangladesh */
    {470, "04", "BD"}, /* Bangladesh */
    {470, "05", "BD"}, /* Bangladesh */
    {470, "07", "BD"}, /* Bangladesh */
    {470, "09", "BD"}, /* Bangladesh */
    {472, "01", "MV"}, /* Maldives */
    {472, "02", "MV"}, /* Maldives */
    {502, "01", "MY"}, /* Malaysia */
    {502, "10", "MY"}, /* Malaysia */
    {502, "11", "MY"}, /* Malaysia */
    {502, "12", "MY"}, /* Malaysia */
    {502, "13", "MY"}, /* Malaysia */
    {502, "14", "MY"}, /* Malaysia */
    {502, "150", "MY"}, /* Malaysia */
    {502, "152", "MY"}, /* Malaysia */
    {502, "153", "MY"}, /* Malaysia */
    {502, "154", "MY"}, /* Malaysia */
    {502, "155", "MY"}, /* Malaysia */
    {502, "156", "MY"}, /* Malaysia */
    {502, "157", "MY"}, /* Malaysia */
    {502, "16", "MY"}, /* Malaysia */
    {502, "17", "MY"}, /* Malaysia */
    {502, "18", "MY"}, /* Malaysia */
    {502, "19", "MY"}, /* Malaysia */
    {502, "20", "MY"}, /* Malaysia */
    {505, "01", "AU"}, /* Australia */
    {505, "02", "AU"}, /* Australia */
    {505, "03", "AU"}, /* Australia */
    {505, "04", "AU"}, /* Australia */
    {505, "05", "AU"}, /* Australia */
    {505, "06", "AU"}, /* Australia */
    {505, "07", "AU"}, /* Australia */
    {505, "08", "AU"}, /* Australia */
    {505, "09", "AU"}, /* Australia */
    {505, "10", "AU"}, /* Australia */
    {505, "10", "NF"}, /* Norfolk Island */
    {505, "11", "AU"}, /* Australia */
    {505, "12", "AU"}, /* Australia */
    {505, "14", "AU"}, /* Australia */
    {505, "15", "AU"}, /* Australia */
    {505, "16", "AU"}, /* Australia */
    {505, "17", "AU"}, /* Australia */
    {505, "18", "AU"}, /* Australia */
    {505, "19", "AU"}, /* Australia */
    {505, "20", "AU"}, /* Australia */
    {505, "21", "AU"}, /* Australia */
    {505, "22", "AU"}, /* Australia */
    {505, "23", "AU"}, /* Australia */
    {505, "24", "AU"}, /* Australia */
    {505, "25", "AU"}, /* Australia */
    {505, "26", "AU"}, /* Australia */
    {505, "27", "AU"}, /* Australia */
    {505, "28", "AU"}, /* Australia */
    {505, "30", "AU"}, /* Australia */
    {505, "31", "AU"}, /* Australia */
    {505, "32", "AU"}, /* Australia */
    {505, "33", "AU"}, /* Australia */
    {505, "34", "AU"}, /* Australia */
    {505, "35", "AU"}, /* Australia */
    {505, "36", "AU"}, /* Australia */
    {505, "37", "AU"}, /* Australia */
    {505, "38", "AU"}, /* Australia */
    {505, "39", "AU"}, /* Australia */
    {505, "40", "AU"}, /* Australia */
    {505, "41", "AU"}, /* Australia */
    {505, "42", "AU"}, /* Australia */
    {505, "61", "AU"}, /* Australia */
    {505, "62", "AU"}, /* Australia */
    {505, "68", "AU"}, /* Australia */
    {505, "71", "AU"}, /* Australia */
    {505, "72", "AU"}, /* Australia */
    {505, "88", "AU"}, /* Australia */
    {505, "90", "AU"}, /* Australia */
    {505, "99", "AU"}, /* Australia */
    {510, "00", "ID"}, /* Indonesia */
    {510, "01", "ID"}, /* Indonesia */
    {510, "03", "ID"}, /* Indonesia */
    {510, "07", "ID"}, /* Indonesia */
    {510, "08", "ID"}, /* Indonesia */
    {510, "09", "ID"}, /* Indonesia */
    {510, "10", "ID"}, /* Indonesia */
    {510, "11", "ID"}, /* Indonesia */
    {510, "20", "ID"}, /* Indonesia */
    {510, "21", "ID"}, /* Indonesia */
    {510, "27", "ID"}, /* Indonesia */
    {510, "28", "ID"}, /* Indonesia */
    {510, "88", "ID"}, /* Indonesia */
    {510, "89", "ID"}, /* Indonesia */
    {510, "99", "ID"}, /* Indonesia */
    {514, "02", "TL"}, /* East Timor */
    {514, "03", "TL"}, /* East Timor */
    {515, "01", "PH"}, /* Philippines */
    {515, "02", "PH"}, /* Philippines */
    {515, "03", "PH"}, /* Philippines */
    {515, "05", "PH"}, /* Philippines */
    {515, "11", "PH"}, /* Philippines */
    {515, "18", "PH"}, /* Philippines */
    {515, "24", "PH"}, /* Philippines */
    {515, "88", "PH"}, /* Philippines */
    {520, "00", "TH"}, /* Thailand */
    {520, "01", "TH"}, /* Thailand */
    {520, "02", "TH"}, /* Thailand */
    {520, "03", "TH"}, /* Thailand */
    {520, "04", "TH"}, /* Thailand */
    {520, "05", "TH"}, /* Thailand */
    {520, "15", "TH"}, /* Thailand */
    {520, "18", "TH"}, /* Thailand */
    {520, "20", "TH"}, /* Thailand */
    {520, "23", "TH"}, /* Thailand */
    {520, "25", "TH"}, /* Thailand */
    {520, "47", "TH"}, /* Thailand */
    {520, "99", "TH"}, /* Thailand */
    {525, "02", "SG"}, /* Singapore */
    {525, "03", "SG"}, /* Singapore */
    {525, "05", "SG"}, /* Singapore */
    {525, "06", "SG"}, /* Singapore */
    {525, "07", "SG"}, /* Singapore */
    {525, "08", "SG"}, /* Singapore */
    {525, "09", "SG"}, /* Singapore */
    {525, "12", "SG"}, /* Singapore */
    {528, "01", "BN"}, /* Brunei */
    {528, "02", "BN"}, /* Brunei */
    {528, "11", "BN"}, /* Brunei */
    {530, "00", "NZ"}, /* New Zealand */
    {530, "01", "NZ"}, /* New Zealand */
    {530, "02", "NZ"}, /* New Zealand */
    {530, "03", "NZ"}, /* New Zealand */
    {530, "04", "NZ"}, /* New Zealand */
    {530, "05", "NZ"}, /* New Zealand */
    {530, "06", "NZ"}, /* New Zealand */
    {530, "07", "NZ"}, /* New Zealand */
    {530, "24", "NZ"}, /* New Zealand */
    {536, "02", "NR"}, /* Nauru */
    {537, "01", "PG"}, /* Papua New Guinea */
    {537, "02", "PG"}, /* Papua New Guinea */
    {537, "03", "PG"}, /* Papua New Guinea */
    {539, "01", "TO"}, /* Tonga */
    {539, "43", "TO"}, /* Tonga */
    {539, "88", "TO"}, /* Tonga */
    {540, "02", "SB"}, /* Solomon Islands */
    {541, "00", "VU"}, /* Vanuatu */
    {541, "01", "VU"}, /* Vanuatu */
    {541, "05", "VU"}, /* Vanuatu */
    {541, "07", "VU"}, /* Vanuatu */
    {542, "01", "FJ"}, /* Fiji */
    {542, "02", "FJ"}, /* Fiji */
    {542, "03", "FJ"}, /* Fiji */
    {543, "01", "WF"}, /* Wallis and Futuna */
    {545, "01", "KI"}, /* Kiribati */
    {545, "02", "KI"}, /* Kiribati */
    {545, "09", "KI"}, /* Kiribati */
    {546, "01", "NC"}, /* New Caledonia (France) */
    {547, "05", "PF"}, /* French Polynesia (France) */
    {547, "10", "PF"}, /* French Polynesia (France) */
    {547, "15", "PF"}, /* French Polynesia (France) */
    {547, "20", "PF"}, /* French Polynesia (France) */
    {548, "01", "CK"}, /* Cook Islands (Pacific Ocean) */
    {549, "01", "WS"}, /* Samoa */
    {550, "01", "FM"}, /* Federated States of Micronesia */
    {551, "01", "MH"}, /* Marshall Islands */
    {552, "01", "PW"}, /* Palau */
    {552, "80", "PW"}, /* Palau */
    {553, "01", "TV"}, /* Tuvalu */
    {554, "01", "TK"}, /* Tokelau */
    {555, "01", "NU"}, /* Niue */
    {602, "01", "EG"}, /* Egypt */
    {602, "02", "EG"}, /* Egypt */
    {602, "03", "EG"}, /* Egypt */
    {603, "01", "DZ"}, /* Algeria */
    {603, "02", "DZ"}, /* Algeria */
    {603, "03", "DZ"}, /* Algeria */
    {603, "07", "DZ"}, /* Algeria */
    {603, "09", "DZ"}, /* Algeria */
    {603, "21", "DZ"}, /* Algeria */
    {604, "00", "MA"}, /* Morocco */
    {604, "01", "MA"}, /* Morocco */
    {604, "02", "MA"}, /* Morocco */
    {604, "05", "MA"}, /* Morocco */
    {605, "01", "TN"}, /* Tunisia */
    {605, "02", "TN"}, /* Tunisia */
    {605, "03", "TN"}, /* Tunisia */
    {606, "00", "LY"}, /* Libya */
    {606, "01", "LY"}, /* Libya */
    {606, "02", "LY"}, /* Libya */
    {606, "03", "LY"}, /* Libya */
    {606, "06", "LY"}, /* Libya */
    {607, "01", "GM"}, /* Gambia */
    {607, "02", "GM"}, /* Gambia */
    {607, "03", "GM"}, /* Gambia */
    {607, "04", "GM"}, /* Gambia */
    {607, "05", "GM"}, /* Gambia */
    {607, "06", "GM"}, /* Gambia */
    {608, "01", "SN"}, /* Senegal */
    {608, "02", "SN"}, /* Senegal */
    {608, "03", "SN"}, /* Senegal */
    {608, "04", "SN"}, /* Senegal */
    {609, "01", "MR"}, /* Mauritania */
    {609, "02", "MR"}, /* Mauritania */
    {609, "10", "MR"}, /* Mauritania */
    {610, "01", "ML"}, /* Mali */
    {610, "02", "ML"}, /* Mali */
    {611, "02", "GN"}, /* Guinea */
    {611, "03", "GN"}, /* Guinea */
    {611, "04", "GN"}, /* Guinea */
    {611, "05", "GN"}, /* Guinea */
    {612, "01", "CI"}, /* Ivory Coast */
    {612, "02", "CI"}, /* Ivory Coast */
    {612, "03", "CI"}, /* Ivory Coast */
    {612, "04", "CI"}, /* Ivory Coast */
    {612, "05", "CI"}, /* Ivory Coast */
    {612, "06", "CI"}, /* Ivory Coast */
    {612, "07", "CI"}, /* Ivory Coast */
    {612, "18", "CI"}, /* Ivory Coast */
    {613, "01", "BF"}, /* Burkina Faso */
    {613, "02", "BF"}, /* Burkina Faso */
    {613, "03", "BF"}, /* Burkina Faso */
    {614, "01", "NE"}, /* Niger */
    {614, "02", "NE"}, /* Niger */
    {614, "03", "NE"}, /* Niger */
    {614, "04", "NE"}, /* Niger */
    {615, "01", "TG"}, /* Togo */
    {615, "03", "TG"}, /* Togo */
    {616, "01", "BJ"}, /* Benin */
    {616, "02", "BJ"}, /* Benin */
    {616, "04", "BJ"}, /* Benin */
    {616, "05", "BJ"}, /* Benin */
    {617, "01", "MU"}, /* Mauritius */
    {617, "02", "MU"}, /* Mauritius */
    {617, "03", "MU"}, /* Mauritius */
    {617, "10", "MU"}, /* Mauritius */
    {618, "01", "LR"}, /* Liberia */
    {618, "02", "LR"}, /* Liberia */
    {618, "04", "LR"}, /* Liberia */
    {618, "07", "LR"}, /* Liberia */
    {618, "20", "LR"}, /* Liberia */
    {619, "01", "SL"}, /* Sierra Leone */
    {619, "02", "SL"}, /* Sierra Leone */
    {619, "03", "SL"}, /* Sierra Leone */
    {619, "04", "SL"}, /* Sierra Leone */
    {619, "05", "SL"}, /* Sierra Leone */
    {619, "06", "SL"}, /* Sierra Leone */
    {619, "09", "SL"}, /* Sierra Leone */
    {619, "25", "SL"}, /* Sierra Leone */
    {619, "40", "SL"}, /* Sierra Leone */
    {619, "50", "SL"}, /* Sierra Leone */
    {620, "01", "GH"}, /* Ghana */
    {620, "02", "GH"}, /* Ghana */
    {620, "03", "GH"}, /* Ghana */
    {620, "04", "GH"}, /* Ghana */
    {620, "06", "GH"}, /* Ghana */
    {620, "07", "GH"}, /* Ghana */
    {620, "08", "GH"}, /* Ghana */
    {620, "10", "GH"}, /* Ghana */
    {620, "11", "GH"}, /* Ghana */
    {621, "00", "NG"}, /* Nigeria */
    {621, "20", "NG"}, /* Nigeria */
    {621, "22", "NG"}, /* Nigeria */
    {621, "24", "NG"}, /* Nigeria */
    {621, "25", "NG"}, /* Nigeria */
    {621, "26", "NG"}, /* Nigeria */
    {621, "27", "NG"}, /* Nigeria */
    {621, "30", "NG"}, /* Nigeria */
    {621, "40", "NG"}, /* Nigeria */
    {621, "50", "NG"}, /* Nigeria */
    {621, "60", "NG"}, /* Nigeria */
    {622, "02", "TD"}, /* Chad */
    {622, "03", "TD"}, /* Chad */
    {622, "07", "TD"}, /* Chad */
    {623, "01", "CF"}, /* Central African Republic */
    {623, "02", "CF"}, /* Central African Republic */
    {623, "03", "CF"}, /* Central African Republic */
    {623, "04", "CF"}, /* Central African Republic */
    {624, "01", "CM"}, /* Cameroon */
    {624, "02", "CM"}, /* Cameroon */
    {624, "04", "CM"}, /* Cameroon */
    {625, "01", "CV"}, /* Cape Verde */
    {625, "02", "CV"}, /* Cape Verde */
    {626, "01", "ST"}, /* Sao Tome and Principe */
    {626, "02", "ST"}, /* Sao Tome and Principe */
    {627, "01", "GQ"}, /* Equatorial Guinea */
    {627, "03", "GQ"}, /* Equatorial Guinea */
    {628, "01", "GA"}, /* Gabon */
    {628, "02", "GA"}, /* Gabon */
    {628, "03", "GA"}, /* Gabon */
    {628, "04", "GA"}, /* Gabon */
    {628, "05", "GA"}, /* Gabon */
    {629, "01", "CG"}, /* Republic of the Congo|Congo */
    {629, "07", "CG"}, /* Republic of the Congo|Congo */
    {629, "10", "CG"}, /* Republic of the Congo|Congo */
    {630, "01", "CD"}, /* Democratic Republic of the Congo */
    {630, "02", "CD"}, /* Democratic Republic of the Congo */
    {630, "04", "CD"}, /* Democratic Republic of the Congo */
    {630, "05", "CD"}, /* Democratic Republic of the Congo */
    {630, "10", "CD"}, /* Democratic Republic of the Congo */
    {630, "86", "CD"}, /* Democratic Republic of the Congo */
    {630, "88", "CD"}, /* Democratic Republic of the Congo */
    {630, "89", "CD"}, /* Democratic Republic of the Congo */
    {630, "90", "CD"}, /* Democratic Republic of the Congo */
    {631, "04", "AO"}, /* Angola */
    {632, "01", "GW"}, /* Guinea-Bissau */
    {632, "03", "GW"}, /* Guinea-Bissau */
    {632, "07", "GW"}, /* Guinea-Bissau */
    {633, "01", "SC"}, /* Seychelles */
    {633, "02", "SC"}, /* Seychelles */
    {633, "10", "SC"}, /* Seychelles */
    {634, "01", "SD"}, /* Sudan */
    {634, "02", "SD"}, /* Sudan */
    {634, "03", "SD"}, /* Sudan */
    {634, "05", "SD"}, /* Sudan */
    {634, "07", "SD"}, /* Sudan */
    {634, "09", "SD"}, /* Sudan */
    {635, "10", "RW"}, /* Rwanda */
    {635, "11", "RW"}, /* Rwanda */
    {635, "12", "RW"}, /* Rwanda */
    {635, "13", "RW"}, /* Rwanda */
    {635, "14", "RW"}, /* Rwanda */
    {635, "17", "RW"}, /* Rwanda */
    {636, "01", "ET"}, /* Ethiopia */
    {637, "01", "SO"}, /* Somalia */
    {637, "04", "SO"}, /* Somalia */
    {637, "10", "SO"}, /* Somalia */
    {637, "30", "SO"}, /* Somalia */
    {637, "50", "SO"}, /* Somalia */
    {637, "57", "SO"}, /* Somalia */
    {637, "60", "SO"}, /* Somalia */
    {637, "67", "SO"}, /* Somalia */
    {637, "70", "SO"}, /* Somalia */
    {637, "71", "SO"}, /* Somalia */
    {637, "82", "SO"}, /* Somalia */
    {638, "01", "DJ"}, /* Djibouti */
    {639, "02", "KE"}, /* Kenya */
    {639, "03", "KE"}, /* Kenya */
    {639, "05", "KE"}, /* Kenya */
    {639, "07", "KE"}, /* Kenya */
    {640, "01", "TZ"}, /* Tanzania */
    {640, "02", "TZ"}, /* Tanzania */
    {640, "03", "TZ"}, /* Tanzania */
    {640, "04", "TZ"}, /* Tanzania */
    {640, "05", "TZ"}, /* Tanzania */
    {640, "06", "TZ"}, /* Tanzania */
    {640, "07", "TZ"}, /* Tanzania */
    {640, "08", "TZ"}, /* Tanzania */
    {640, "09", "TZ"}, /* Tanzania */
    {640, "11", "TZ"}, /* Tanzania */
    {640, "12", "TZ"}, /* Tanzania */
    {640, "13", "TZ"}, /* Tanzania */
    {641, "01", "UG"}, /* Uganda */
    {641, "04", "UG"}, /* Uganda */
    {641, "06", "UG"}, /* Uganda */
    {641, "10", "UG"}, /* Uganda */
    {641, "11", "UG"}, /* Uganda */
    {641, "14", "UG"}, /* Uganda */
    {641, "16", "UG"}, /* Uganda */
    {641, "18", "UG"}, /* Uganda */
    {641, "22", "UG"}, /* Uganda */
    {641, "26", "UG"}, /* Uganda */
    {641, "30", "UG"}, /* Uganda */
    {641, "33", "UG"}, /* Uganda */
    {641, "40", "UG"}, /* Uganda */
    {641, "44", "UG"}, /* Uganda */
    {641, "66", "UG"}, /* Uganda */
    {642, "02", "BI"}, /* Burundi */
    {642, "03", "BI"}, /* Burundi */
    {642, "07", "BI"}, /* Burundi */
    {642, "08", "BI"}, /* Burundi */
    {642, "82", "BI"}, /* Burundi */
    {643, "01", "MZ"}, /* Mozambique */
    {643, "04", "MZ"}, /* Mozambique */
    {645, "01", "ZM"}, /* Zambia */
    {645, "03", "ZM"}, /* Zambia */
    {646, "01", "MG"}, /* Madagascar */
    {646, "02", "MG"}, /* Madagascar */
    {646, "03", "MG"}, /* Madagascar */
    {646, "04", "MG"}, /* Madagascar */
    {647, "00", "YT"}, /* French Indian Ocean Territories (France) */
    {647, "01", "YT"}, /* French Indian Ocean Territories (France) */
    {647, "02", "YT"}, /* French Indian Ocean Territories (France) */
    {647, "03", "YT"}, /* French Indian Ocean Territories (France) */
    {647, "10", "YT"}, /* French Indian Ocean Territories (France) */
    {648, "01", "ZW"}, /* Zimbabwe */
    {648, "03", "ZW"}, /* Zimbabwe */
    {648, "04", "ZW"}, /* Zimbabwe */
    {649, "01", "NA"}, /* Namibia */
    {649, "02", "NA"}, /* Namibia */
    {649, "03", "NA"}, /* Namibia */
    {649, "04", "NA"}, /* Namibia */
    {649, "05", "NA"}, /* Namibia */
    {650, "01", "MW"}, /* Malawi */
    {650, "02", "MW"}, /* Malawi */
    {650, "10", "MW"}, /* Malawi */
    {651, "01", "LS"}, /* Lesotho */
    {651, "02", "LS"}, /* Lesotho */
    {652, "01", "BW"}, /* Botswana */
    {652, "04", "BW"}, /* Botswana */
    {653, "01", "SZ"}, /* Swaziland */
    {653, "10", "SZ"}, /* Swaziland */
    {654, "01", "KM"}, /* Comoros */
    {654, "02", "KM"}, /* Comoros */
    {655, "01", "ZA"}, /* South Africa */
    {655, "02", "ZA"}, /* South Africa */
    {655, "04", "ZA"}, /* South Africa */
    {655, "06", "ZA"}, /* South Africa */
    {655, "07", "ZA"}, /* South Africa */
    {655, "10", "ZA"}, /* South Africa */
    {655, "11", "ZA"}, /* South Africa */
    {655, "12", "ZA"}, /* South Africa */
    {655, "13", "ZA"}, /* South Africa */
    {655, "14", "ZA"}, /* South Africa */
    {655, "16", "ZA"}, /* South Africa */
    {655, "17", "ZA"}, /* South Africa */
    {655, "19", "ZA"}, /* South Africa */
    {655, "21", "ZA"}, /* South Africa */
    {655, "24", "ZA"}, /* South Africa */
    {655, "25", "ZA"}, /* South Africa */
    {655, "27", "ZA"}, /* South Africa */
    {655, "28", "ZA"}, /* South Africa */
    {655, "30", "ZA"}, /* South Africa */
    {655, "31", "ZA"}, /* South Africa */
    {655, "32", "ZA"}, /* South Africa */
    {655, "33", "ZA"}, /* South Africa */
    {655, "34", "ZA"}, /* South Africa */
    {655, "35", "ZA"}, /* South Africa */
    {655, "36", "ZA"}, /* South Africa */
    {655, "38", "ZA"}, /* South Africa */
    {655, "41", "ZA"}, /* South Africa */
    {655, "46", "ZA"}, /* South Africa */
    {655, "50", "ZA"}, /* South Africa */
    {655, "51", "ZA"}, /* South Africa */
    {655, "53", "ZA"}, /* South Africa */
    {655, "73", "ZA"}, /* South Africa */
    {655, "74", "ZA"}, /* South Africa */
    {657, "01", "ER"}, /* Eritrea */
    {658, "01", "BL"}, /* Saint Barthélemy (France) */
    {659, "03", "SS"}, /* South Sudan */
    {659, "04", "SS"}, /* South Sudan */
    {659, "06", "SS"}, /* South Sudan */
    {659, "07", "SS"}, /* South Sudan */
    {702, "67", "BZ"}, /* Belize */
    {702, "68", "BZ"}, /* Belize */
    {702, "69", "BZ"}, /* Belize */
    {702, "99", "BZ"}, /* Belize */
    {704, "01", "GT"}, /* Guatemala */
    {704, "02", "GT"}, /* Guatemala */
    {704, "03", "GT"}, /* Guatemala */
    {706, "01", "SV"}, /* El Salvador */
    {706, "02", "SV"}, /* El Salvador */
    {706, "03", "SV"}, /* El Salvador */
    {706, "04", "SV"}, /* El Salvador */
    {706, "05", "SV"}, /* El Salvador */
    {708, "002", "HN"}, /* Honduras */
    {708, "030", "HN"}, /* Honduras */
    {708, "040", "HN"}, /* Honduras */
    {710, "21", "NI"}, /* Nicaragua */
    {710, "30", "NI"}, /* Nicaragua */
    {710, "73", "NI"}, /* Nicaragua */
    {712, "01", "CR"}, /* Costa Rica */
    {712, "02", "CR"}, /* Costa Rica */
    {712, "03", "CR"}, /* Costa Rica */
    {712, "04", "CR"}, /* Costa Rica */
    {712, "20", "CR"}, /* Costa Rica */
    {714, "01", "PA"}, /* Panama */
    {714, "02", "PA"}, /* Panama */
    {714, "03", "PA"}, /* Panama */
    {714, "04", "PA"}, /* Panama */
    {716, "06", "PE"}, /* Peru */
    {716, "07", "PE"}, /* Peru */
    {716, "10", "PE"}, /* Peru */
    {716, "17", "PE"}, /* Peru */
    {722, "020", "AR"}, /* Argentina */
    {722, "034", "AR"}, /* Argentina */
    {722, "040", "AR"}, /* Argentina */
    {722, "070", "AR"}, /* Argentina */
    {722, "310", "AR"}, /* Argentina */
    {722, "320", "AR"}, /* Argentina */
    {722, "330", "AR"}, /* Argentina */
    {722, "341", "AR"}, /* Argentina */
    {722, "350", "AR"}, /* Argentina */
    {724, "00", "BR"}, /* Brazil */
    {724, "01", "BR"}, /* Brazil */
    {724, "02", "BR"}, /* Brazil */
    {724, "03", "BR"}, /* Brazil */
    {724, "04", "BR"}, /* Brazil */
    {724, "05", "BR"}, /* Brazil */
    {724, "06", "BR"}, /* Brazil */
    {724, "10", "BR"}, /* Brazil */
    {724, "11", "BR"}, /* Brazil */
    {724, "15", "BR"}, /* Brazil */
    {724, "16", "BR"}, /* Brazil */
    {724, "17", "BR"}, /* Brazil */
    {724, "18", "BR"}, /* Brazil */
    {724, "23", "BR"}, /* Brazil */
    {724, "24", "BR"}, /* Brazil */
    {724, "30", "BR"}, /* Brazil */
    {724, "31", "BR"}, /* Brazil */
    {724, "32", "BR"}, /* Brazil */
    {724, "33", "BR"}, /* Brazil */
    {724, "34", "BR"}, /* Brazil */
    {724, "35", "BR"}, /* Brazil */
    {724, "36", "BR"}, /* Brazil */
    {724, "37", "BR"}, /* Brazil */
    {724, "38", "BR"}, /* Brazil */
    {724, "39", "BR"}, /* Brazil */
    {724, "54", "BR"}, /* Brazil */
    {724, "99", "BR"}, /* Brazil */
    {730, "01", "CL"}, /* Chile */
    {730, "02", "CL"}, /* Chile */
    {730, "03", "CL"}, /* Chile */
    {730, "04", "CL"}, /* Chile */
    {730, "05", "CL"}, /* Chile */
    {730, "06", "CL"}, /* Chile */
    {730, "07", "CL"}, /* Chile */
    {730, "08", "CL"}, /* Chile */
    {730, "09", "CL"}, /* Chile */
    {730, "10", "CL"}, /* Chile */
    {730, "11", "CL"}, /* Chile */
    {730, "12", "CL"}, /* Chile */
    {730, "13", "CL"}, /* Chile */
    {730, "14", "CL"}, /* Chile */
    {730, "15", "CL"}, /* Chile */
    {730, "16", "CL"}, /* Chile */
    {730, "17", "CL"}, /* Chile */
    {730, "18", "CL"}, /* Chile */
    {730, "19", "CL"}, /* Chile */
    {730, "99", "CL"}, /* Chile */
    {732, "001", "CO"}, /* Colombia */
    {732, "002", "CO"}, /* Colombia */
    {732, "003", "CO"}, /* Colombia */
    {732, "004", "CO"}, /* Colombia */
    {732, "020", "CO"}, /* Colombia */
    {732, "099", "CO"}, /* Colombia */
    {732, "101", "CO"}, /* Colombia */
    {732, "102", "CO"}, /* Colombia */
    {732, "103", "CO"}, /* Colombia */
    {732, "111", "CO"}, /* Colombia */
    {732, "123", "CO"}, /* Colombia */
    {732, "130", "CO"}, /* Colombia */
    {732, "142", "CO"}, /* Colombia */
    {732, "154", "CO"}, /* Colombia */
    {732, "165", "CO"}, /* Colombia */
    {732, "176", "CO"}, /* Colombia */
    {732, "187", "CO"}, /* Colombia */
    {732, "199", "CO"}, /* Colombia */
    {734, "01", "VE"}, /* Venezuela */
    {734, "02", "VE"}, /* Venezuela */
    {734, "03", "VE"}, /* Venezuela */
    {734, "04", "VE"}, /* Venezuela */
    {734, "06", "VE"}, /* Venezuela */
    {736, "01", "BO"}, /* Bolivia */
    {736, "03", "BO"}, /* Bolivia */
    {738, "01", "GY"}, /* Guyana */
    {738, "02", "GY"}, /* Guyana */
    {738, "03", "GY"}, /* Guyana */
    {738, "05", "GY"}, /* Guyana */
    {740, "00", "EC"}, /* Ecuador */
    {740, "01", "EC"}, /* Ecuador */
    {740, "02", "EC"}, /* Ecuador */
    {740, "03", "EC"}, /* Ecuador */
    {744, "01", "PY"}, /* Paraguay */
    {744, "02", "PY"}, /* Paraguay */
    {744, "03", "PY"}, /* Paraguay */
    {744, "04", "PY"}, /* Paraguay */
    {744, "05", "PY"}, /* Paraguay */
    {744, "06", "PY"}, /* Paraguay */
    {746, "02", "SR"}, /* Suriname */
    {746, "03", "SR"}, /* Suriname */
    {746, "04", "SR"}, /* Suriname */
    {746, "05", "SR"}, /* Suriname */
    {748, "00", "UY"}, /* Uruguay */
    {748, "01", "UY"}, /* Uruguay */
    {748, "03", "UY"}, /* Uruguay */
    {748, "07", "UY"}, /* Uruguay */
    {748, "10", "UY"}, /* Uruguay */
    {750, "001", "FK"}, /* Falkland Islands (United Kingdom) */
    {995, "01", "IO"} /* British Indian Ocean Territory (United Kingdom) */
};

} // namespace ModemManager
#endif
