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

/**
 * @file PeripheralsenumMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取usb信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-system-java
 * @{
 * 
 */

package lingmo.lingmosdk.java.method;

import lingmo.lingmosdk.java.client.DBusClient;
import com.lingmo.lingmosdk.peripheralsenum;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;

import java.util.List;

/**
 * @brief 获取usb设备信息类
 * 主要用来获取usb设备的名称、类型、pid、vid、序列号、路径信息
 */
@Slf4j
public class PeripheralsenumMethod implements peripheralsenum {
    /**
     * @brief 获取外接usb设备信息
     * 
     * @return List<String> usb设备名称、类型、pid、vid、序列号、路径信息列表
     */
    @Override
    public List<String> getAllUsbInfo(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            peripheralsenum obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/peripheralsenum", peripheralsenum.class);
            return obj.getAllUsbInfo();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 判断是否为远程对象
     *
     * @return boolean Returns true on remote objects. Local objects implementing this interface MUST return false.
     */
    @Override
    public boolean isRemote() {
        return false;
    }

    /**
     * @brief 获取ObjectPath【未实现】
     *
     * @return String ObjectPath
     */
    @Override
    public String getObjectPath() {
        return null;
    }
}

/**
 * \example lingmosdk-system/src/lingmosdk-java/lingmosdk-java-ext/lingmosdk-java/src/test/java/lingmo/lingmosdk/java/EventTest.java
 * 
 */
