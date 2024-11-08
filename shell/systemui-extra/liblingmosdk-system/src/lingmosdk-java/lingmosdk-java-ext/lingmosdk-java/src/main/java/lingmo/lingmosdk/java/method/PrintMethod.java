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
 * @file PrintMethod.java
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief Java获取打印信息
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
import com.lingmo.lingmosdk.print;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;

import java.util.List;

/**
 * @brief 获取打印信息类
 * 主要用来获取本地的所有打印机(包含可用打印机和不可用打印机)、获取本地的所有可用打印机(打印机处于idle状态)、
 * 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印、
 * 下载网络文件、打印本地文件、清除某个打印机的所有打印队列、获取当前打印机状态(状态不是实时更新)、
 * 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）、获取url内的文件名
 */
@Slf4j
public class PrintMethod implements print {
    /**
     * @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
     * 
     * @return List<String> 地的所有打印机名称列表
     */
    @Override
    public List<String> getPrinterList(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterList();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }
    /**
     * @brief 获取本地的所有可用打印机(打印机处于idle状态)
     * 
     * @return List<String> 本地的所有可用打印机名称列表
     */
    @Override
    public List<String> getPrinterAvailableList(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterAvailableList();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
     * 
     * @param page cups属性，一张纸打印几页，如2 4等
     * @param paperType cups属性，纸张类型，如A4
     * @param cups cups属性，如lrtb
     * @param sided ups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
     */
    @Override
    public void setPrinterOptions(int page,String paperType,String cups,String sided){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            obj.setPrinterOptions(page,paperType,cups,sided);
            return;
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return;
    }

    /**
     * @brief 打印本地文件
     * 
     * @param printer 打印机名
     * @param file 文件绝对路径
     * 
     * @return int 打印作业的id
     * 
     * @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
     *         打印机处于stop状态函数返回0
     *         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
     *         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
     * 
     */
    @Override
    public int getPrinterPrintLocalFile(String printer,String file){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterPrintLocalFile(printer, file);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief 清除某个打印机的所有打印队列
     * 
     * @param printer 打印机名
     * @return int 成功返回0,失败返回-1
     */
    @Override
    public int getPrinterCancelAllJobs(String printer){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterCancelAllJobs(printer);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief 获取当前打印机状态(状态不是实时更新)
     * 
     * @param printer 打印机名
     * @return int 状态码
     */
    @Override
    public int getPrinterStatus(String printer){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterStatus(printer);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief  获取url内的文件名
     * 
     * @param  url
     * @return String 解析出来的函数名
     */
    @Override
    public String getPrinterFilename(String url){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterFilename(url);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return null;
    }

    /**
     * @brief 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
     * 
     * @param job 打印机名
     * @param id 打印作业的id
     * @return int 状态码
     */
    @Override
    public int getPrinterJobStatus(String job, int id){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterJobStatus(job,id);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
    }

    /**
     * @brief 下载网络文件
     * 
     * @param remoteFile 要打印的url
     * @param localFile 保存的文件路径
     * 
     * @return int 打印作业的id
     */
    @Override
    public int getPrinterDownloadRemoteFile(String remoteFile,String localFile){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            print obj = conn.getRemoteObject("com.lingmo.lingmosdk.service",
                    "/com/lingmo/lingmosdk/print", print.class);
            return obj.getPrinterDownloadRemoteFile(remoteFile, localFile);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException GetEUser :",e);
        }
        return 0;
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
