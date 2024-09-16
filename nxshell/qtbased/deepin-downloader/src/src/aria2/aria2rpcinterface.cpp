// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020} Uniontech Technology Co., Ltd.
 *
 * @file aria2rpcinterface.h
 *
 * @brief aria2 RPC 后端接口实现
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

#include "aria2rpcinterface.h"

#include "log.h"
#include "config.h"
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QCryptographicHash>
#include <QtMath>
#include <QUuid>

Aria2RPCInterface *Aria2RPCInterface::m_instance = new Aria2RPCInterface;

Aria2RPCInterface *Aria2RPCInterface::instance()
{
    return m_instance;
}

Aria2RPCInterface::Aria2RPCInterface(QObject *parent)
    : QObject(parent)
    , m_aria2cCmd(ARIA2C_NAME)
    , m_basePath(ARIA2C_PATH)
{
}

Aria2RPCInterface::~Aria2RPCInterface()
{
    delete m_proc;
}

bool Aria2RPCInterface::startUp()
{
    /*
     *检测aria2c执行文件是否存在
     */
    if (!checkAria2cFile()) {
        qDebug() << "未发现" << m_basePath + m_aria2cCmd;
        return false;
    }

    /*
     *检测aria2c进程是否启动 如果启动,杀它,杀死.
     */
    bool bCheck = checkAria2cProc();
    if (checkAria2cProc()) {
        qDebug() << m_aria2cCmd + "进程已存在,killAria2cProc()";
        killAria2cProc();
    }

    /*
    *设置aria2c  session 路径  时间  input 路径
    */
    QString sessionCacheFile = QDir::homePath() + "/.cache/uos-aria2c.session"; //session 文件路径
    QString inputFile = QDir::homePath() + "/.cache/uos-aria2c.input"; //.input文件路径
    QString dhtFile = QDir::homePath() + "/.config/uos/downloader/dht.dat"; //
    QString dht6File = QDir::homePath() + "/.config/uos/downloader/dht6.dat"; //
    QString saveSessionInterval = "30"; //秒

    qDebug() << "创建session缓存文件: " << sessionCacheFile;
    QProcess::execute("touch", QStringList() << sessionCacheFile); //创建session缓存文件
    //QProcess::execute("touch", QStringList() << dhtFile); //创建dht文件
    //QProcess::execute("touch", QStringList() << dht6File); //创建dht6文件

    QString opt;
    opt += " --enable-rpc=true"; //启动RPC
    opt += " --rpc-secret=" + getToken();
    opt += " --rpc-listen-port=" + this->m_rpcPort; //RPC监听的端口
    opt += " --check-certificate=false"; //停用rpc身份验证
    opt += " --rpc-allow-origin-all=true"; // 允许所有来源
    opt += " --rpc-max-request-size=99999999"; //设置rpc最大接收数
    opt += " --rpc-save-upload-metadata=true"; //

    //opt += " --not-conf=true";//不使用配置文件
    if (!this->m_configPath.isEmpty()) {
        opt += " --conf-path=" + this->m_configPath; //加载指定的配置文件
    }
    if (!this->m_defaultDownloadPath.isEmpty()) {
        opt += " --dir=" + this->m_defaultDownloadPath; //配置默认下载路径。优先级高于配置文件，已移动到配置文件中
    }
    opt += " --continue=true"; //http续传配置
    opt += " --disable-ipv6"; //禁用ipv6
    //opt += " --seed-time=0";//bt完成不做种
    opt += " --bt-metadata-only=true"; //仅下载bt metadata，不自动发起follow下载
    opt += " --bt-save-metadata=true"; //保存magnet metadata到同目录下.torrent文件
    opt += " --follow-torrent=false"; //当下载的文件是以.torrent结尾的，是否继续下载。true，是；false，否，只下载torrent文件；mem,不写文件保存在内存
    //opt += " --follow-metalink=false";//类似torrent
    opt += " --bt-remove-unselected-file=true";
    //opt += " --input-file=" + inputFile;
    opt += " --save-session=" + sessionCacheFile;
    opt += " --save-session-interval=" + saveSessionInterval;
    opt += " --enable-dht=true"; //启动dht文件
    opt += " --enable-dht6=false"; //禁用dht6文件
    opt += " --dht-file-path=" + dhtFile;
    opt += " --dht-file-path6=" + dht6File;
    opt += " --follow-metalink=false";
    if(QSysInfo::currentCpuArchitecture() == "loongarch64"){
        opt += " --async-dns=false";
    }

   // qDebug() << m_basePath + m_aria2cCmd << opt.join(' ');

    QProcess proc; // = new QProcess;
    proc.setStandardOutputFile("/dev/null");
    proc.setStandardErrorFile("/dev/null");
    proc.startDetached("sh -c \"" + m_basePath + m_aria2cCmd + " " + opt + "\"");
    proc.waitForStarted();

//    proc.start("ulimit -n 40");
//    bool b = proc.waitForFinished();
//    QByteArray sssss = proc.readAllStandardError();
//    QByteArray wwwww = proc.readAllStandardOutput();

//    proc.start("ulimit -n");
//    int r = proc.waitForFinished(-1);
//    int r1 = proc.waitForReadyRead();
//    sssss = proc.readAllStandardError();
//    wwwww = proc.readAllStandardOutput();

    bCheck = checkAria2cProc();
    qDebug() << "启动aria2c完成！ " << proc.state() << bCheck;
    return bCheck;
}

