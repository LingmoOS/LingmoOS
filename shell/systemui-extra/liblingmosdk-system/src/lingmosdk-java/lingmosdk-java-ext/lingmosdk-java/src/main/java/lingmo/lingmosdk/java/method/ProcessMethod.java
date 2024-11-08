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
 * @file ProcessMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取某一进程的瞬时详细信息
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
import com.lingmo.lingmosdk.process;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import java.util.List;

/**
 * @brief 获取某一进程的瞬时详细信息类
 * 主要用来获取某一进程的CPU使用率、获取某一进程的内存占用率、获取某一进程的进程状态、获取某一进程的端口号占用、
 * 获取某一进程的启动时间、获取某一进程的运行时间、获取某一进程的cpu时间、获取某一进程的Command、获取某一进程的属主、
 * 获取某一进程的信息、获取进程所有信息
 */
@Slf4j
public class ProcessMethod implements process {
    /**
     * @brief 获取某一进程的CPU使用率
     * 
     * @param proc 进程号
     * @return double 进程的CPU使用率
     */
    @Override
    public double getProcInfoCpuUsage(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoCpuUsage(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    
    // @Override
    // public double getProcInfoIoUsage(int proc){
    //     DBusConnection conn = null;
    //     try {
    //         conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
    //         process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
    //                 "/com/lingmo/lingmosdk/process", process.class);
    //         return obj.getProcInfoIoUsage(proc);
    //     } catch (DBusException e) {
    //         if (conn != null) conn.disconnect();
    //         log.error("DBusException GetEUser :",e);
    //     }
    //     return -1.0;
    // }

    /**
     * @brief 获取某一进程的内存占用率
     * 
     * @param proc 进程号
     * @return double 进程的内存占用率
     */
    @Override
    public double getProcInfoMemUsage(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoMemUsage(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1.0;
    }

    /**
     * @brief 获取某一进程的进程状态
     * 
     * @param proc 进程号
     * @return String 进程状态
     */
    @Override
    public String getProcInfoStatus(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoStatus(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的端口号占用
     * 
     * @param proc 进程号
     * @return int 进程使用的端口号
     */
    @Override
    public int getProcInfoPort(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoPort(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1;
    }

    /**
     * @brief 获取某一进程的启动时间
     * 
     * @param proc 进程号
     * @return String 进程的启动时间
     */
    @Override
    public String getProcInfoStartTime(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoStartTime(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的运行时间
     * 
     * @param proc 进程号
     * @return String 进程的运行时间
     */
    @Override
    public String getProcInfoRunningTime(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoRunningTime(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的cpu时间
     * 
     * @param proc 进程号
     * @return String 进程的cpu时间
     */
    @Override
    public String getProcInfoCpuTime(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoCpuTime(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的Command
     * 
     * @param proc 进程号
     * @return String 进程的Command
     */
    @Override
    public String getProcInfoCmd(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoCpuTime(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的属主
     * 
     * @param proc 进程号
     * @return String 进程的属主
     */
    @Override
    public String getProcInfoUser(int proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfoUser(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取某一进程的信息
     * 
     * @param proc 进程名
     * @return List<String> 某一进程CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
     */
    @Override
    public List<String> getProcInfo(String proc){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getProcInfo(proc);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取进程所有信息
     * 
     * @return List<String> 所有进程的CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
     */
    @Override
    public List<String> getAllProcInfo(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            process obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/process", process.class);
            return obj.getAllProcInfo();
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
