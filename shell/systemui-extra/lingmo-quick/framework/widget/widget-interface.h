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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_WIDGETINTERFACE_H
#define LINGMO_QUICK_WIDGETINTERFACE_H
#define WidgetInterface_iid "org.lingmo.quick.widgetInterface"
#define WIDGET_INTERFACE_VERSION "1.0.0"
#include <QAction>
#include <QList>
namespace LingmoUIQuick
{

class WidgetInterface
{
public:
    WidgetInterface()= default;
    virtual QList<QAction *> actions()
    {
        return {};
    };

};
}
Q_DECLARE_INTERFACE(LingmoUIQuick::WidgetInterface, WidgetInterface_iid)
Q_DECLARE_METATYPE(LingmoUIQuick::WidgetInterface *);

#endif //LINGMO_QUICK_WIDGETINTERFACE_H