bool Aria2RPCInterface::checkAria2cFile()
{
    QFile file(m_basePath + m_aria2cCmd);
    return file.exists();
}

bool Aria2RPCInterface::Aria2RPCInterface::init()
{
    setupConfig();
    bool rs = startUp();
    qDebug() << "Startup aria2:" << QString::number(rs);
    return rs;
}

bool Aria2RPCInterface::checkAria2cProc()
{
    QProcess proc;
    QStringList opt;
    opt << "-c";
    //opt << "ps aux | grep aria2c";
    opt << "ps aux|grep " + m_aria2cCmd;
    proc.start("/bin/bash", opt);
    proc.waitForFinished();
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    QStringList lineList = output.split("\n");
    int cnt = 0;
    for (QString t : lineList) {
        if (t.isEmpty()) {
            continue;
        }
        if (t.indexOf("grep " + m_aria2cCmd) >= 0) {
            continue;
        }
        if (t.indexOf(m_aria2cCmd) >= 0) {
            cnt++;
            //break;
        }
    }
    if (cnt > 0) {
        return true;
    } else {
        return false;
    }
}

int Aria2RPCInterface::killAria2cProc()
{
    QStringList opt;
    opt << "-c";
    opt << "ps -ef|grep " + m_aria2cCmd + "|grep -v grep|awk '{print $2}'|xargs kill -9";
    return QProcess::execute("/bin/bash", opt);
}

void Aria2RPCInterface::setDefaultDownLoadDir(QString dir)
{
    this->m_defaultDownloadPath = dir;
}

QString Aria2RPCInterface::getDefaultDownLoadDir()
{
    return m_defaultDownloadPath;
}

void Aria2RPCInterface::setConfigFilePath(const QString path)
{
    m_configPath = path;
}

QString Aria2RPCInterface::getConfigFilePath() const
{
    return m_configPath;
}

bool Aria2RPCInterface::addUri(QString uri, QMap<QString, QVariant> opt, QString id)
{
    if (uri.isEmpty() || opt.isEmpty() || id.isEmpty()) {
        return false;
    }
    uri = processThunderUri(uri); //处理迅雷链接
    QJsonArray ja, inner;
    inner.append(uri); //可支持多个URI
    ja.append(inner);

    QJsonDocument doc = QJsonDocument::fromVariant(QVariant(opt));
    QJsonObject optJson = doc.object();
    ja.append(optJson);

    return callRPC(ARIA2C_METHOD_ADD_URI, ja, id);
}

