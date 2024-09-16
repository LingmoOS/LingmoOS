// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020} Uniontech Technology Co., Ltd.
 *
 * @file aria2rpcinterface.h
 *
 * @brief aria2 RPC 后端接口头文件
 *
 * @date 2020-05-26 11:55
 *
 * Author: denglinglong  <denglinglong@uniontech.com>
 *
 * Maintainer: denglinglong  <denglinglong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARIA2RPCINTERFACE_H
#define ARIA2RPCINTERFACE_H

#include "aria2cbtinfo.h"
#include "aria2const.h"

#include <QObject>
#include <QJsonObject>
#include <QProcess>

class QNetworkReply;
class Aria2RPCInterface : public QObject
{
    Q_OBJECT
public:
    explicit Aria2RPCInterface(QObject *parent = nullptr);
    ~Aria2RPCInterface();
    static Aria2RPCInterface *instance();

public:
    /**
     *@brief init 初始化
     *@return
     */
    bool init(); //初始化RPC服务

    /**
     * @brief addUri 添加下载 HTTP(S)/FTP/BitTorrent Magnet 链接 ，
     * @param uri 链接
     * @param opt QMap<QString, QVariant> 下载参数，配置仅应用于本次下载
     * 参数详见aria2c文档
     * 示例代码
     * QMap<QString, QVariant> opt;
     * //dir 指定文件存储的路径
     * opt.insert("dir", dir);
     * //max-download-limit 指定最大下载速度（字节/秒）速度值可以追加K或者M
     * opt.insert("max-download-limit", "100K");
     *
     * @param strId 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2const.h常量
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool addUri(QString uri, QMap<QString, QVariant> opt, QString id); //添加uri地址

    /**
     * @brief addNewUri 添加下载 HTTP(S)/FTP/BitTorrent Magnet 链接 ，
     * @param uri 链接
     * @param savepath  下载路径
     * @param filename  文件名称
     * @param strId     Gid
     */
    bool addNewUri(QString uri, QString savepath, QString filename, QString id);

    /**
     * @brief addTorrent 添加下载Torrent
     * @param torrentFile torrent种子文件路径
     * @param opt QMap<QString, QVariant> 下载参数，配置仅应用于本次下载
     * 参数详见aria2c文档
     * 示例代码
     * QMap<QString, QVariant> opt;
     * //dir 指定文件存储的路径
     * opt.insert("dir", dir);
     * //seed-time bt下载完成后做种上传的时间。默认为0不做种。如果不为0，下载会一直处于active状态儿不会完成。
     * opt.insert("seed-time", 0);
     * //max-upload-limit 指定最大上传速度（字节/秒）速度值可以追加K或者M
     * opt.insert("max-upload-limit", "100K");
     * //max-download-limit 指定最大下载速度（字节/秒）速度值可以追加K或者M
     *
     * @param id 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2const.h常量
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool addTorrent(QString torrentFile, QMap<QString, QVariant> opt, QString id); //添加bt文件

    /**
     * @brief addMetalink 添加Metalink
     * @param metalinkFile metalink种子文件路径
     * @param opt 同addTorrent
     * @param id 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2const.h常量
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool addMetalink(QString metalink, QMap<QString, QVariant> opt, QString id); //

    /**
     * @brief tellStatus 获取下载状态
     * @param gId GID aria2c生成的下载唯一标示
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回指定下载的当前状态
     * result为一个JsonObject，包含类似map键值，详见aria2文档
     */
    bool tellStatus(QString gId, QString id = "");

    /**
     * @brief tellStatus 获取下载状态 可指定需要的返回字段
     * @param gId GID aria2c生成的下载唯一标示
     * @param keys 指定需要的返回字段列表，参数详见aria2文档
     * [status|totalLength|completedLength|uploadLength|bitfield|downloadSpeed|uploadSpeed|...]
     * 示例代码：
     * QStringList keys;
     * keys<<"gid"<<"status"<<"totalLength"<<"completedLength"<<"downloadSpeed";
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回指定下载的当前状态
     * result为一个JsonObject，包含类似map键值，详见aria2文档
     */
    bool tellStatus(QString gId, QStringList keys, QString id = "");

    /**
     * @brief pause 暂停指定下载 active/waiting状态将变为paused
     * @param gId GID
     * @param id 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2cconst.h常量
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool pause(QString gId, QString id = "");

    /**
     * @brief forcePause 强制暂停 active/waiting状态将变为paused
     * @param gId GID aria2c生成的下载唯一标示
     * @param id 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2cconst.h常量
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool forcePause(QString gId, QString id = "");

    /**
     * @brief pauseAll 全部暂停 active/waiting状态将变为paused
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回 "OK"
     * {id:"", jsonrpc:"2.0", result:"ok"}
     * QString ok = json.value("result").toString();
     */
    bool pauseAll(QString id = ""); //

