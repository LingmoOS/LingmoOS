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
 * @file DiskMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取磁盘信息
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
import com.lingmo.lingmosdk.disk;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import org.freedesktop.dbus.types.UInt32;
import org.freedesktop.dbus.types.UInt64;

import java.util.List;

/**
 * @brief 获取磁盘信息类
 * 主要用来获取磁盘的列表，磁盘的扇区数量，每个扇区的字节数，容量，型号、序列号、子分区数量，类型和固件版本信息
 */
@Slf4j
public class DiskMethod implements disk {
    /**
     * @brief 获取所有磁盘的列表
     *
     * @return List<String> 磁盘的列表，每个字符串表示一个磁盘的绝对路径
     */
    @Override
    public List<String> getDiskList() {
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskList();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取磁盘的每个扇区的字节数
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return UInt32 每个扇区的字节数
     */
    @Override
    public UInt32 getDiskSectorSize(String name) {
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskSectorSize(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return new UInt32(0);
    }
    
    /**
     * @brief 获取磁盘容量
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return float 磁盘容量，MiB为单位
     */
    @Override
    public float getDiskTotalSizeMiB(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskTotalSizeMiB(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1;
    }

    /**
     * @brief 获取磁盘型号
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return String 磁盘型号
     */
    @Override
    public String getDiskModel(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskModel(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取磁盘序列号
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return String 磁盘序列号
     */
    @Override
    public String getDiskSerial(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskSerial(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取磁盘/分区下的子分区数量
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return UInt32 该磁盘/分区下的子分区数量
     */
    @Override
    public UInt32 getDiskPartitionNums(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskPartitionNums(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return new UInt32(0);
    }

    /**
     * @brief 获取磁盘类型
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return String 磁盘类型，固态 or 机械 or 混合
     */
    @Override
    public String getDiskType(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskType(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取磁盘固件版本信息
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return String 固件版本信息
     */
    @Override
    public String getDiskVersion(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskVersion(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取磁盘扇区数量
     *
     * @param name 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return UInt64 扇区数量
     */
    @Override
    public UInt64 getDiskSectorNum(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            disk obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/disk", disk.class);
            return obj.getDiskSectorNum(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return new UInt64(0);
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