bool Aria2RPCInterface::addNewUri(QString uri, QString savepath, QString filename, QString id)
{
    if (uri.isEmpty() || savepath.isEmpty() || filename.isEmpty() || id.isEmpty()) {
        return false;
    }
    QMap<QString, QVariant> opt;
    opt.insert("dir", savepath);
    opt.insert("out", filename);

    return addUri(uri, opt, id);

    //qDebug() << "Add new uri" << uri;
}

bool Aria2RPCInterface::addTorrent(QString torrentFile, QMap<QString, QVariant> opt, QString id)
{
    if (torrentFile.isEmpty() || opt.isEmpty() || id.isEmpty()) {
        return false;
    }
    QString torrentB64Str = fileToBase64(torrentFile); //把bt文件转成base64编码
    QJsonArray ja;
    ja.append(torrentB64Str);
    ja.append(QJsonArray());

    QJsonDocument doc = QJsonDocument::fromVariant(QVariant(opt));
    QJsonObject optJson = doc.object();
    ja.append(optJson);

    return callRPC(ARIA2C_METHOD_ADD_TORRENT, ja, id);
}

bool Aria2RPCInterface::addMetalink(QString metalink, QMap<QString, QVariant> opt, QString id)
{
    if (metalink.isEmpty() || opt.isEmpty() || id.isEmpty()) {
        return false;
    }
    QString metalinkB64Str = fileToBase64(metalink);
    QJsonArray ja;
    ja.append(metalinkB64Str);
    //ja.append(QJsonArray());

    QJsonDocument doc = QJsonDocument::fromVariant(QVariant(opt));
    QJsonObject optJson = doc.object();
    ja.append(optJson);

    return callRPC(ARIA2C_METHOD_ADD_METALINK, ja, id);
}

QString Aria2RPCInterface::fileToBase64(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    QByteArray ba = file.readAll();
    QString b64Str = ba.toBase64();
    return b64Str;
}

bool Aria2RPCInterface::purgeDownloadResult(QString id)
{
    return callRPC(ARIA2C_METHOD_PURGE_DOWNLOAD_RESULT, id);
}

Aria2cBtInfo Aria2RPCInterface::getBtInfo(QString torrentPath)
{
    QProcess Proc; //进程调用指针
    QStringList opt;
    opt << "--show-files=true";
    opt << torrentPath;
    Proc.start(m_basePath + m_aria2cCmd, opt); //启动aria2c进程
    Proc.waitForFinished(); //等待执行完成

    QByteArray array = Proc.readAllStandardOutput(); //获取进程执行返回值
    Proc.close(); //关闭进程

    QStringList strList = QString(array).split("\n"); //将array内容分割开了

    Aria2cBtInfo btInfo;
    bool bFlag = false;
    QString temp = "";

    for (QString line : strList) {
        if (line.startsWith("Mode: ")) {
            btInfo.mode = line.mid(6);
        } else if (line.startsWith("Announce:")) {
            continue;
        } else if (!bFlag && line.startsWith(" ")) {
            btInfo.announceList.append(line.mid(1));
        } else if (line.startsWith("Info Hash: ")) {
            btInfo.infoHash = line.mid(11);
        } else if (line.startsWith("Piece Length: ")) {
            btInfo.pieceLength = line.mid(14).remove('i');
        } else if (line.startsWith("The Number of Pieces: ")) {
            btInfo.pieceNumber = line.mid(22).toInt();
        } else if (line.startsWith("Total Length: ")) {
            QString tpl = line.mid(14);
            QStringList sp = tpl.split(" ");
            btInfo.totalLength = sp[0].remove('i'); //MiB==>MB

            QString len = sp[1].remove(',');
            QString len2 = len.mid(1, len.length() - 2);
            btInfo.totalLengthByets = len2.toLong();
        } else if (line.startsWith("Name: ")) {
            btInfo.name = line.mid(6);
        } else if (line.startsWith("Magnet URI: ")) {
            btInfo.magnet = line.mid(12);
        } else if (line.startsWith("Files:")) {
            continue;
        } else if (line.startsWith("idx")) {
            continue;
        } else if (line.startsWith("===+===")) {
            bFlag = true;
            temp = "";
            continue;
        } else if (bFlag && line.startsWith("---+---")) { //
            QStringList stl = temp.split("|");
            Aria2cBtFileInfo f;
            f.index = stl[0].toInt();
            f.path = stl[1];
            QStringList spl = stl[2].split(" ");
            f.length = spl[0].remove('i');

            QString len = spl[1].remove(',');
            QString len2 = len.mid(1, len.length() - 2);
            f.lengthBytes = len2.toLong();
            btInfo.files.append(f);
            temp = "";
        } else if (bFlag) { //idx|fileName
            temp += line.trimmed();
        }
    }
    if (btInfo.totalLengthByets == 0) {
        for (int i = 0; i < btInfo.files.size(); i++) {
            btInfo.totalLengthByets += btInfo.files[i].lengthBytes;
        }
    }
    return btInfo;
}

