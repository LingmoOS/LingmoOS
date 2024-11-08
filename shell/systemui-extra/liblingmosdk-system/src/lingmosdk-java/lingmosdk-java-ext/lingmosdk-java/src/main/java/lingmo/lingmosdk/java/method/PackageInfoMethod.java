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
 * @file PackageInfoMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取包列表信息
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
import  com.lingmo.lingmosdk.packageinfo;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;

import java.util.List;

/**
 * @brief 获取包列表信息类
 * 主要用来获取系统中所有包列表、获取系统中指定包的版本号、检测指定包名的软件包是否正确安装
 */
@Slf4j
public class PackageInfoMethod implements packageinfo{
    /**
     * @brief 获取系统中所有包列表
     * 
     * @return List<String> 系统中所有包列表
     */
    @Override
    public List<String> getPackageList(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            packageinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/packageinfo", packageinfo.class);
            return obj.getPackageList();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取系统中指定包的版本号
     * 
     * @param name 软件包名
     * @return String 版本号
     */
    @Override
    public String getPackageVersion(String name){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            packageinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/packageinfo", packageinfo.class);
            return obj.getPackageVersion(name);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 检测指定包名的软件包是否正确安装
     * 
     * @param name 包名
     * @param version 版本号，大部分情况下为NULL，预留为snap做准备
     * @return int 成功返回0，失败返回错误码
     */
    @Override
    public int getPackageInstalled(String name,String version){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            packageinfo obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/packageinfo", packageinfo.class);
            return obj.getPackageInstalled(name,version);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return -1;
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
