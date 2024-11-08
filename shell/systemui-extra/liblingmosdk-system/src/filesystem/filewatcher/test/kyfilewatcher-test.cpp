/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QDebug>
#include "../libkyfilewatcher.hpp"

using namespace LINGMOSDK_FILEWATCHER;

int main(int argc, char *argv[])
{
    LINGMOSDK_FILEWATCHER::FileWatcher watcher;
    watcher.addWatchTargetRecursive("/home/lingmo/", PERIODIC, ALL, 3, REGULAR);

    getchar();

//    qDebug() << "update attr test";
//    watcher.updateWatchTargetAttribute("/data/testcode/test.c", DELETE);

//    getchar();

//    qDebug() << "re-add test";
//    watcher.addWatchTarget("/data/testcode/test.c", PERIODIC, ALL);

//    getchar();

//    qDebug() << "update oneshot test";
//    watcher.updateWatchTargetType("/data/testcode/test.c", ONESHOT);

//    getchar();

//    qDebug() << "remove target test";
//    watcher.removeWatchTarget("/data/testcode/test.c");

//    getchar();

    watcher.clearWatchList();
    return 0;
}