bool Aria2RPCInterface::callRPC(QString method, QJsonArray params, QString id)
{
    QJsonObject json;
    params.prepend("token:" + getToken());
    json.insert("jsonrpc", "2.0");
    if (id.isEmpty()) {
        json.insert("id", method);
    } else {
        json.insert("id", id);
    }
    json.insert("method", method);
    if (!params.isEmpty()) {

        json.insert("params", params);
    }
    json.insert("", 0);
    return sendMessage(json, method);
}

bool Aria2RPCInterface::callRPC(QString method, QString id)
{
    return callRPC(method, QJsonArray(), id);
}

bool Aria2RPCInterface::sendMessage(QJsonObject jsonObj, const QString &method)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager; //定义网络对象

    if (!jsonObj.isEmpty()) { //json如果不为空
        QNetworkRequest requset; //定义请求对象
        requset.setUrl(QUrl(this->m_rpcServer)); //设置服务器的uri
        requset.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply *networkReply = manager->post(requset, QJsonDocument(jsonObj).toJson()); //post信息到服务器

        //调用返回的信息
        connect(manager,
                &QNetworkAccessManager::finished,
                this,
                [=](QNetworkReply *reply) {
                    this->rpcRequestReply(reply, method, jsonObj.value("id").toString()); //调用出来函数
                    manager->deleteLater(); //删除
                    //manager->destroyed();
                });

        //        connect(networkReply,
        //                QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
        //                [=](QNetworkReply::NetworkError error){
        //                    QByteArray buf = networkReply->readAll();
        //                    qDebug() << error << ":  " << buf;
        //                });
        //        connect(networkReply,
        //                &QNetworkReply::finished,
        //                [=](){
        //                    QByteArray buf = networkReply->readAll();
        //                    qDebug() << "finished" << ":  " << buf;
        //                });
        return networkReply->error() ? false : true;
    }

    if (manager != nullptr) {
        manager->deleteLater();
        manager = nullptr;
    }

    return false;
}

void Aria2RPCInterface::rpcRequestReply(QNetworkReply *reply, const QString &method, const QString id)
{
    if (method == ARIA2C_METHOD_FORCE_PAUSE) {
        method.size();
    }
    if (method == ARIA2C_METHOD_PAUSE) {
        method.size();
    }
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); //请求返回的属性值
    QByteArray buf = reply->readAll(); //获取信息
    QJsonDocument doc = QJsonDocument::fromJson(buf); //转换为json格式
    QJsonObject obj = doc.object();
    if (code == 200) { //返回正常
        emit RPCSuccess(method, obj);
    } else { //错误
        emit RPCError(method, id, code, obj);
    }

    reply->deleteLater();
    //reply->destroyed();
}

