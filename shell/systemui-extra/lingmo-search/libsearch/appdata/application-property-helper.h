/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef APPLICATIONINFOHELPER_H
#define APPLICATIONINFOHELPER_H

#include "application-property.h"
#include <QMetaType>
#include <QVariant>
namespace LingmoUISearch {

class ApplicationPropertyHelperPrivate;
class ApplicationPropertyHelper
{
public:
    ApplicationPropertyHelper();
    ApplicationPropertyHelper(ApplicationProperty::Property property);
    ApplicationPropertyHelper(const ApplicationPropertyHelper &other);
    ~ApplicationPropertyHelper();

    ApplicationPropertyHelper& operator=(const ApplicationPropertyHelper &rhs);
    bool operator==(const ApplicationPropertyHelper &rhs) const;

    ApplicationProperty::Property info();
    QString dataBaseField();
    QMetaType::Type valueType();
    QString toDataBaseString(QVariant &value);

private:
    ApplicationPropertyHelperPrivate *d = nullptr;
};
}
#endif // APPLICATIONINFOHELPER_H
