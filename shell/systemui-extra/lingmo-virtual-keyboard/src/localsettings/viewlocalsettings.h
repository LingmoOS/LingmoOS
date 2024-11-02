/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VIEWLOCALSETTINGS_H
#define VIEWLOCALSETTINGS_H

#include <QFutureWatcher>
#include <QMap>

#include "localsettings.h"

class ViewLocalSettings : public LocalSettings {
public:
    using SettingMap = QMap<QString, QVariant>;
    using GroupSettingMap = QMap<QString, SettingMap>;

public:
    ViewLocalSettings(const QString &organization, const QString &application);
    ~ViewLocalSettings() override;

    QVariant getValue(const QString &group, const QString &key);

    QVariant getValue(const QString &group, const QString &key,
                      const QVariant &defaultValue) override;
    void setValue(const QString &group, const QString &key,
                  const QVariant &value) override;
    void remove(const QString &key);
    bool contains(const QString &key) const;

private:
    void saveSettingsAsync();

private:
    const QString organization_;
    const QString application_;

    GroupSettingMap groupSettingMap_;
    QFutureWatcher<void> futureWatcher_;
};

#endif // VIEWLOCALSETTINGS_H
