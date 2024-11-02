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

import java.util.List;
import org.freedesktop.dbus.interfaces.DBusInterface;

public interface netcard extends DBusInterface {

    public List<String> getNetCardName();
    public int getNetCardUp(String nc);
    public List<String> getNetCardUpcards();
    public String getNetCardPhymac(String nc);
    public String getNetCardPrivateIPv4(String nc);
    public List<String> getNetCardIPv4(String nc);
    public String getNetCardPrivateIPv6(String nc);
    public  int getNetCardType(String nc);
    public  List<String> getNetCardProduct(String nc);
    public List<String> getNetCardIPv6(String nc);
}
