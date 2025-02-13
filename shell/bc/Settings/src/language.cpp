/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "language.h"
#include <QLocale>
#include <QDebug>
#include <QDir>

#include <QDBusPendingCall>

#include <unicode/locid.h>
#include <unicode/unistr.h>

static const QStringList supportList = {"en_US", "en_GB", "de_DE", "cs_CZ", "es_ES",
                                        "es_MX", "ar_AA", "bg_BG", "fa_IR", "fi_FI", "fr_FR",
                                        "uk_UA", "uz_UZ", "hu_HU", "hi_IN", "id_ID"
                                        "it_IT", "ja_JP", "nb_NO", "ne_NP", "pl_PL",
                                        "pt_BR", "pt_PT", "ru_RU", "sv_SE", "so",
                                        "be_BY", "be_Latn", "tr_TR", "zh_CN", "zh_HK", "zh_TW"
                                       };

static const QMap<QString, QString> s_displayName = {{"zh_CN", "简体中文"},
                                                     {"zh_HK", "繁體中文(香港)"},
                                                     {"zh_TW", "正體中文"}
                                                    };

Language::Language(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Language",
                  "com.lingmo.Language",
                  QDBusConnection::sessionBus())
    , m_currentLanguage(-1)
{
    QString systemLanguage = m_interface.property("languageCode").toString();

    for (const QString &code : supportList) {
        std::string string;
        icu::Locale locale = qPrintable(code);
        icu::UnicodeString unicodeString;
        locale.getDisplayName(locale, unicodeString);
        unicodeString.toUTF8String(string);

        QString displayName = s_displayName.contains(code) ? s_displayName.value(code)
                                                           : string.c_str();

        if (displayName.isEmpty() || displayName == "C")
            continue;

        m_languageNames.append(displayName);
        m_languageCodes.append(code);
    }

    // Update current language
    m_currentLanguage = m_languageCodes.indexOf(systemLanguage);

    emit loadLanguageFinished();
    emit currentLanguageChanged();
}

int Language::currentLanguage() const
{
    return m_currentLanguage;
}

void Language::setCurrentLanguage(int index)
{
    if (index >= 0 && index < m_languageCodes.length()) {
        m_interface.asyncCall("setLanguage", m_languageCodes[index]);
        qDebug() << "set language: " << m_languageCodes[index];
        m_currentLanguage = index;
        emit currentLanguageChanged();
    }
}

QStringList Language::languages() const
{
    return m_languageNames;
}
