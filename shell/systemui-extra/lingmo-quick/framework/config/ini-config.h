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

#ifndef LINGMO_QUICK_INI_CONFIG_H
#define LINGMO_QUICK_INI_CONFIG_H

#include "config.h"

class QSettings;

namespace LingmoUIQuick {

class IniConfigPrivate;

class IniConfig : public ConfigIFace
{
    Q_OBJECT
public:
    explicit IniConfig(const QString &filename, QObject *parent = nullptr);

    QStringList keys() const override;
    QVariant getValue(const QString &key) const override;
    void setValue(const QString &key, const QVariant &value) override;

    void forceSync() override;

    QSettings *settings() const;

private:
    IniConfigPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_INI_CONFIG_H
