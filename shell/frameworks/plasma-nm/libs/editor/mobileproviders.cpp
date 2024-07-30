/*
    SPDX-FileCopyrightText: 2010-2012 Lamarque Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mobileproviders.h"
#include "plasma_nm_editor.h"

#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QTextStream>

const QString MobileProviders::ProvidersFile = QStringLiteral(BROADBANDPROVIDER_DATABASE);

bool localeAwareCompare(const QString &one, const QString &two)
{
    return one.localeAwareCompare(two) < 0;
}

MobileProviders::MobileProviders()
{
    for (int c = 1; c <= QLocale::LastCountry; c++) {
        const auto country = static_cast<QLocale::Country>(c);
        QLocale locale(QLocale::AnyLanguage, country);
        if (locale.country() == country) {
            const QString localeName = locale.name();
            const auto idx = localeName.indexOf(QLatin1Char('_'));
            if (idx != -1) {
                const QString countryCode = localeName.mid(idx + 1);
                QString countryName = locale.nativeCountryName();
                if (countryName.isEmpty()) {
                    countryName = QLocale::countryToString(country);
                }
                mCountries.insert(countryCode, countryName);
            }
        }
    }
    mError = Success;

    QFile file2(ProvidersFile);

    if (file2.open(QIODevice::ReadOnly)) {
        if (mDocProviders.setContent(&file2)) {
            docElement = mDocProviders.documentElement();

            if (docElement.isNull()) {
                qCWarning(PLASMA_NM_EDITOR_LOG) << ProvidersFile << ": document is null";
                mError = ProvidersIsNull;
            } else {
                if (docElement.isNull() || docElement.tagName() != QLatin1String("serviceproviders")) {
                    qCWarning(PLASMA_NM_EDITOR_LOG) << ProvidersFile << ": wrong format";
                    mError = ProvidersWrongFormat;
                } else {
                    if (docElement.attribute(QStringLiteral("format")) != QLatin1String("2.0")) {
                        qCWarning(PLASMA_NM_EDITOR_LOG) << ProvidersFile << ": mobile broadband provider database format '"
                                                        << docElement.attribute(QStringLiteral("format")) << "' not supported.";
                        mError = ProvidersFormatNotSupported;
                    } else {
                        // qCDebug(PLASMA_NM_EDITOR_LOG) << "Everything is alright so far";
                    }
                }
            }
        }

        file2.close();
    } else {
        qCWarning(PLASMA_NM_EDITOR_LOG) << "Error opening providers file" << ProvidersFile;
        mError = ProvidersMissing;
    }
}

MobileProviders::~MobileProviders() = default;

QStringList MobileProviders::getCountryList() const
{
    QStringList temp = mCountries.values();
    std::sort(temp.begin(), temp.end(), localeAwareCompare);
    return temp;
}

QString MobileProviders::countryFromLocale() const
{
    const QString localeName = QLocale().name();
    const auto idx = localeName.indexOf(QLatin1Char('_'));
    if (idx != -1) {
        return localeName.mid(idx + 1);
    }
    return {};
}

QStringList MobileProviders::getProvidersList(QString country, NetworkManager::ConnectionSettings::ConnectionType type)
{
    mProvidersGsm.clear();
    mProvidersCdma.clear();
    QDomNode n = docElement.firstChild();

    // country is a country name and we parse country codes.
    if (!mCountries.key(country).isNull()) {
        country = mCountries.key(country);
    }
    QMap<QString, QString> sortedGsm;
    QMap<QString, QString> sortedCdma;
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <country ...>

        if (!e.isNull() && e.attribute(QStringLiteral("code")).toUpper() == country) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <provider ...>

                if (!e2.isNull() && e2.tagName().toLower() == QLatin1String("provider")) {
                    QDomNode n3 = e2.firstChild();
                    bool hasGsm = false;
                    bool hasCdma = false;
                    QMap<QString, QString> localizedProviderNames;

                    while (!n3.isNull()) {
                        QDomElement e3 = n3.toElement(); // <name | gsm | cdma>

                        if (!e3.isNull()) {
                            if (e3.tagName().toLower() == QLatin1String("gsm")) {
                                hasGsm = true;
                            } else if (e3.tagName().toLower() == QLatin1String("cdma")) {
                                hasCdma = true;
                            } else if (e3.tagName().toLower() == QLatin1String("name")) {
                                QString lang = e3.attribute(QStringLiteral("xml:lang"));
                                if (lang.isEmpty()) {
                                    lang = QStringLiteral("en"); // English is default
                                } else {
                                    lang = lang.toLower();
                                    lang.remove(QRegularExpression(QStringLiteral("\\-.*$"))); // Remove everything after '-' in xml:lang attribute.
                                }
                                localizedProviderNames.insert(lang, e3.text());
                            }
                        }
                        n3 = n3.nextSibling();
                    }
                    const QString name = getNameByLocale(localizedProviderNames);
                    if (hasGsm) {
                        mProvidersGsm.insert(name, e2.firstChild());
                        sortedGsm.insert(name.toLower(), name);
                    }
                    if (hasCdma) {
                        mProvidersCdma.insert(name, e2.firstChild());
                        sortedCdma.insert(name.toLower(), name);
                    }
                }
                n2 = n2.nextSibling();
            }
            break;
        }
        n = n.nextSibling();
    }

    if (type == NetworkManager::ConnectionSettings::Gsm) {
        return sortedGsm.values();
    }
    return sortedCdma.values();
}

QStringList MobileProviders::getApns(const QString &provider)
{
    mApns.clear();
    mNetworkIds.clear();
    if (!mProvidersGsm.contains(provider)) {
        return {};
    }

    QDomNode n = mProvidersGsm[provider];

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <gsm | cdma>

        if (!e.isNull() && e.tagName().toLower() == QLatin1String("gsm")) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <apn | network-id>

                if (!e2.isNull() && e2.tagName().toLower() == QLatin1String("apn")) {
                    bool isInternet = true;
                    QDomNode n3 = e2.firstChild();
                    while (!n3.isNull()) {
                        QDomElement e3 = n3.toElement(); // <usage>
                        if (!e3.isNull() && e3.tagName().toLower() == QLatin1String("usage") && !e3.attribute(QStringLiteral("type")).isNull()
                            && e3.attribute(QStringLiteral("type")).toLower() != QLatin1String("internet")) {
                            // qCDebug(PLASMA_NM_EDITOR_LOG) << "apn" << e2.attribute("value") << "ignored because of usage" << e3.attribute("type");
                            isInternet = false;
                            break;
                        }
                        n3 = n3.nextSibling();
                    }
                    if (isInternet) {
                        mApns.insert(e2.attribute(QStringLiteral("value")), e2.firstChild());
                    }
                } else if (!e2.isNull() && e2.tagName().toLower() == QLatin1String("network-id")) {
                    mNetworkIds.append(e2.attribute(QStringLiteral("mcc")) + '-' + e2.attribute(QStringLiteral("mnc")));
                }

                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    QStringList temp = mApns.keys();
    temp.sort();
    return temp;
}

ProviderData MobileProviders::parseProvider(const QDomNode &providerNode)
{
    ProviderData result;

    QMap<QString, QString> localizedProviderNames;

    QDomNode c = providerNode.firstChild(); // <name | gsm | cdma>

    while (!c.isNull()) {
        QDomElement ce = c.toElement();

        if (ce.tagName().toLower() == QLatin1String("gsm")) {
            QDomNode gsmNode = c.firstChild();

            while (!gsmNode.isNull()) {
                QDomElement gsmElement = gsmNode.toElement();

                if (gsmElement.tagName().toLower() == QLatin1String("network-id")) {
                    result.mccmncs.append(gsmElement.attribute(QStringLiteral("mcc")) + gsmElement.attribute(QStringLiteral("mnc")));
                }
                gsmNode = gsmNode.nextSibling();
            }
        } else if (ce.tagName().toLower() == QLatin1String("name")) {
            QString lang = ce.attribute(QStringLiteral("xml:lang"));
            if (lang.isEmpty()) {
                lang = QStringLiteral("en"); // English is default
            } else {
                lang = lang.toLower();
                lang.remove(QRegularExpression(QStringLiteral("\\-.*$"))); // Remove everything after '-' in xml:lang attribute.
            }
            localizedProviderNames.insert(lang, ce.text());
        }
        c = c.nextSibling();
    }

    result.name = getNameByLocale(localizedProviderNames);

    return result;
}

QStringList MobileProviders::getProvidersFromMCCMNC(const QString &targetMccMnc)
{
    QStringList result;

    QDomNode n = docElement.firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <country ...>

        if (!e.isNull()) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <provider ...>

                if (!e2.isNull() && e2.tagName().toLower() == QLatin1String("provider")) {
                    ProviderData data = parseProvider(e2);

                    if (data.mccmncs.contains(targetMccMnc)) {
                        result << data.name;
                    }
                }
                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    return result;
}

QStringList MobileProviders::getNetworkIds(const QString &provider)
{
    if (mNetworkIds.isEmpty()) {
        getApns(provider);
    }
    return mNetworkIds;
}

QVariantMap MobileProviders::getApnInfo(const QString &apn)
{
    QVariantMap temp;
    QDomNode n = mApns[apn];
    QStringList dnsList;
    QMap<QString, QString> localizedPlanNames;

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <name|username|password|dns(*)>

        if (!e.isNull()) {
            if (e.tagName().toLower() == QLatin1String("name")) {
                QString lang = e.attribute(QStringLiteral("xml:lang"));
                if (lang.isEmpty()) {
                    lang = QStringLiteral("en"); // English is default
                } else {
                    lang = lang.toLower();
                    lang.remove(QRegularExpression(QStringLiteral("\\-.*$"))); // Remove everything after '-' in xml:lang attribute.
                }
                localizedPlanNames.insert(lang, e.text());
            } else if (e.tagName().toLower() == QLatin1String("username")) {
                temp.insert(QStringLiteral("username"), e.text());
            } else if (e.tagName().toLower() == QLatin1String("password")) {
                temp.insert(QStringLiteral("password"), e.text());
            } else if (e.tagName().toLower() == QLatin1String("dns")) {
                dnsList.append(e.text());
            }
        }

        n = n.nextSibling();
    }

    QString name = getNameByLocale(localizedPlanNames);
    if (!name.isEmpty()) {
        temp.insert(QStringLiteral("name"), QVariant::fromValue(name));
    }
    temp.insert(QStringLiteral("number"), getGsmNumber());
    temp.insert(QStringLiteral("apn"), apn);
    temp.insert(QStringLiteral("dnsList"), dnsList);

    return temp;
}

QVariantMap MobileProviders::getCdmaInfo(const QString &provider)
{
    if (!mProvidersCdma.contains(provider)) {
        return {};
    }

    QVariantMap temp;
    QDomNode n = mProvidersCdma[provider];
    QStringList sidList;

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <gsm or cdma ...>

        if (!e.isNull() && e.tagName().toLower() == QLatin1String("cdma")) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <name | username | password | sid>

                if (!e2.isNull()) {
                    if (e2.tagName().toLower() == QLatin1String("username")) {
                        temp.insert(QStringLiteral("username"), e2.text());
                    } else if (e2.tagName().toLower() == QLatin1String("password")) {
                        temp.insert(QStringLiteral("password"), e2.text());
                    } else if (e2.tagName().toLower() == QLatin1String("sid")) {
                        sidList.append(e2.text());
                    }
                }

                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    temp.insert(QStringLiteral("number"), getCdmaNumber());
    temp.insert(QStringLiteral("sidList"), sidList);
    return temp;
}

QString MobileProviders::getNameByLocale(const QMap<QString, QString> &localizedNames) const
{
    QString name;
    const QStringList locales = QLocale().uiLanguages();
    for (const QString &locale : locales) {
        QString language = locale.split(QLatin1Char('-')).at(0);

        if (localizedNames.contains(language)) {
            return localizedNames[language];
        }
    }

    name = localizedNames[QStringLiteral("en")];

    // Use any language if no proper localized name were found.
    if (name.isEmpty() && !localizedNames.isEmpty()) {
        name = localizedNames.constBegin().value();
    }
    return name;
}