bool Aria2RPCInterface::tellStatus(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_TELL_STATUS, ja, id);
}

bool Aria2RPCInterface::tellStatus(QString gId, QStringList keys, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    QJsonArray ka;
    for (QString k : keys) {
        ka.append(k);
    }
    ja.append(ka);
    return callRPC(ARIA2C_METHOD_TELL_STATUS, ja, id);
}

bool Aria2RPCInterface::pause(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_PAUSE, ja, id);
}

bool Aria2RPCInterface::forcePause(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_FORCE_PAUSE, ja, id);
}

bool Aria2RPCInterface::pauseAll(QString id)
{
    return callRPC(ARIA2C_METHOD_PAUSE_ALL, id);
}

bool Aria2RPCInterface::forcePauseAll(QString id)
{
    return callRPC(ARIA2C_METHOD_FORCE_PAUSE_ALL, id);
}

bool Aria2RPCInterface::unpause(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_UNPAUSE, ja, id);
}

bool Aria2RPCInterface::unpauseAll(QString id)
{
    return callRPC(ARIA2C_METHOD_UNPAUSE_ALL, id);
}

bool Aria2RPCInterface::remove(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_REMOVE, ja, id);
}

bool Aria2RPCInterface::forceRemove(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_FORCE_REMOVE, ja, id);
}

bool Aria2RPCInterface::removeDownloadResult(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_REMOVE_DOWNLOAD_RESULT, ja, id);
}

bool Aria2RPCInterface::getFiles(QString gId, QString id)
{
    QJsonArray ja;
    ja.append(gId);
    return callRPC(ARIA2C_METHOD_GET_FILES, ja, id);
}

bool Aria2RPCInterface::getGlobalSatat()
{
    QJsonArray ja;
    return callRPC(ARIA2C_METHOD_GET_GLOBAL_STAT, ja, "");
}

bool Aria2RPCInterface::getGlobalOption()
{
    QJsonArray ja;
    return callRPC(ARIA2C_METHOD_GET_GLOBAL_OPTION, ja, "");
}

bool Aria2RPCInterface::changeGlobalOption(QMap<QString, QVariant> options, QString id)
{
    QJsonArray ja;
    QJsonDocument doc = QJsonDocument::fromVariant(QVariant(options));
    QByteArray jba = doc.toJson();
    QString jsonString = QString(jba);
    QByteArray njba = jsonString.toUtf8();
    QJsonObject nobj = QJsonObject(QJsonDocument::fromJson(njba).object());
    QJsonObject optJson = doc.object();
    ja.append(nobj);
    return callRPC(ARIA2C_METHOD_CHANGE_GLOBAL_OPTION, ja, id);
}

bool Aria2RPCInterface::changePosition(QString gId, int pos)
{
    QJsonArray ja;
    ja.append(gId);
    ja.append(pos);
    ja.append("POS_SET");
    return callRPC(ARIA2C_METHOD_CHANGE_POSITION, ja, gId);
}

bool Aria2RPCInterface::modifyConfigFile(QString configItem, QString value)
{
    QString strAll;
    QStringList strList;

    QString m_aria2configPath = QString("%1/%2/%3/aria2.conf")
                                    .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                    .arg(qApp->organizationName())
                                    .arg(qApp->applicationName());

    QFile readFile(m_aria2configPath);
    if (readFile.open((QIODevice::ReadOnly | QIODevice::Text))) {
        QTextStream stream(&readFile);
        strAll = stream.readAll();
    } else {
        return false;
    }
    readFile.close();
    QFile writeFile(m_aria2configPath);
    if (writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&writeFile);
        strList = strAll.split("\n");
        for (int i = 0; i < strList.count(); i++) {
            if (strList.at(i).contains(configItem)) {
                QString tempStr = strList.at(i);
                tempStr.replace(0, tempStr.length(), value);
                stream << tempStr << '\n';
            } else {
                if (i == strList.count() - 1) {
                    //最后一行不需要换行
                    stream << strList.at(i);
                } else {
                    stream << strList.at(i) << '\n';
                }
            }
        }
    }
    writeFile.close();
    return true;
}