    /**
     * @brief forcePauseAll 强制全部暂停
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回 "OK"
     * {id:"", jsonrpc:"2.0", result:"ok"}
     * QString ok = json.value("result").toString();
     */
    bool forcePauseAll(QString id = "");

    /**
     * @brief unpause 恢复指定GID下载 paused状态变为waiting
     * @param gId GID aria2c生成的下载唯一标示
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool unpause(QString gId, QString id = "");

    /**
     * @brief unpauseAll 全部恢复下载
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回 "OK"
     * {id:"", jsonrpc:"2.0", result:"ok"}
     * QString ok = json.value("result").toString();
     */
    bool unpauseAll(QString id = "");

    /**
     * @brief remove 移除指定下载。下载状态会变为removed。
     * 注意，实验表明并不会删除下载的文件。
     * @param gId GID aria2c生成的下载唯一标示
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool remove(QString gId, QString id = "");

    /**
     * @brief forceRemove 强制移除,不会做后处理
     * @param gId GID aria2c生成的下载唯一标示
     * @param id 同其它方法参数
     *
     * signal_success信号中异步返回本次下载的GID
     * {id:"", jsonrpc:"2.0", result:"gid"}
     * QString gId = json.value("result").toString();
     */
    bool forceRemove(QString gId, QString id = "");

    /**
     * @brief removeDownloadResult 移出指定下载项下载结果（包括已完成/下载错误/已移出）,不会删除文件
     * @param gId
     * @param id
     *
     * signal_success信号中异步返回 OK
     */
    bool removeDownloadResult(QString gId, QString id = "");

    /**
     * @brief getFiles 获取文件列表
     * @param gId
     * @param id
     *
     * signal_success信号中异步返回,参数详见aria2文档
     */
    bool getFiles(QString gId, QString id = "");

    /**
     * @brief getGlobalSatat 获取全局状态
     * signal_success信号中异步返回,参数详见aria2文档
     */
    bool getGlobalSatat();

    /**
     * @brief getGlobalOption 获取全局状态
     * signal_success信号中异步返回,参数详见aria2文档
     */
    bool getGlobalOption();

    /**
     * @brief modifyConfigFile 写入配置文件
     * @param config_item 配置项
     * @param value 配置值
     *
     * signal_success信号中异步返回，参数结构见aria2文档
     */
    bool modifyConfigFile(QString configItem, QString value);

    /**
     * @brief changeGlobalOption 变更全局配置
     * @param options 配置项键值map
     * @param id
     *
     * signal_success信号中异步返回，参数结构见aria2文档
     */
    bool changeGlobalOption(QMap<QString, QVariant> options, QString id = "");

    /**
     * @brief changeGlobalOption 修改指定任务在aria2下载队列中的位置
     * @param options 配置项键值map
     * @param id
     *
     * signal_success信号中异步返回，参数结构见aria2文档
     */
    bool changePosition(QString gId, int pos);

    /**
     * @brief setMaxDownloadNum 设置最大任务数
     * @param maxDownload
     *
     * 调用changeglobaloption设置,参数详见aria2文档
     */
    bool setMaxDownloadNum(QString maxDownload);

    /**
     * @brief setDownloadUploadSpeed 下载和上传速度的设置
     * @param downloadSpeed 下载速度
     * @param uploadSpeed   上传速度
     *
     */
    bool setDownloadUploadSpeed(QString downloadSpeed, QString uploadSpeed);

    /**
     * @brief SetDisckCacheNum 设置磁盘缓存大小
     * @param diskCacheNum 缓存大小
     *
     */
    bool SetDisckCacheNum(QString disckCacheNum);

    /**
     * @brief setDownloadLimitSpeed 设置最大下载速度
     * @param downloadlimitSpeed
     *
     */
    void setDownloadLimitSpeed(QString downloadLimitSpeed);

    /**
     * @brief setUploadLimitSpeed 设置最大上传速度
     * @param UploadlimitSpeed
     *
     */
    bool setUploadLimitSpeed(QString UploadLimitSpeed);

    /**
     * @brief getBtToMetalink bt文件转磁力链
     * @param strFileName bt文件名
     *
     */
    QString getBtToMetalink(QString filePath);

    /**
     * @brief bytesFormat 格式化字节
     * @param size
     * @return
     */
    QString bytesFormat(qint64 size);

