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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#ifndef APPWIDGETCOMMON_H
#define APPWIDGETCOMMON_H

namespace AppWidget {

class AppWidgetCommon
{
public:
    static int getProviderIndexFromdecodeAppWidgetId(const int id) {
        int sum = id;
        int index = (sum >> 16) & (int)0xffff;
        return index;
    }
    static int getWidgetIndexFromdecodeAppWidgetId(const int id) {
        int sum = id;
        int index = sum & (int)0xffff;
        return index;
    }
    static int createAppWidgetId(const int providerindex, const int appwidgetindex) {
        int index1 = providerindex;
        int index2 = appwidgetindex;
        int  id;
        id = (index1 << 16) | index2;
        return id;
    }
};

} // namespace AppWidget
#endif // APPWIDGETCOMMON_H
