/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "currency_p.h"
#include "kunitconversion_debug.h"
#include "unit_p.h"
#include "unitcategory.h"
#include <KLocalizedString>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QStandardPaths>
#include <QXmlStreamReader>

using namespace std::chrono_literals;

namespace KUnitConversion
{
static const char URL[] = "https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";

static QString cacheLocation()
{
#ifndef Q_OS_ANDROID
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
#endif
}

class CurrencyCategoryPrivate : public UnitCategoryPrivate
{
public:
    CurrencyCategoryPrivate(CategoryId id, const QString &name, const QString &description)
        : UnitCategoryPrivate(id, name, description)
    {
    }

    Value convert(const Value &value, const Unit &toUnit) override;
    bool hasOnlineConversionTable() const override;
    UpdateJob* syncConversionTable(std::chrono::seconds updateSkipSeconds) override;

    QNetworkReply *m_currentReply = nullptr;
    bool readConversionTable(const QString &cacheLocation);
    bool m_initialized{false}; //!< indicates if units are prepared from currency table
};

bool CurrencyCategoryPrivate::hasOnlineConversionTable() const
{
    return true;
}

UnitCategory Currency::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(new CurrencyCategoryPrivate(CurrencyCategory, i18n("Currency"), i18n("From ECB")));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (currency)", "%1 %2");

