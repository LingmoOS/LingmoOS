/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "ini-config.h"

#include <QSettings>
#include <QDebug>

namespace LingmoUIQuick {

class IniConfigPrivate
{
public:
    explicit IniConfigPrivate(const QString &file, IniConfig *q);
    QSettings *settings {nullptr};
};

IniConfigPrivate::IniConfigPrivate(const QString &file, IniConfig *q)
{
    settings = new QSettings(file, QSettings::IniFormat, q);
}

// ===== IniConfig ===== //
IniConfig::IniConfig(const QString &filename, QObject *parent) : ConfigIFace(parent), d(new IniConfigPrivate(filename, this))
{

}

QVariant IniConfig::getValue(const QString &key) const
{
    if (d->settings) {
        return d->settings->value(key);
    }

    return {};
}

void IniConfig::setValue(const QString &key, const QVariant &value)
{
    if (d->settings) {
        d->settings->setValue(key, value);
        Q_EMIT configChanged(key);
    }
}

void IniConfig::forceSync()
{
    if (d->settings) {
        d->settings->sync();
    }
}

QSettings *IniConfig::settings() const
{
    return d->settings;
}

QStringList IniConfig::keys() const
{
    if (d->settings) {
        return d->settings->allKeys();
    }

    return {};
}


} // LingmoUIQuick
