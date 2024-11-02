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
 * @file SysInfoMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取操作系统基础信息
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
import com.lingmo.lingmosdk.sysinfo;
import com.lingmo.lingmosdk.resolution;
import org.freedesktop.dbus.types.UInt32;

import java.util.List;

/**
 * @brief 获取操作系统基础信息类
 * 主要用来获取操作系统架构信息、操作系统名称、操作系统版本号、操作系统激活状态、操作系统服务序列号、内核版本号、
 * 当前登录用户的用户名（Effect User）、操作系统项目编号名、操作系统项目子编号名、操作系统产品标识码、操作系统宿主机的虚拟机类型、
 * 操作系统宿主机的云平台类型、判断当前镜像系统是否为 专用机 系统、系统版本号/补丁版本号、系统当前显示屏幕,分辨率,系统支持分辨率
 */
@Slf4j
public class SysInfoMethod implements sysinfo{
    /**
     * @brief 获取操作系统架构信息
     * 
     * @return String 操作系统架构信息
     */
    @Override
    public String getSystemArchitecture(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemArchitecture();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统名称
     * 
     * @return String 操作系统名称
     */
    @Override
    public String getSystemName(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemName();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统版本号
     * 
     * @param verbose 0获取简略版本号，1获取详细版本号
     * @return String 操作系统版本号
     */
    @Override
    public String getSystemVersion(boolean verbose){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemVersion(verbose);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统激活状态
     * 
     * @return int 2表示已过期；0表示未激活，处于试用期；1表示已激活；-1表示接口内部错误；
     */
    @Override
    public int getSystemActivationStatus(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemActivationStatus();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief 获取操作系统服务序列号
     * 
     * @return  String 操作系统服务序列号
     */
    @Override
    public String getSystemSerialNumber(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemSerialNumber();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取内核版本号
     * 
     * @return String 内核版本号
     */
    @Override
    public String getSystemKernelVersion(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemKernelVersion();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取当前登录用户的用户名（Effect User）
     * 
     * @return String 当前登录用户的用户名（Effect User）
     */
    @Override
    public String getSystemEffectUser(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemEffectUser();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统项目编号名
     * 
     * @return String 操作系统项目编号名
     */
    @Override
    public String getSystemProjectName(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemProjectName();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统项目子编号名
     * 
     * @return String 操作系统项目子编号名
     */
    @Override
    public String getSystemProjectSubName(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemProjectSubName();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统产品标识码
     * 
     * @return UInt32 返回标志码
     *         0000 信息异常
     *         0001 仅PC特性
     *         0010 仅平板特性
     *         0011 支持平板与PC特性
     */
    @Override
    public UInt32 getSystemProductFeatures(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemProductFeatures();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统宿主机的虚拟机类型
     * 
     * @return String 操作系统宿主机的虚拟机类型，类型如下：
     *         [none, qemu, kvm, zvm, vmware, hyper-v, orcale virtualbox, xen, bochs, \
     *          uml, parallels, bhyve, qnx, arcn, openvz, lxc, lxc-libvirt, systemd-nspawn,\
     *          docker, podman, rkt, wsl]
     *         其中 none 表示运行在物理机环境中；其他字符串代表具体的虚拟环境类型
     */
    @Override
    public String getSystemHostVirtType(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemHostVirtType();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取操作系统宿主机的云平台类型
     * 
     * @return String 获取失败返回NULL，获取成功返回一个字符串，字符串内容如下：
     *               [none, huawei]
     *               其中 none 表示运行在物理机或未知的云平台环境中；其他字符串代表不同的云平台
     */
    @Override
    public String getSystemHostCloudPlatform(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemHostCloudPlatform();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief  获取系统版本号
     * 
     * @return String 系统版本号
     */
    @Override
    public String getSystemOSVersion(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemOSVersion();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief  获取补丁版本号
     * 
     * @return String 补丁版本号
     */
    @Override
    public String getSystemUpdateVersion(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemUpdateVersion();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief  判断当前镜像系统是否为 专用机 系统
     * 
     * @return boolean true代表是 false代表不是
     */
    @Override
    public boolean getSystemIsZYJ(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            sysinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/sysinfo", sysinfo.class);
            return obj.getSystemIsZYJ();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return false;
    }

    /**
     * @brief  获取系统当前显示屏幕,分辨率,系统支持分辨率
     * 
     * @return List<String> 当前显示屏幕,分辨率,系统支持分辨率信息列表
     */
    public List<String> getSysLegalResolution(){
        ResolutionMethod obj = new ResolutionMethod();
        return obj.getSysLegalResolution();
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

    public class ResolutionMethod implements resolution{
        @Override
        public List<String> getSysLegalResolution(){
            DBusConnection conn = null;
            try {
                conn = DBusClient.getConn(DBusConnection.DBusBusType.SESSION);
                resolution obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                        "/com/lingmo/lingmosdk/resolution", resolution.class);

                return obj.getSysLegalResolution();
            } catch (DBusException e) {
                if (conn != null) conn.disconnect();
                log.error("DBusException GetEUser :",e);
            }
            return null;
        }
        @Override
        public boolean isRemote() {
            return false;
        }

        @Override
        public String getObjectPath() {
            return null;
        }
    }
}

/**
 * \example lingmosdk-system/src/lingmosdk-java/lingmosdk-java-ext/lingmosdk-java/src/test/java/lingmo/lingmosdk/java/EventTest.java
 * 
 */