bool Aria2RPCInterface::setMaxDownloadNum(QString maxDownload)
{
    QMap<QString, QVariant> opt;
    QString value = "max-concurrent-downloads=" + maxDownload;
    modifyConfigFile("max-concurrent-downloads=", value);
    opt.insert("max-concurrent-downloads", maxDownload);
    return changeGlobalOption(opt);
}

bool Aria2RPCInterface::setDownloadUploadSpeed(QString downloadSpeed, QString uploadSpeed)
{
    QMap<QString, QVariant> opt;
    QString down_speed = downloadSpeed + "K";
    opt.insert("max-overall-download-limit", down_speed);

    QString upload_speed = uploadSpeed + "K";
    opt.insert("max-overall-upload-limit", upload_speed);
    changeGlobalOption(opt);
    QString value = "max-overall-download-limit=" + down_speed;
    modifyConfigFile("max-overall-download-limit=", value);

    value = "max-overall-upload-limit=" + upload_speed;
    return modifyConfigFile("max-overall-upload-limit=", value);
}

bool Aria2RPCInterface::SetDisckCacheNum(QString disckCacheNum)
{
    QMap<QString, QVariant> opt;
    QString cacheNum = disckCacheNum + "M";
    opt.insert("disk-cache", cacheNum);
    changeGlobalOption(opt);
    QString value = "disk-cache=" + cacheNum;
    return modifyConfigFile("disk-cache=", value);
}

void Aria2RPCInterface::setDownloadLimitSpeed(QString downloadLimitSpeed)
{
    QMap<QString, QVariant> opt;

    QString speed = downloadLimitSpeed + "K";

    opt.insert("max-overall-download-limit", speed);
    changeGlobalOption(opt);

    QString value = "max-overall-download-limit=" + speed;
    modifyConfigFile("max-overall-download-limit=", value);

    qDebug() << "set download limit speed:" << downloadLimitSpeed;
}

bool Aria2RPCInterface::setUploadLimitSpeed(QString UploadLimitSpeed)
{
    QMap<QString, QVariant> opt;
    QString speed = UploadLimitSpeed + "K";
    opt.insert("max-overall-upload-limit", speed);
    changeGlobalOption(opt);

    QString value = "max-overall-upload-limit=" + speed;
    return modifyConfigFile("max-overall-upload-limit=", value);
}

QString Aria2RPCInterface::processThunderUri(QString thunder)
{
    QString uri = thunder;
    if (thunder.startsWith("thunder://")) {
        QString oUir = thunder.mid(thunder.indexOf("thunder://") + 9 + 1);
        uri = QString(QByteArray::fromBase64(oUir.toLatin1()));
        uri = uri.mid(2, uri.length() - 4); //AA[URI]ZZ
    }
    return uri;
}

//QString Aria2RPCInterface::getCapacityFree(QString path)
//{
//    QProcess *proc = new QProcess;
//    QStringList opt;
//    opt << "-c";
//    opt << "df -h " + path;
//   // opt << "df";
//    proc->start("/bin/bash", opt);
//  //  proc->start("df", QStringList());
//    proc->waitForFinished();
//    QByteArray rt = proc->readAllStandardOutput();
//    proc->close();
//    delete proc;

