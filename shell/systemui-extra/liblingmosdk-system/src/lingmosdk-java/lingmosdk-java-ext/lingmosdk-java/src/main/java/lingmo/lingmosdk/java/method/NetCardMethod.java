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
 * @file NetCardMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取网卡信息
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
import com.lingmo.lingmosdk.netcard;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;

import java.util.List;

/**
 * @brief 获取网卡信息类
 * 主要用来获取网卡列表、检测指定网卡是否处于UP状态、获取系统中当前处于 link up 状态的网卡列表、获取指定网卡的物理MAC地址
 * 获取指定网卡的第一个IPv4地址、获取指定网卡的所有IPv4地址、获取指定网卡的第一个IPv6地址、获取指定网卡的所有IPv6地址、
 * 获取指定网卡的有线/无线类型、获取指定网卡的厂商名称和设备型号
 */
@Slf4j
public class NetCardMethod implements netcard {
    /**
     * @brief 获取系统中所有的网卡
     *
     * @return List<String> 网卡名称列表
     */
    @Override
    public List<String> getNetCardName(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardName();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 检测指定网卡是否处于UP状态
     *
     * @param nc 网卡名称，如eno1
     * @return int Up返回1，Down返回0
     */
    @Override
    public int getNetCardUp(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardUp(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief 获取系统中当前处于 link up 状态的网卡列表
     *
     * @return List<String> 处于link up状态的网卡列表
     */
    @Override
    public List<String> getNetCardUpcards(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardUpcards();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的物理MAC地址
     *
     * @param nc 网卡名称，如eno1
     * @return String 物理MAC地址
     */
    @Override
    public String getNetCardPhymac(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardPhymac(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的第一个IPv4地址
     *
     * @param nc 网卡名称，如eno1
     * @return String IPv4地址
     */
    @Override
    public String getNetCardPrivateIPv4(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardPrivateIPv4(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的所有IPv4地址
     *
     * @param nc 网卡名称，如eno1
     * @return List<String> IPv4地址列表
     */
    @Override
    public List<String> getNetCardIPv4(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardIPv4(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的第一个IPv6地址
     *
     * @param nc 网卡名称，如eno1
     * @return String IPv6地址
     */
    @Override
    public String getNetCardPrivateIPv6(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardPrivateIPv6(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的所有IPv6地址
     *
     * @param nc 网卡名称，如eno1
     * @return List<String> IPv6地址列表
     */
    @Override
    public List<String> getNetCardIPv6(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardIPv6(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取指定网卡的有线/无线类型
     *
     * @param nc 网卡名称，如eno1
     * @return int 0 有线 1 无线
     */
    @Override
    public  int getNetCardType(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardType(nc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1;
    }

    /**
     * @brief 获取指定网卡的厂商名称和设备型号
     * 
     * @param nc 网卡名称，如eno1
     * @return List<String> 网卡的厂商名称和设备型号
     */
    @Override
    public  List<String> getNetCardProduct(String nc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            netcard obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/netcard", netcard.class);
            return obj.getNetCardProduct(nc);
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
