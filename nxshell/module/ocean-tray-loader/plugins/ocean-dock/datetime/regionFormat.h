// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REGIONFORMAT_H
#define REGIONFORMAT_H

#include <QObject>

#include <DConfig>

const QString localeName_key = "localeName";
const QString country_key = "country";
const QString languageRegion_key = "languageRegion";
const QString firstDayOfWeek_key = "firstDayOfWeek";
const QString shortDateFormat_key = "shortDateFormat";
const QString longDateFormat_key = "longDateFormat";
const QString shortTimeFormat_key = "shortTimeFormat";
const QString longTimeFormat_key = "longTimeFormat";
const QString currencyFormat_key = "currencyFormat";
const QString numberFormat_key = "numberFormat";
const QString paperFormat_key = "paperFormat";

class RegionFormat : public QObject
{
    Q_OBJECT

public:
    explicit RegionFormat(QObject *parent = 0);

    void initConnect();
    void initData();

    QString getShortDateFormat() const;
    void setShortDateFormat(const QString &newShortDateFormat);
    QString getLongDateFormat() const;
    void setLongDateFormat(const QString &newLongDateFormat);
    QString getShortTimeFormat() const;
    void setShortTimeFormat(const QString &newShortTimeFormat);
    QString getLongTimeFormat() const;
    void setLongTimeFormat(const QString &newLongTimeFormat);

    QString getLocaleName() const;
    void setLocaleName(const QString &newLocaleName);

    bool is24HourFormat() const;
    void sync24HourFormatConfig(bool is24HourFormat);
    QString transformLongHourFormat(QString longTimeFormat);

    QString originShortDateFormat() const { return m_originShortDateFormat; }

signals:
    void shortDateFormatChanged() const;
    void longDateFormatChanged() const;
    void shortTimeFormatChanged() const;
    void longTimeFormatChanged() const;
    void localeNameChanged(const QString &localeName);

public slots:
    void onDockPositionChanged(int position);

private:
    QString shortDateFormat;
    QString longDateFormat;
    QString shortTimeFormat;
    QString longTimeFormat;
    QString m_originShortDateFormat;
    QString m_12HourFormat;

    QString localeName;
    DTK_CORE_NAMESPACE::DConfig *m_config;
};



#endif //REGIONFORMAT_H
