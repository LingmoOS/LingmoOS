/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangzihao <zhangzihao@kylinos.cn>
 *
 */
#ifndef LINGMOSEARCHQDBUS_H
#define LINGMOSEARCHQDBUS_H

#include <QDBusInterface>
#include <QDBusReply>
namespace LingmoUISearch {
class LingmoUISearchQDBus {
public:
    LingmoUISearchQDBus();
    ~LingmoUISearchQDBus();
    void setInotifyMaxUserWatches();
    void addInotifyUserInstances(int addNum);
private:
    QDBusInterface* tmpSystemQDBusInterface;
};
}

#endif // LINGMOSEARCHQDBUS_H