    /**
     * @brief purgeDownloadResult 清除已下载结果（包括已完成/下载错误/已移出）,不会删除文件
     * @param id
     *
     * signal_success信号中异步返回 OK
     */
    bool purgeDownloadResult(QString id = "");

    /**
     * @brief getBtInfo  获取torrent信息
     * @param strTorrentPath  bt文件路径
     * @return Aria2cBtInfo
     */
    Aria2cBtInfo getBtInfo(QString torrentPath); //得到bt文件信息

    /**
     * @brief getCapacityFree 获取指定路径所在分区的磁盘剩余容量
     * @param path
     * @return
     */
    QString getCapacityFree(QString path);

    /**
     * @brief getCapacityFreeByte 获取指定路径所在分区的磁盘剩余容量(字节)
     * @param path
     * @return
     */
    long getCapacityFreeByte(QString path);
    /**
     * @brief shutdown 停止aria2c程序
     * 调用后会给aria2c发送shutdown消息。
     * signal_success信号中异步返回 "OK"
     * {id:"", jsonrpc:"2.0", result:"ok"}
     * QString ok = json.value("result").toString();
     *
     * @param id 可选，该参数用来唯一标示一次请求，异步返回的结果里会包含请求发出是指定的id，用以区分匹配多次请求。
     * 默认为method名，详见aria2cconst.h常量
     * QString id = json.value("id").toString();
     */
    bool shutdown(QString id = "");

    /**
     * @brief forceShutdown 强制停止aria2c程序
     * @param id
     * 同shutdown，
     */
    bool forceShutdown(QString id = "");

private:
    /**
     *@brief 设置配置文件目录
     *@return
     */
    bool setupConfig();

    /**
     *@brief startUp 启动aria2c进程
     *@return 启动成功true  启动失败 false
     */
    bool startUp();
    /**
     *@brief 调用RPC
     *@param method 调用的方法 params  该方法对应的参数   id
     *
     *@return
     */
    bool callRPC(QString method, QJsonArray params, QString id = "");

    /**
     *@brief 调用RPC
     *@param method 调用的方法   id
     *
     *@return
     */
    bool callRPC(QString method, QString id = "");

    /**
     *@brief 发送请求
     *@param jsonObj json包 method请求的方法
     *
     *@return
     */
    bool sendMessage(QJsonObject jsonObj, const QString &method);

    /**
     *@brief 请求的返回
     *@param reply网络对象 method 方法 id
     *
     *@return
     */
    void rpcRequestReply(QNetworkReply *reply, const QString &method, const QString id);

    /**
     *@brief 文件转base64
     *@param filePath 文件路径
     *
     *@return 获得转换后的文件
     */
    QString fileToBase64(QString filePath); //文件转base64

    /**
     *@brief 如果是迅雷链接会解密处理，否则原样返回
     *@param thunder 链接
     *
     *@return 解密后链接
     */
    QString processThunderUri(QString);

    /**
     *@brief checkAria2cProc() 检测aria2c是否启动
     *@return 启动成功 true  否则 false
     */
    bool checkAria2cProc();

    /**
     *@brief killAria2cProc关闭aria2c进程
     *@return 成功 大于0  否则 小于0
     */
    int killAria2cProc();

    /**
     *@brief 设置默认的下载路径
     *@param strDir
     *@return
     */
    void setDefaultDownLoadDir(QString dir);

    /**
     *@brief 获得默认的下载路径
     *@return
     */
    QString getDefaultDownLoadDir(); //{return this->defaultDownloadPath();}

    /**
     *@brief 设置配置文件路径
     *@param strPath
     *@return
     */
    void setConfigFilePath(const QString path);

    /**
     *@brief 获得配置文件路径
     *@param
     *@return 返回路径
     */
    QString getConfigFilePath() const;

    /**
     *@brief 检查aria2c文件
     *@param
     *@return aria2c 存在返回true 不存在返回false
     */
    bool checkAria2cFile();

    /**
     * @brief getUUID 获取thken
     * @return token
     */
    QString getToken();

signals:
    void RPCSuccess(QString method, QJsonObject json); //rpc 正确返回处理信号
    void RPCError(QString method, QString id, int errCode, QJsonObject obj); //rpc错误返回处理信号

private:
    static Aria2RPCInterface *m_instance;
    QString m_rpcPort = "16800"; //rpc端口
    QString m_rpcServer = "http://localhost:" + m_rpcPort + "/jsonrpc"; //rpc服务器地址
    QString m_defaultDownloadPath; //默认下载路径
    QString m_configPath = ""; //配置文件路径
    const QString m_aria2cCmd; //aria2c程序路径
    const QString m_basePath; //下载器安装目录
    QProcess *m_proc;
};

#endif // ARIA2RPCINTERFACE_H
