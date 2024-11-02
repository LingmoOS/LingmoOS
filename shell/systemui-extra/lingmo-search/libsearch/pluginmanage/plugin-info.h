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
#ifndef PLUGININFO_H
#define PLUGININFO_H

#include <QString>

namespace LingmoUISearch {

class PluginInfo
{

public:
    explicit PluginInfo(QString name, bool enable, int order, bool isFixed, bool isExternal, QString path = QString())
        : m_name(name),
          m_enable(enable),
          m_order(order),
          m_isFixed(isFixed),
          m_isExternal(isExternal),
          m_path(path) {}
    QString name() const {return m_name;}
    int order() const {return m_order;}
    bool isEnable() const {return m_enable;}
    bool isFixed() const {return m_isFixed;}
    bool isExternal() const {return m_isExternal;}
    QString path() const {return m_path;}

private:
    QString m_name;
    bool m_enable;
    int m_order;
    bool m_isFixed;
    bool m_isExternal;
    QString m_path;
};

}

#endif // PLUGININFO_H
