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
 * @file ResourceMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取当前系统资源占用信息
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
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import com.lingmo.lingmosdk.resource;

/**
 * @brief 获取当前系统资源占用信息类
 * 主要用来获取系统中物理内存总大小、物理内存使用率、物理内存使用大小、实际可用的物理内存大小、实际空闲的物理内存大小、
 * 所有应用申请的虚拟内存总量、系统中Swap分区总大小、Swap分区使用率、Swap分区使用量、Swap分区空闲大小、CPU瞬时使用率、
 * 操作系统开机时长
 */
@Slf4j
public class ResourceMethod implements resource{
    /**
     * @brief 获取系统中物理内存总大小
     * 
     * @return double 物理内存大小，KiB为单位
     */
    @Override
    public double getMemTotalKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemTotalKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取物理内存使用率
     * 
     * @return double 物理内存使用率
     */
    @Override
    public double getMemUsagePercent(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemUsagePercent();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取物理内存使用大小，注意Buffer/Cache被计算为已使用内存
     * 
     * @return double 物理内存使用大小，KiB为单位
     */
    @Override
    public double getMemUsageKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemUsageKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取实际可用的物理内存大小，该数值约等于Free + Buffer + Cache
     * 
     * @return double 可用物理内存大小，KiB为单位
     */
    @Override
    public double getMemAvailableKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemAvailableKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取实际空闲的物理内存大小，注意Buffer/Cache被计算为已使用内存
     * 
     * @return double 空闲的物理内存大小，KiB为单位
     */
    @Override
    public double getMemFreeKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemFreeKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取所有应用申请的虚拟内存总量
     * 
     * @return double 虚拟内存总申请量，KiB为单位
     */
    @Override
    public double getMemVirtAllocKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemVirtAllocKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取系统中Swap分区总大小
     * 
     * @return double Swap分区大小，KiB为单位
     */
    @Override
    public double getMemSwapTotalKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemSwapTotalKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取Swap分区使用率
     * 
     * @return double Swap分区使用率
     */
    @Override
    public double getMemSwapUsagePercent(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemSwapUsagePercent();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取Swap分区使用量
     * 
     * @return double Swap分区使用量，KiB为单位
     */
    @Override
    public double getMemSwapUsageKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemSwapUsageKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取Swap分区空闲大小
     * 
     * @return double Swap分区空闲大小，KiB为单位
     */
    @Override
    public double getMemSwapFreeKiB(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getMemSwapFreeKiB();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取CPU瞬时使用率
     * 
     * @return double CPU瞬时使用率，该值 < 1.00
     */
    @Override
    public double getCpuCurrentUsage(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getCpuCurrentUsage();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;

    }

    /**
     * @brief 获取操作系统开机时长
     * 
     * @return String 开机时长
     */
    @Override
    public String getUpTime(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            resource obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/resource", resource.class);
            return obj.getUpTime();
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
