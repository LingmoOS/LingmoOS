/*
 * Copyright (C) 2011 ~ 2017 Lingmo Technology Co., Ltd.
 *
 * Author:     quezhiyong <quezhiyong@uniontech.com>
 *
 * Maintainer: quezhiyong <quezhiyong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOUCHSCREENINFOLIST_H
#define TOUCHSCREENINFOLIST_H

#include <QString>
#include <QList>
#include <QDBusMetaType>

struct TouchscreenInfo {
    qint32 id;
    QString name;
    QString deviceNode;
    QString serialNumber;

    bool operator ==(const TouchscreenInfo& info) const;
    bool operator !=(const TouchscreenInfo& info) const;
};

typedef QList<TouchscreenInfo> TouchscreenInfoList;

// 自定义 QList<TouchscreenInfo_V2> 的 != 操作符
inline bool operator!=(const TouchscreenInfoList& list1, const TouchscreenInfoList& list2) {
    if (list1.size() != list2.size())
        return true;

    for (int i = 0; i < list1.size(); ++i) {
        if (list1[i] != list2[i]) {
            return true;
        }
    }
    return false;
}

Q_DECLARE_METATYPE(TouchscreenInfo)
Q_DECLARE_METATYPE(TouchscreenInfoList)

QDBusArgument &operator<<(QDBusArgument &arg, const TouchscreenInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &arg, TouchscreenInfo &info);

void registerTouchscreenInfoListMetaType();

#endif // !TOUCHSCREENINFOLIST_H
