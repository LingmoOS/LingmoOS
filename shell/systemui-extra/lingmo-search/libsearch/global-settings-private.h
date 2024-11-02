/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef GLOBALSETTINGSPRIVATE_H
#define GLOBALSETTINGSPRIVATE_H

#include "global-settings.h"
#include <QMutex>
#include <QGSettings>
#include <QMap>

namespace LingmoUISearch {
class GlobalSettingsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit GlobalSettingsPrivate(QObject *parent = nullptr);
    QVariant getValue(const QString&);

Q_SIGNALS:
    void valueChanged(const QString& key, QVariant value);

private:
    void setValue(const QString& key, const QVariant& value);

    QMutex m_mutex;

    QGSettings *m_transGsettings = nullptr;
    QGSettings *m_themeGsettings = nullptr;
    QMap<QString, QVariant> m_cache;
};
}


#endif // GLOBALSETTINGSPRIVATE_H
