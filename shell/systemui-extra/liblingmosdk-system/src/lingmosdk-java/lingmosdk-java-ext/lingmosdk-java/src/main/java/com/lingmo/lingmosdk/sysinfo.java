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

import java.util.List;

public interface sysinfo extends DBusInterface {

    public String getSystemArchitecture();

    public String getSystemName();

    public String getSystemVersion(boolean verbose);

    public int getSystemActivationStatus();

    public String getSystemSerialNumber();

    public String getSystemKernelVersion();

    public String getSystemEffectUser();

    public String getSystemProjectName();

    public String getSystemProjectSubName();

    public UInt32 getSystemProductFeatures();

    public String getSystemHostVirtType();

    public String getSystemHostCloudPlatform();

    public String getSystemOSVersion();

    public String getSystemUpdateVersion();

    public boolean getSystemIsZYJ();
}
