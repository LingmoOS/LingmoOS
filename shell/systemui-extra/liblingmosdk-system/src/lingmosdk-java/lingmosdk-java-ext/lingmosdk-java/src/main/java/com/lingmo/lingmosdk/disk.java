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

package com.lingmo.lingmosdk;

import org.freedesktop.dbus.interfaces.DBusInterface;
import org.freedesktop.dbus.types.UInt32;
import org.freedesktop.dbus.types.UInt64;
import java.util.List;

public interface disk extends DBusInterface {
    public List<String> getDiskList();
    public UInt32 getDiskSectorSize(String name);
    public float getDiskTotalSizeMiB(String name);
    public String getDiskModel(String name);
    public String getDiskSerial(String name);
    public UInt32 getDiskPartitionNums(String name);
    public String getDiskType(String name);
    public String getDiskVersion(String name);
    public UInt64 getDiskSectorNum(String name);

}
