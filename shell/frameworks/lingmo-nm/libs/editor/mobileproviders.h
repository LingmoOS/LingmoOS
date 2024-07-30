/*
    SPDX-FileCopyrightText: 2010-2012 Lamarque Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LINGMO_NM_MOBILE_PROVIDERS_H
#define LINGMO_NM_MOBILE_PROVIDERS_H

#include "lingmonm_editor_export.h"

#include <QDomDocument>
#include <QHash>
#include <QStringList>
#include <QVariantMap>

#include <NetworkManagerQt/ConnectionSettings>

struct ProviderData {
    QStringList mccmncs;
    QString name;
};

class LINGMONM_EDITOR_EXPORT MobileProviders
{
public:
    static const QString ProvidersFile;

    enum ErrorCodes {
        Success,
        CountryCodesMissing,
        ProvidersMissing,
        ProvidersIsNull,
        ProvidersWrongFormat,
        ProvidersFormatNotSupported,
    };

    MobileProviders();
    ~MobileProviders();

    QStringList getCountryList() const;
    QString countryFromLocale() const;
    QString getCountryName(const QString &key) const
    {
        return mCountries.value(key);
    }
    QStringList getProvidersList(QString country, NetworkManager::ConnectionSettings::ConnectionType type);
    QStringList getApns(const QString &provider);
    QStringList getNetworkIds(const QString &provider);
    QVariantMap getApnInfo(const QString &apn);
    QVariantMap getCdmaInfo(const QString &provider);
    QStringList getProvidersFromMCCMNC(const QString &mccmnc);
    QString getGsmNumber() const
    {
        return QStringLiteral("*99#");
    }
    QString getCdmaNumber() const
    {
        return QStringLiteral("#777");
    }
    inline ErrorCodes getError()
    {
        return mError;
    }

private:
    ProviderData parseProvider(const QDomNode &providerNode);

    QHash<QString, QString> mCountries;
    QHash<QString, QString> mMccMncToName;
    QMap<QString, QDomNode> mProvidersGsm;
    QMap<QString, QDomNode> mProvidersCdma;
    QMap<QString, QDomNode> mApns;
    QStringList mNetworkIds;
    QDomDocument mDocProviders;
    QDomElement docElement;
    ErrorCodes mError;
    QString getNameByLocale(const QMap<QString, QString> &names) const;
};

#endif // LINGMO_NM_MOBILE_PROVIDERS_H
