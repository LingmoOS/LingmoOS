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

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <QObject>
#include "widget-interface.h"

class TestPlugin : public QObject, public LingmoUIQuick::WidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WidgetInterface_iid)
    Q_INTERFACES(LingmoUIQuick::WidgetInterface)
public:
    TestPlugin(QObject *parent = nullptr);
    QList<QAction *> actions() override;
private:
    QList<QAction *> m_action;

};



#endif //TEST_PLUGIN_H
