/*
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
 * Authors: jishengjie <jishengjie@kylinos.cn>
 *
 */

#include "buriedpoint.h"
#include "libkydiagnostics.h"

int kdk_buried_point(char *appName , char *messageType , KBuriedPoint *data , int length)
{
    std::map<std::string , std::string> uploadData;
    for (int i = 0 ; i < length ; i++) {
        std::string key = data[i].key;
        std::string value = data[i].value;
        uploadData[key] = value;
    }

    kdk::BuriedPoint buriedPoint;
    if (buriedPoint.uploadMessage(appName , messageType , uploadData)) {
        return 0;
    }

    return -1;
}