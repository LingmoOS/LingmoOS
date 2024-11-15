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

#ifndef LOCALE_H
#define LOCALE_H

#include <QObject>
#include <QDBusInterface>

class Language : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList languages READ languages NOTIFY loadLanguageFinished)
    Q_PROPERTY(int currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

public:
    explicit Language(QObject *parent = nullptr);

    int currentLanguage() const;
    Q_INVOKABLE void setCurrentLanguage(int index);
    QStringList languages() const;

signals:
    void loadLanguageFinished();
    void currentLanguageChanged();

private:
    QDBusInterface m_interface;
    QStringList m_languageNames;
    QStringList m_languageCodes;
    int m_currentLanguage;
};

#endif // LOCALE_H