    // Static rates
    d->addDefaultUnit(UnitPrivate::makeUnit(CurrencyCategory,
                              Eur,
                              1.0,
                              QStringLiteral("EUR"),
                              i18nc("currency name", "Euro"),
                              i18nc("EUR Euro - unit synonyms for matching user input", "euro;euros") + QStringLiteral(";EUR;") + QStringLiteral("€")
                                  + QLatin1Char(';') + i18nc("currency name", "Euro"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 euros"),
                              ki18ncp("amount in units (integer)", "%1 euro", "%1 euros")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Ats,
                       0.0726728,
                       QStringLiteral("ATS"),
                       i18nc("currency name", "Austrian Schilling"),
                       i18nc("ATS Austrian Schilling - unit synonyms for matching user input", "schilling;schillings") + QStringLiteral(";ATS;")
                           + QLocale::territoryToString(QLocale::Austria) + QLatin1Char(';') + i18nc("currency name", "Austrian Schilling"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 schillings"),
                       ki18ncp("amount in units (integer)", "%1 schilling", "%1 schillings")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Bef,
                       0.0247894,
                       QStringLiteral("BEF"),
                       i18nc("currency name", "Belgian Franc"),
                       i18nc("BEF Belgian Franc - unit synonyms for matching user input", "franc;francs") + QStringLiteral(";BEF;")
                           + QLocale::territoryToString(QLocale::Belgium) + QLatin1Char(';') + i18nc("currency name", "Belgian Franc"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Belgian francs"),
                       ki18ncp("amount in units (integer)", "%1 Belgian franc", "%1 Belgian francs")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Nlg,
                       0.45378,
                       QStringLiteral("NLG"),
                       i18nc("currency name", "Netherlands Guilder"),
                       i18nc("NLG Netherlands Guilder - unit synonyms for matching user input", "guilder;guilders") + QStringLiteral(";NLG;")
                           + QLocale::territoryToString(QLocale::Netherlands) + QLatin1Char(';') + i18nc("currency name", "Netherlands Guilder"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 guilders"),
                       ki18ncp("amount in units (integer)", "%1 guilder", "%1 guilders")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Fim,
                       0.168188,
                       QStringLiteral("FIM"),
                       i18nc("currency name", "Finnish Markka"),
                       i18nc("FIM Finnish Markka - unit synonyms for matching user input", "markka;markkas;markkaa") + QStringLiteral(";FIM;")
                           + QLocale::territoryToString(QLocale::Finland) + QLatin1Char(';') + i18nc("currency name", "Finnish Markka"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 markkas"),
                       ki18ncp("amount in units (integer)", "%1 markka", "%1 markkas"))); // Alternative = markkaa

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Frf,
                       0.152449,
                       QStringLiteral("FRF"),
                       i18nc("currency name", "French Franc"),
                       i18nc("FRF French Franc - unit synonyms for matching user input", "franc;francs") + QStringLiteral(";FRF;")
                           + QLocale::territoryToString(QLocale::France) + QLatin1Char(';') + i18nc("currency name", "French Franc"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 French francs"),
                       ki18ncp("amount in units (integer)", "%1 French franc", "%1 French francs")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Dem,
                       0.511292,
                       QStringLiteral("DEM"),
                       i18nc("currency name", "German Mark"),
                       i18nc("DEM German Mark - unit synonyms for matching user input", "mark;marks") + QStringLiteral(";DEM;")
                           + QLocale::territoryToString(QLocale::Germany) + QLatin1Char(';') + i18nc("currency name", "German Mark"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 marks"),
                       ki18ncp("amount in units (integer)", "%1 mark", "%1 marks")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Iep,
                       1.26974,
                       QStringLiteral("IEP"),
                       i18nc("currency name", "Irish Pound"),
                       i18nc("IEP Irish Pound - unit synonyms for matching user input", "Irish pound;Irish pounds") + QStringLiteral(";IEP;")
                           + QLocale::territoryToString(QLocale::Ireland) + QLatin1Char(';') + i18nc("currency name", "Irish Pound"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Irish pounds"),
                       ki18ncp("amount in units (integer)", "%1 Irish pound", "%1 Irish pounds")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Itl,
                       0.000516457,
                       QStringLiteral("ITL"),
                       i18nc("currency name", "Italian Lira"),
                       i18nc("ITL Italian Lira - unit synonyms for matching user input", "lira;liras") + QStringLiteral(";ITL;")
                           + QLocale::territoryToString(QLocale::Italy) + QLatin1Char(';') + i18nc("currency name", "Italian Lira"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Italian lira"),
                       ki18ncp("amount in units (integer)", "%1 Italian lira", "%1 Italian lira")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Luf,
                       0.0247894,
                       QStringLiteral("LUF"),
                       i18nc("currency name", "Luxembourgish Franc"),
                       i18nc("LUF Luxembourgish Franc - unit synonyms for matching user input", "franc;francs") + QStringLiteral(";LUF;")
                           + QLocale::territoryToString(QLocale::Luxembourg) + QLatin1Char(';') + i18nc("currency name", "Luxembourgish Franc"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Luxembourgish francs"),
                       ki18ncp("amount in units (integer)", "%1 Luxembourgish franc", "%1 Luxembourgish francs")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Pte,
                       0.00498798,
                       QStringLiteral("PTE"),
                       i18nc("currency name", "Portuguese Escudo"),
                       i18nc("PTE Portuguese Escudo - unit synonyms for matching user input", "escudo;escudos") + QStringLiteral(";PTE;")
                           + QLocale::territoryToString(QLocale::Portugal) + QLatin1Char(';') + i18nc("currency name", "Portuguese Escudo"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 escudos"),
                       ki18ncp("amount in units (integer)", "%1 escudo", "%1 escudos")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Esp,
                       0.00601012,
                       QStringLiteral("ESP"),
                       i18nc("currency name", "Spanish Peseta"),
                       i18nc("ESP Spanish Peseta - unit synonyms for matching user input", "peseta;pesetas") + QStringLiteral(";ESP;")
                           + QLocale::territoryToString(QLocale::Spain) + QLatin1Char(';') + i18nc("currency name", "Spanish Peseta"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 pesetas"),
                       ki18ncp("amount in units (integer)", "%1 peseta", "%1 pesetas")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Grd,
                       0.0029347,
                       QStringLiteral("GRD"),
                       i18nc("currency name", "Greek Drachma"),
                       i18nc("GRD Greek Drachma - unit synonyms for matching user input", "drachma;drachmas") + QStringLiteral(";GRD;")
                           + QLocale::territoryToString(QLocale::Greece) + QLatin1Char(';') + i18nc("currency name", "Greek Drachma"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 drachmas"),
                       ki18ncp("amount in units (integer)", "%1 drachma", "%1 drachmas")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Sit,
                       0.00417293,
                       QStringLiteral("SIT"),
                       i18nc("currency name", "Slovenian Tolar"),
                       i18nc("SIT Slovenian Tolar - unit synonyms for matching user input", "tolar;tolars;tolarjev") + QStringLiteral(";SIT;")
                           + QLocale::territoryToString(QLocale::Slovenia) + QLatin1Char(';') + i18nc("currency name", "Slovenian Tolar"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 tolars"),
                       ki18ncp("amount in units (integer)", "%1 tolar", "%1 tolars"))); // Alt: tolarjev/tolarja/tolarji

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Cyp,
                       1.7086,
                       QStringLiteral("CYP"),
                       i18nc("currency name", "Cypriot Pound"),
                       i18nc("CYP Cypriot Pound - unit synonyms for matching user input", "Cypriot pound;Cypriot pounds") + QStringLiteral(";CYP;")
                           + QLocale::territoryToString(QLocale::Cyprus) + QLatin1Char(';') + i18nc("currency name", "Cypriot Pound"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Cypriot pounds"),
                       ki18ncp("amount in units (integer)", "%1 Cypriot pound", "%1 Cypriot pounds")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Mtl,
                       2.32937,
                       QStringLiteral("MTL"),
                       i18nc("currency name", "Maltese Lira"),
                       i18nc("MTL Maltese Lira - unit synonyms for matching user input", "Maltese lira") + QStringLiteral(";MTL;")
                           + QLocale::territoryToString(QLocale::Malta) + QLatin1Char(';') + i18nc("currency name", "Maltese Lira"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Maltese lira"),
                       ki18ncp("amount in units (integer)", "%1 Maltese lira", "%1 Maltese lira")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Skk,
                       0.0331939,
                       QStringLiteral("SKK"),
                       i18nc("currency name", "Slovak Koruna"),
                       i18nc("SKK Slovak Koruna - unit synonyms for matching user input", "koruna;korunas;koruny;korun") + QStringLiteral(";SKK;")
                           + QLocale::territoryToString(QLocale::Slovakia) + QLatin1Char(';') + i18nc("currency name", "Slovak Koruna"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Slovak korunas"),
                       ki18ncp("amount in units (integer)", "%1 Slovak koruna", "%1 Slovak korunas"))); // Alt: koruny, korun

    // From ECB
    d->addCommonUnit(UnitPrivate::makeUnit(CurrencyCategory,
                             Usd,
                             qSNaN(),
                             QStringLiteral("USD"),
                             i18nc("currency name", "United States Dollar"),
                             i18nc("USD United States Dollar - unit synonyms for matching user input", "dollar;dollars") + QStringLiteral(";USD;$;")
                                 + QLocale::territoryToString(QLocale::UnitedStates) + QLatin1Char(';') + i18nc("currency name", "United States Dollar"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 United States dollars"),
                             ki18ncp("amount in units (integer)", "%1 United States dollar", "%1 United States dollars")));

    d->addCommonUnit(UnitPrivate::makeUnit(CurrencyCategory,
                             Jpy,
                             qSNaN(),
                             QStringLiteral("JPY"),
                             i18nc("currency name", "Japanese Yen"),
                             i18nc("JPY Japanese Yen - unit synonyms for matching user input", "yen") + QStringLiteral(";JPY;") + QStringLiteral("¥")
                                 + QLatin1Char(';') + QLocale::territoryToString(QLocale::Japan) + QLatin1Char(';') + i18nc("currency name", "Japanese Yen"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 yen"),
                             ki18ncp("amount in units (integer)", "%1 yen", "%1 yen")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Bgn,
                       qSNaN(),
                       QStringLiteral("BGN"),
                       i18nc("currency name", "Bulgarian Lev"),
                       i18nc("BGN Bulgarian Lev - unit synonyms for matching user input", "lev;leva") + QStringLiteral(";BGN;")
                           + QLocale::territoryToString(QLocale::Bulgaria) + QLatin1Char(';') + i18nc("currency name", "Bulgarian Lev"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 leva"),
                       ki18ncp("amount in units (integer)", "%1 lev", "%1 leva")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Czk,
                       qSNaN(),
                       QStringLiteral("CZK"),
                       i18nc("currency name", "Czech Koruna"),
                       i18nc("CZK Czech Koruna - unit synonyms for matching user input", "koruna;korunas") + QStringLiteral(";CZK;")
                           + QLocale::territoryToString(QLocale::CzechRepublic) + QLatin1Char(';') + i18nc("currency name", "Czech Koruna"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Czech korunas"),
                       // Alt: koruny, korun
                       ki18ncp("amount in units (integer)", "%1 Czech koruna", "%1 Czech korunas")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Dkk,
                       qSNaN(),
                       QStringLiteral("DKK"),
                       i18nc("currency name", "Danish Krone"),
                       i18nc("DKK Danish Krone - unit synonyms for matching user input", "Danish krone;Danish kroner") + QStringLiteral(";DKK;")
                           + QLocale::territoryToString(QLocale::Denmark) + QLatin1Char(';') + i18nc("currency name", "Danish Krone"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Danish kroner"),
                       ki18ncp("amount in units (integer)", "%1 Danish krone", "%1 Danish kroner")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Eek,
                       qSNaN(),
                       QStringLiteral("EEK"),
                       i18nc("currency name", "Estonian Kroon"),
                       i18nc("EEK Estonian Kroon - unit synonyms for matching user input", "kroon;kroons;krooni") + QStringLiteral(";EEK;")
                           + QLocale::territoryToString(QLocale::Estonia) + QLatin1Char(';') + i18nc("currency name", "Estonian Kroon"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 kroons"),
                       ki18ncp("amount in units (integer)", "%1 kroon", "%1 kroons"))); // Alt: krooni

    d->addCommonUnit(UnitPrivate::makeUnit(CurrencyCategory,
                             Gbp,
                             qSNaN(),
                             QStringLiteral("GBP"),
                             i18nc("currency name", "British Pound"),
                             i18nc("GBP British Pound - unit synonyms for matching user input", "pound;pounds;pound sterling;pounds sterling")
                                 + QStringLiteral(";GBP;") + QStringLiteral("£") + QLatin1Char(';') + QLocale::territoryToString(QLocale::UnitedKingdom)
                                 + QLatin1Char(';') + i18nc("currency name", "British Pound"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 pounds sterling"),
                             ki18ncp("amount in units (integer)", "%1 pound sterling", "%1 pounds sterling")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Huf,
                       qSNaN(),
                       QStringLiteral("HUF"),
                       i18nc("currency name", "Hungarian Forint"),
                       i18nc("HUF hungarian Forint - unit synonyms for matching user input", "forint") + QStringLiteral(";HUF;")
                           + QLocale::territoryToString(QLocale::Hungary) + QLatin1Char(';') + i18nc("currency name", "Hungarian Forint"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 forint"),
                       ki18ncp("amount in units (integer)", "%1 forint", "%1 forint")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Ils,
                       qSNaN(),
                       QStringLiteral("ILS"),
                       i18nc("currency name", "Israeli New Shekel"),
                       i18nc("ILS Israeli New Shekel - unit synonyms for matching user input", "shekel;shekels;sheqel;sheqels;sheqalim")
                           + QStringLiteral(";ILS;") + QLocale::territoryToString(QLocale::Israel) + QLatin1Char(';')
                           + i18nc("currency name", "Israeli New Shekel"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 shekels"),
                       ki18ncp("amount in units (integer)", "%1 shekel", "%1 shekels")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Ltl,
                       qSNaN(),
                       QStringLiteral("LTL"),
                       i18nc("currency name", "Lithuanian Litas"),
                       i18nc("LTL Lithuanian Litas - unit synonyms for matching user input", "litas;litai;litu") + QStringLiteral(";LTL;")
                           + QLocale::territoryToString(QLocale::Lithuania) + QLatin1Char(';') + i18nc("currency name", "Lithuanian Litas"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 litas"),
                       ki18ncp("amount in units (integer)", "%1 litas", "%1 litai"))); // Alt: litu

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Lvl,
                       qSNaN(),
                       QStringLiteral("LVL"),
                       i18nc("currency name", "Latvian Lats"),
                       i18nc("LVL Latvian Lats - unit synonyms for matching user input", "lats;lati") + QStringLiteral(";LVL;")
                           + QLocale::territoryToString(QLocale::Latvia) + QLatin1Char(';') + i18nc("currency name", "Latvian Lats"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 lati"),
                       ki18ncp("amount in units (integer)", "%1 lats", "%1 lati")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Pln,
                       qSNaN(),
                       QStringLiteral("PLN"),
                       i18nc("currency name", "Polish Zloty"),
                       i18nc("PLN Polish Zloty - unit synonyms for matching user input", "zloty;zlotys;zloties") + QStringLiteral(";PLN;")
                           + QLocale::territoryToString(QLocale::Poland) + QLatin1Char(';') + i18nc("currency name", "Polish Zloty"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zlotys"),
                       ki18ncp("amount in units (integer)", "%1 zloty", "%1 zlotys"))); // Alt: zloty, zlote, zlotych

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Ron,
                       qSNaN(),
                       QStringLiteral("RON"),
                       i18nc("currency name", "Romanian Leu"),
                       i18nc("RON Romanian Leu - unit synonyms for matching user input", "leu;lei") + QStringLiteral(";RON;")
                           + QLocale::territoryToString(QLocale::Romania) + QLatin1Char(';') + i18nc("currency name", "Romanian Leu"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 lei"),
                       ki18ncp("amount in units (integer)", "%1 leu", "%1 lei")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Sek,
                       qSNaN(),
                       QStringLiteral("SEK"),
                       i18nc("currency name", "Swedish Krona"),
                       i18nc("SEK Swedish Krona - unit synonyms for matching user input", "krona;kronor") + QStringLiteral(";SEK;")
                           + QLocale::territoryToString(QLocale::Sweden) + QLatin1Char(';') + i18nc("currency name", "Swedish Krona"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 kronor"),
                       ki18ncp("amount in units (integer)", "%1 krona", "%1 kronor")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Chf,
                       qSNaN(),
                       QStringLiteral("CHF"),
                       i18nc("currency name", "Swiss Franc"),
                       i18nc("CHF Swiss Franc - unit synonyms for matching user input", "franc;francs") + QStringLiteral(";CHF;")
                           + QLocale::territoryToString(QLocale::Switzerland) + QLatin1Char(';') + i18nc("currency name", "Swiss Franc"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Swiss francs"),
                       ki18ncp("amount in units (integer)", "%1 Swiss franc", "%1 Swiss francs")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Nok,
                       qSNaN(),
                       QStringLiteral("NOK"),
                       i18nc("currency name", "Norwegian Krone"),
                       i18nc("Norwegian Krone - unit synonyms for matching user input", "Norwegian krone;Norwegian kroner") + QStringLiteral(";NOK;")
                           + QLocale::territoryToString(QLocale::Norway) + QLatin1Char(';') + i18nc("currency name", "Norwegian Krone"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Norwegian kroner"),
                       ki18ncp("amount in units (integer)", "%1 Norwegian krone", "%1 Norwegian kroner")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Hrk,
                       1.0/7.53450,
                       QStringLiteral("HRK"),
                       i18nc("currency name", "Croatian Kuna"),
                       i18nc("HRK Croatian Kuna - unit synonyms for matching user input", "kuna;kune") + QStringLiteral(";HRK;")
                           + QLocale::territoryToString(QLocale::Croatia) + QLatin1Char(';') + i18nc("currency name", "Croatian Kuna"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 kune"),
                       ki18ncp("amount in units (integer)", "%1 kuna", "%1 kune")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Rub,
                       qSNaN(),
                       QStringLiteral("RUB"),
                       i18nc("currency name", "Russian Ruble"),
                       i18nc("RUB Russian Ruble - unit synonyms for matching user input", "ruble;rubles;rouble;roubles") + QStringLiteral(";RUB;")
                           + QLocale::territoryToString(QLocale::Russia) + QLatin1Char(';') + i18nc("currency name", "Russian Ruble"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 rubles"),
                       ki18ncp("amount in units (integer)", "%1 ruble", "%1 rubles"))); // Alt: rouble/roubles

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Try,
                       qSNaN(),
                       QStringLiteral("TRY"),
                       i18nc("currency name", "Turkish Lira"),
                       i18nc("TRY Turkish Lira - unit synonyms for matching user input", "lira") + QStringLiteral(";TRY;")
                           + QLocale::territoryToString(QLocale::Turkey) + QLatin1Char(';') + i18nc("currency name", "Turkish Lira"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Turkish lira"),
                       ki18ncp("amount in units (integer)", "%1 Turkish lira", "%1 Turkish lira")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Aud,
                       qSNaN(),
                       QStringLiteral("AUD"),
                       i18nc("currency name", "Australian Dollar"),
                       i18nc("AUD Australian Dollar - unit synonyms for matching user input", "Australian dollar;Australian dollars") + QStringLiteral(";AUD;")
                           + QLocale::territoryToString(QLocale::Australia) + QLatin1Char(';') + i18nc("currency name", "Australian Dollar"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Australian dollars"),
                       ki18ncp("amount in units (integer)", "%1 Australian dollar", "%1 Australian dollars")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Brl,
                       qSNaN(),
                       QStringLiteral("BRL"),
                       i18nc("currency name", "Brazilian Real"),
                       i18nc("BRL Brazilian Real - unit synonyms for matching user input", "real;reais") + QStringLiteral(";BRL;")
                           + QLocale::territoryToString(QLocale::Brazil) + QLatin1Char(';') + i18nc("currency name", "Brazilian Real"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 reais"),
                       ki18ncp("amount in units (integer)", "%1 real", "%1 reais")));

    d->addCommonUnit(UnitPrivate::makeUnit(CurrencyCategory,
                             Cad,
                             qSNaN(),
                             QStringLiteral("CAD"),
                             i18nc("currency name", "Canadian Dollar"),
                             i18nc("Canadian Dollar - unit synonyms for matching user input", "Canadian dollar;Canadian dollars") + QStringLiteral(";CAD;")
                                 + QLocale::territoryToString(QLocale::Canada) + QLatin1Char(';') + i18nc("currency name", "Canadian Dollar"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 Canadian dollars"),
                             ki18ncp("amount in units (integer)", "%1 Canadian dollar", "%1 Canadian dollars")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Cny,
                       qSNaN(),
                       QStringLiteral("CNY"),
                       i18nc("currency name", "Chinese Yuan"),
                       i18nc("Chinese Yuan - unit synonyms for matching user input", "yuan") + QStringLiteral(";CNY;")
                           + QLocale::territoryToString(QLocale::China) + QLatin1Char(';') + i18nc("currency name", "Chinese Yuan"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yuan"),
                       ki18ncp("amount in units (integer)", "%1 yuan", "%1 yuan")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Hkd,
                       qSNaN(),
                       QStringLiteral("HKD"),
                       i18nc("currency name", "Hong Kong Dollar"),
                       i18nc("Hong Kong Dollar - unit synonyms for matching user input", "Hong Kong dollar;Hong Kong dollars") + QStringLiteral(";HKD;")
                           + QLocale::territoryToString(QLocale::HongKong) + QLatin1Char(';') + i18nc("currency name", "Hong Kong Dollar"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Hong Kong dollars"),
                       ki18ncp("amount in units (integer)", "%1 Hong Kong dollar", "%1 Hong Kong dollars")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Idr,
                       qSNaN(),
                       QStringLiteral("IDR"),
                       i18nc("currency name", "Indonesian Rupiah"),
                       i18nc("IDR Indonesian Rupiah - unit synonyms for matching user input", "rupiah;rupiahs") + QStringLiteral(";IDR;")
                           + QLocale::territoryToString(QLocale::Indonesia) + QLatin1Char(';') + i18nc("currency name", "Indonesian Rupiah"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 rupiahs"),
                       ki18ncp("amount in units (integer)", "%1 rupiah", "%1 rupiahs"))); // Alt: rupiah

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Inr,
                       qSNaN(),
                       QStringLiteral("INR"),
                       i18nc("currency name", "Indian Rupee"),
                       i18nc("INR Indian Rupee - unit synonyms for matching user input", "rupee;rupees") + QStringLiteral(";INR;")
                           + QLocale::territoryToString(QLocale::India) + QLatin1Char(';') + i18nc("currency name", "Indian Rupee"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 rupees"),
                       ki18ncp("amount in units (integer)", "%1 rupee", "%1 rupees"))); // Alt: rupee

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Krw,
                       qSNaN(),
                       QStringLiteral("KRW"),
                       i18nc("currency name", "Korean Won"),
                       i18nc("KRW Korean Won - unit synonyms for matching user input", "won") + QStringLiteral(";KRW;")
                           + QLocale::territoryToString(QLocale::SouthKorea) + QLatin1Char(';') + i18nc("currency name", "Korean Won"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 won"),
                       ki18ncp("amount in units (integer)", "%1 won", "%1 won")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Mxn,
                       qSNaN(),
                       QStringLiteral("MXN"),
                       i18nc("currency name", "Mexican Peso"),
                       i18nc("MXN Mexican Peso - unit synonyms for matching user input", "Mexican peso;Mexican pesos") + QStringLiteral(";MXN;")
                           + QLocale::territoryToString(QLocale::Mexico) + QLatin1Char(';') + i18nc("currency name", "Mexican Peso"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Mexican pesos"),
                       ki18ncp("amount in units (integer)", "%1 Mexican peso", "%1 Mexican pesos")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Myr,
                       qSNaN(),
                       QStringLiteral("MYR"),
                       i18nc("currency name", "Malaysian Ringgit"),
                       i18nc("MYR Malaysian Ringgit - unit synonyms for matching user input", "ringgit;ringgits") + QStringLiteral(";MYR;")
                           + QLocale::territoryToString(QLocale::Malaysia) + QLatin1Char(';') + i18nc("currency name", "Malaysian Ringgit"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 ringgit"),
                       ki18ncp("amount in units (integer)", "%1 ringgit", "%1 ringgit"))); // Alt: ringgits

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Nzd,
                       qSNaN(),
                       QStringLiteral("NZD"),
                       i18nc("currency name", "New Zealand Dollar"),
                       i18nc("NZD New Zealand Dollar - unit synonyms for matching user input", "New Zealand dollar;New Zealand dollars")
                           + QStringLiteral(";NZD;") + QLocale::territoryToString(QLocale::NewZealand) + QLatin1Char(';')
                           + i18nc("currency name", "New Zealand Dollar"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 New Zealand dollars"),
                       ki18ncp("amount in units (integer)", "%1 New Zealand dollar", "%1 New Zealand dollars")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Php,
                       qSNaN(),
                       QStringLiteral("PHP"),
                       i18nc("currency name", "Philippine Peso"),
                       i18nc("PHP Philippine Peso - unit synonyms for matching user input", "Philippine peso;Philippine pesos") + QStringLiteral(";PHP;")
                           + QLocale::territoryToString(QLocale::Philippines) + QLatin1Char(';') + i18nc("currency name", "Philippine Peso"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Philippine pesos"),
                       ki18ncp("amount in units (integer)", "%1 Philippine peso", "%1 Philippine pesos")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Sgd,
                       qSNaN(),
                       QStringLiteral("SGD"),
                       i18nc("currency name", "Singapore Dollar"),
                       i18nc("SGD Singapore Dollar - unit synonyms for matching user input", "Singapore dollar;Singapore dollars") + QStringLiteral(";SGD;")
                           + QLocale::territoryToString(QLocale::Singapore) + QLatin1Char(';') + i18nc("currency name", "Singapore Dollar"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Singapore dollars"),
                       ki18ncp("amount in units (integer)", "%1 Singapore dollar", "%1 Singapore dollars")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Thb,
                       qSNaN(),
                       QStringLiteral("THB"),
                       i18nc("currency name", "Thai Baht"),
                       i18nc("THB Thai Baht - unit synonyms for matching user input", "baht") + QStringLiteral(";THB;")
                           + QLocale::territoryToString(QLocale::Thailand) + QLatin1Char(';') + i18nc("currency name", "Thai Baht"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 baht"),
                       ki18ncp("amount in units (integer)", "%1 baht", "%1 baht")));

    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Zar,
                       qSNaN(),
                       QStringLiteral("ZAR"),
                       i18nc("currency name", "South African Rand"),
                       i18nc("South African Rand - unit synonyms for matching user input", "rand") + QStringLiteral(";ZAR;")
                           + QLocale::territoryToString(QLocale::SouthAfrica) + QLatin1Char(';') + i18nc("currency name", "South African Rand"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 rand"),
                       ki18ncp("amount in units (integer)", "%1 rand", "%1 rand")));
    d->addUnit(UnitPrivate::makeUnit(CurrencyCategory,
                       Isk,
                       qSNaN(),
                       QStringLiteral("ISK"),
                       i18nc("currency name", "Icelandic Krona"),
                       i18nc("Icelandic Krona - unit synonyms for matching user input", "Icelandic krona;Icelandic crown") + QStringLiteral(";ISK;")
                           + QLocale::territoryToString(QLocale::Iceland) + QLatin1Char(';') + i18nc("currency name", "Icelandic Krona"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Icelandic kronar"),
                       ki18ncp("amount in units (integer)", "%1 Icelandic krona", "%1 Icelandic kronar")));

    return c;
}

static bool isConnected()
{
    // don't download currencies in autotests
    if (qEnvironmentVariableIsSet("KF5UNITCONVERT_NO_DOWNLOAD")) {
        return false;
    }
    bool ret = false;
    const auto listInterface = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &net : listInterface) {
        if (net.flags().testFlag(QNetworkInterface::IsUp) && !net.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            ret = true;
            break;
        }
    }
    return ret;
}

QDateTime Currency::lastConversionTableUpdate()
{
    QFileInfo info(cacheLocation());
    if (!info.exists()) {
        qCDebug(LOG_KUNITCONVERSION) << "No cache file available:" << cacheLocation();
        return QDateTime();
    } else {
        return info.lastModified();
    }
}

UpdateJob* CurrencyCategoryPrivate::syncConversionTable(std::chrono::seconds updateSkipPeriod)
{
    QFileInfo info(cacheLocation());
    if (info.exists() && info.lastModified().secsTo(QDateTime::currentDateTime()) <= updateSkipPeriod.count()) {
        return nullptr; // already present and up to date
    }
    if (!isConnected()) {
        qCInfo(LOG_KUNITCONVERSION) << "currency conversion table update has no network connection, abort update";
        return nullptr;
    }
    if (m_currentReply) {
        return makeUpdateJob(m_currentReply);
    }

    qCDebug(LOG_KUNITCONVERSION) << "currency conversion table sync started";
    m_currentReply = nam()->get(QNetworkRequest(QUrl(QString::fromLatin1(URL))));
    QObject::connect(m_currentReply, &QNetworkReply::finished, [this] {
        auto reply = m_currentReply;
        m_currentReply = nullptr;
        reply->deleteLater();
        if (reply->error()) {
            qCWarning(LOG_KUNITCONVERSION) << "currency conversion table network error" << reply->errorString();
            return;
        }
        const auto cachePath = cacheLocation();
        QFileInfo info(cachePath);
        const QString cacheDir = info.absolutePath();
        if (!QFileInfo::exists(cacheDir)) {
            QDir().mkpath(cacheDir);
        }

        QSaveFile cacheFile(cachePath);
        if (!cacheFile.open(QFile::WriteOnly)) {
            qCCritical(LOG_KUNITCONVERSION) << cacheFile.errorString();
            return;
        }
        cacheFile.write(reply->readAll());
        if (!cacheFile.commit()) {
            qCCritical(LOG_KUNITCONVERSION) << cacheFile.errorString();
            return;
        }
        qCInfo(LOG_KUNITCONVERSION) << "currency conversion table data obtained via network";
        m_initialized = readConversionTable(cachePath);
    });

    return makeUpdateJob(m_currentReply);
}

bool CurrencyCategoryPrivate::readConversionTable(const QString &cachePath)
{
    QFile file(cachePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QLatin1String("Cube")) {
            const auto attributes = xml.attributes();
            if (attributes.hasAttribute(QLatin1String("currency"))) {
                Unit unit = m_unitMap.value(attributes.value(QLatin1String("currency")).toString());
                if (unit.isValid()) {
                    const auto multiplier = attributes.value(QLatin1String("rate")).toDouble();
                    if (!qFuzzyIsNull(multiplier)) {
                        unit.setUnitMultiplier(1.0 / multiplier);
                        qCDebug(LOG_KUNITCONVERSION()) << "currency updated:" << unit.description() << multiplier;
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        qCCritical(LOG_KUNITCONVERSION) << "currency conversion fetch could not parse obtained XML, update aborted";
        return false;
    }
    return true;
}

Value CurrencyCategoryPrivate::convert(const Value &value, const Unit &to)
{
    if (!m_initialized) {
        m_initialized = readConversionTable(cacheLocation());
    }

    Value v = UnitCategoryPrivate::convert(value, to);
    return v;
}

} // KUnitConversion namespace
