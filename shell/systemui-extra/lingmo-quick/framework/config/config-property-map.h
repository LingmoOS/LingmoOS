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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_CONFIGPROPERTYMAP_H
#define LINGMO_QUICK_CONFIGPROPERTYMAP_H

#include <QQmlPropertyMap>
#include <memory>
#include "config.h"

namespace LingmoUIQuick {

class ConfigPropertyMapPrivate;

class ConfigPropertyMap : public QQmlPropertyMap
{
    Q_OBJECT
public:
    explicit ConfigPropertyMap(Config* config, QObject *parent = nullptr);

protected:
    QVariant updateValue(const QString &key, const QVariant &input) override;

private:
    std::unique_ptr<ConfigPropertyMapPrivate> const d;
};
}

#endif //LINGMO_QUICK_CONFIGPROPERTYMAP_H