//    QString free = "0B";
//    QStringList lt = QString(rt).split("\n");
//    if (lt.length() >= 2) {
//        QString line = lt.at(1);
//        QString temp;
//        QStringList tpl;
//        for (int i = 0; i < line.length(); i++) {
//            if (line[i] != ' ') {
//                temp.append(line[i]);
//            } else {
//                if (temp != "") {
//                    tpl.append(temp);
//                    temp = "";
//                }
//            }
//        }
//        free = tpl[3];
//    }
//    return free + "B";
//}

QString Aria2RPCInterface::getCapacityFree(QString path)
{
    QProcess *proc = new QProcess;
    QStringList opt;
    opt << "-h" << path;
    // opt << "df";
    proc->start("df", opt);
    //  proc->start("df", QStringList());
    proc->waitForFinished();
    QByteArray rt = proc->readAllStandardOutput();
    proc->close();
    delete proc;

    QString free = "0B";
    QStringList lt = QString(rt).split("\n");
    if (lt.length() >= 2) {
        QString line = lt.at(1);
        QString temp;
        QStringList tpl;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] != ' ') {
                temp.append(line[i]);
            } else {
                if (!temp.isEmpty()) {
                    tpl.append(temp);
                    temp = "";
                }
            }
        }
        free = tpl[3];
    }
    return free + "B";
}

long Aria2RPCInterface::getCapacityFreeByte(QString path)
{
    QProcess *proc = new QProcess;
    QStringList opt;
    opt << "-c";
    opt << "df " + path;
    proc->start("/bin/bash", opt);
    proc->waitForFinished();
    QByteArray rt = proc->readAllStandardOutput();
    proc->close();
    delete proc;

    QString free = "0";
    QStringList lt = QString(rt).split("\n");
    if (lt.length() >= 2) {
        QString line = lt.at(1);
        QString temp;
        QStringList tpl;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] != ' ') {
                temp.append(line[i]);
            } else {
                if (!temp.isEmpty()) {
                    tpl.append(temp);
                    temp = "";
                }
            }
        }
        free = tpl[3];
    }
    return free.toLong();
}

QString Aria2RPCInterface::getBtToMetalink(QString filePath)
{
    QString strMetaLink = ""; //磁力链

    QFile file(filePath); //strFilePath文件的绝对路径
    if (file.open(QIODevice::ReadOnly)) //只读方式打开
    {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        if (!file.atEnd()) {
            hash.addData(file.readAll());
            QString stHashValue;
            stHashValue.append(hash.result().toHex());
            return stHashValue;
        }
    }

    return strMetaLink;
}

QString Aria2RPCInterface::bytesFormat(qint64 size)
{
    if (!size) {
        return "0B";
    }
    QStringList sl;
    if (sl.empty()) {
        sl << "B"
           << "KB"
           << "MB"
           << "GB"
           << "TB"
           << "PB";
    }
    int i = qFloor(qLn(size) / qLn(1024));
    return QString::number(size * 1.0 / qPow(1024, qFloor(i)), 'f', (i > 1) ? 2 : 0) + sl.at(i);
}

bool Aria2RPCInterface::shutdown(QString id)
{
    return callRPC(ARIA2C_METHOD_SHUTDOWN, id);
}

bool Aria2RPCInterface::forceShutdown(QString id)
{
    return callRPC(ARIA2C_METHOD_FORCE_SHUTDOWN, id);
}

QString Aria2RPCInterface::getToken()
{
    static QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return uuid;
}

bool Aria2RPCInterface::setupConfig()
{
    //定义配置文件路径
    QString m_aria2configPath = QString("%1/%2/%3/aria2.conf")
                                    .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                    .arg(qApp->organizationName())
                                    .arg(qApp->applicationName());

    //判断文件是否存在,如果不存在复制配置文件内容到目录下
    QFileInfo fileInfo(m_aria2configPath);
    if (!fileInfo.exists()) {
        QFile::copy(ARIA_CONFIG_PATH, m_aria2configPath);
    }

    //设置配置文件路径
    setConfigFilePath(m_aria2configPath);
    return true;
}
