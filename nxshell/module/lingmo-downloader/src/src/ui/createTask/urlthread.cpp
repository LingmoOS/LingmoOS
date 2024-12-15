// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "urlthread.h"
#include "analysisurl.h"
#include "aria2rpcinterface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QMutex>
#include <QThread>
#include <QMimeDatabase>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include "config.h"

UrlThread::UrlThread(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<LinkInfo>("LinkInfo");
    QString iniConfigPath = QString("%1/%2/%3/content-type.conf")
                                    .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                    .arg(qApp->organizationName())
                                    .arg(qApp->applicationName());

    //判断文件是否存在,如果不存在复制配置文件内容到目录下
    QFileInfo fileInfo(iniConfigPath);
    if (!fileInfo.exists()) {
        QFile::copy(CONTENT_TYPE_CONFIG_PATH, iniConfigPath);
    }
        if(m_iniFile == nullptr){
            m_iniFile = new QSettings(iniConfigPath, QSettings::IniFormat);
            m_iniFile->beginGroup("content-type");
        }
}

void UrlThread::start(LinkInfo &urlInfo)
{
  //  m_linkInfo = new LinkInfo;
    m_linkInfo = urlInfo;
}

void UrlThread::begin()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest requset; // 定义请求对象
    requset.setUrl(QUrl(m_linkInfo.url)); // 设置服务器的uri
    requset.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->head(requset);
    // post信息到服务器
    QObject::connect(manager, &QNetworkAccessManager::finished, this, &UrlThread::onHttpRequest, Qt::UniqueConnection);
    QThread::usleep(100);
}

void UrlThread::onHttpRequest(QNetworkReply *reply)
{
    m_linkInfo.state = LinkInfo::UrlState::Finished;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (statusCode) {
    case 200: // redirect (Location: [URL])   真实链接
    {
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        QProcess process;
#else
        QProcess *process = new QProcess;
#endif
        QStringList list;
        list << "-I" << reply->url().toString();
        if(list.size() > 2){
         //   list[1] = QUrl::toPercentEncoding(list[1]);
            list[1].replace(" ", "%20");
        }
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        process.start("curl", list);
        connect(&process, &QProcess::readyReadStandardOutput, this, [=]() {
#else
        process->start("curl", list);
        connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
#endif
            static QMutex mutex;
            mutex.lock();
            QProcess *proc = dynamic_cast<QProcess *>(sender());
            QString str = proc->readAllStandardOutput();

            QStringList urlList;
            urlList.append(proc->arguments().at(1));
            proc->kill();
            proc->close();
            delete proc;
            proc = nullptr;
            m_linkInfo.urlSize = getUrlSize(str);
            if(m_linkInfo.urlSize.isEmpty()){
                m_linkInfo.urlSize = "0KB";
            }
            m_linkInfo.type = getUrlType(str);
            if(m_linkInfo.type.isEmpty()){
                m_linkInfo.type = "html";
            }
            if (!str.contains("Content-Disposition: attachment;filename=")) // 为200的真实链接
            {
                emit sendFinishedUrl(m_linkInfo);
                mutex.unlock();
                return;
            }
            QStringList urlInfoList = str.split("\r\n");
            for (int i = 0; i < urlInfoList.size(); i++) {
                if (urlInfoList[i].startsWith("Content-Disposition:")) //为405链接
                {
                   // int start = urlInfoList[i].indexOf("filename=");
                    QString urlName = urlInfoList[i].split("filename=").last();
                    urlName = urlName.remove('"');
                    QString type = urlName.split('.').last();
                    urlName.remove(type);
                    urlName = urlName.left(urlName.size() -1);
                    QString encodingUrlName = QUrl::fromPercentEncoding(urlName.toUtf8());
                    m_linkInfo.urlName = encodingUrlName;
                    emit sendFinishedUrl(m_linkInfo);
                }
            }
            mutex.unlock();
        });
        break;
    }
    case 301:
    case 302: // redirect (Location: [URL])  重定向链接
    {
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        QProcess process;
#else
        QProcess *process = new QProcess;
#endif
        QStringList list;
        list << "-i" << reply->url().toString();
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        process.start("curl", list);
        connect(&process, &QProcess::readyReadStandardOutput, this, [=]() {
#else
        process->start("curl", list);
        connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
#endif
            static QMutex mutex;
            mutex.lock();
            QProcess *proc = dynamic_cast<QProcess *>(sender());
            QString str = proc->readAllStandardOutput();

            QString strUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            QString url = reply->url().toString();
            QStringList l = url.split("/");
            if(!strUrl.startsWith("http")){
                strUrl = l[0] + "//" + l[2] + strUrl;
            }
            m_linkInfo.urlTrueLink = strUrl;
            QStringList strList = strUrl.split("/");
            QStringList strUrlName = strList[strList.size() - 1].split(".");
            //需要转两次
            QString encodingUrlName;
            QMimeDatabase db;
            QString type = db.suffixForFileName(strList[strList.size() - 1]);
            if (!type.isEmpty()) {
                encodingUrlName = strList[strList.size() - 1].mid(0, strList[strList.size() - 1].size() - type.size() - 1);
            } else {
                encodingUrlName = strUrlName[0]; //QUrl::fromPercentEncoding(strUrlName[0].toUtf8());
            }
            m_linkInfo.urlName = encodingUrlName;
            QStringList urlStrList = QStringList(strUrl);
            m_linkInfo.type = getUrlType(strUrl);
           // m_linkInfo->urlSize = getUrlSize(str);
            emit sendTrueUrl(m_linkInfo);
            m_linkInfo.url = m_linkInfo.urlTrueLink;
           // emit sendFinishedUrl(*m_linkInfo);
            //onDownloadNewUrl(strUrl, Settings::getInstance()->getCustomFilePath(), encodingUrlName, type);
            proc->kill();
            proc->close();
            delete proc;
            proc = nullptr;
            mutex.unlock();
            begin();
        });
        break;
    }
//    case 400: {
//        if(m_linkInfo.url.startsWith("https://www.gmssl.cn/")){
//            m_linkInfo.urlSize = "1kb";
//            m_linkInfo.length = 1024;
//            m_linkInfo.type = getUrlType(m_linkInfo.url);
//            if(m_linkInfo.type.isEmpty()){
//                m_linkInfo.type = "html";
//            }
//            emit sendFinishedUrl(m_linkInfo);
//        }
//        break;
//    }
    case 404: {
        emit sendFinishedUrl(m_linkInfo);
        break;
    }
    case 405: //405链接
    {
        QProcess process;
        QStringList list;
        list << "-i" << reply->url().toString();
        process.start("curl", list);
        connect(&process, &QProcess::readyReadStandardOutput, this, [=]() {
            static QMutex mutex;
            mutex.lock();
            QProcess *proc = dynamic_cast<QProcess *>(sender());
            proc->kill();
            proc->close();
            QString str = proc->readAllStandardOutput();
            delete proc;
            proc = nullptr;
            QStringList urlInfoList = str.split("\r\n");
            for (int i = 0; i < urlInfoList.size(); i++) {
                if (urlInfoList[i].startsWith("Content-Disposition:")) //为405链接
                {
                    int start = urlInfoList[i].lastIndexOf("'");
                    QString urlName = urlInfoList[i].mid(start);
                    QString urlNameForZH = QUrl::fromPercentEncoding(urlName.toUtf8());
                    // emit NewDownload_sig(QStringList(redirecUrl),m_defaultDownloadDir,_urlNameForZH);
                    QStringList strList = QStringList(urlInfoList[i]);
                    // onDownloadNewUrl(urlInfoList[i], Settings::getInstance()->getCustomFilePath(), urlNameForZH);
                    mutex.unlock();
                    return;
                }
            }
            mutex.unlock();
        });

        break;
    }
    default:
    {
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        QProcess process;
#else
        QProcess *process = new QProcess;
#endif
        QStringList list;
        list << "-I" << "-k" << reply->url().toString();
        QString str = reply->url().toString();
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        process.start("curl", list);
        connect(&process, &QProcess::readyReadStandardOutput, this, [=]() {
#else
        process->start("curl", list);
        connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
#endif
            qDebug()<<"readyReadStandardOutput";
            static QMutex mutex;
            mutex.lock();
            QProcess *proc = dynamic_cast<QProcess *>(sender());
            QString str =proc->readAllStandardOutput();
            QStringList list = str.split("\r\n");
            if(!list[0].contains("200")){
                mutex.unlock();
                return ;
            }
            proc->kill();
            proc->close();
            delete proc;
            //process = nullptr;

            m_linkInfo.urlSize = getUrlSize(str);
            m_linkInfo.type =getUrlType(str);
            emit sendFinishedUrl(m_linkInfo);
            mutex.unlock();

        });
#if defined(CMAKE_SAFETYTEST_ARG_ON)
        connect(&process, &QProcess::readyReadStandardOutput, this, [=]() {
#else
        connect(process, &QProcess::readyReadStandardError, this, [=]() {
#endif
            QProcess *proc = dynamic_cast<QProcess *>(sender());
            QString str = proc->readAllStandardError();

            qDebug()<<"readyReadStandardError: " << str;
            if(proc->exitCode() == 0 && (!str.contains("curl"))) {
                return ;
            }
            proc->kill();
            proc->deleteLater();
            emit sendFinishedUrl(m_linkInfo);
        });
    }
    }
}

QString UrlThread::getUrlType(QString url)
{
    QMimeDatabase db;
    QString type;
    QStringList urlInfoList = url.split("\r\n");
    for (int i = 0; i < urlInfoList.size(); i++) {
        if (urlInfoList[i].startsWith("content-type:", Qt::CaseInsensitive)) {
            QString contentType = urlInfoList[i].split(" ")[1];
            type = getType(contentType);
            break;
        }
    }
    if(type.isEmpty()){
        type = db.suffixForFileName(m_linkInfo.url);
    }
    if(type.isEmpty()){
        type = getNoContentType();
    }
    return type;
}

QString UrlThread::getUrlSize(QString url)
{
    QStringList urlInfoList = url.split("\r\n");
    for (int i = 0; i < urlInfoList.size(); i++) {
        if (urlInfoList[i].startsWith("Content-Length:", Qt::CaseInsensitive)) {
            QString str = urlInfoList[i].split(" ")[1];
            long size = urlInfoList[i].split(" ")[1].toLatin1().toLong();
            m_linkInfo.length = size;
            return Aria2RPCInterface::instance()->bytesFormat(size);
        }
    }
    return "";
}

QString UrlThread::getType(QString contentType)
{
    contentType.remove(";");
    QString str3 = m_iniFile->value(contentType).toString();
    return  m_iniFile->value(contentType).toString();
}

QString UrlThread::getNoContentType()
{
    QString jsonType = ".asf;.avi;.exe;.iso;.mp3;.mpeg;.mpg;.mpga;"
                   ".ra;.rar;.rm;.rmvb;.tar;.wma;.wmp;.wmv;.mov;"
                   ".zip;.3gp;.chm;.mdf;.torrent;.jar;.msi;.arj;."
                   "bin;.dll;.psd;.hqx;.sit;.lzh;.gz;.tgz;.xlsx;"
                   ".xls;.doc;.docx;.ppt;.pptx;.flv;.swf;.mkv;.tp;"
                   ".ts;.flac;.ape;.wav;.aac;.txt;.dat;.7z;.ttf;.bat;"
                   ".xv;.xvx;.pdf;.mp4;.apk;.ipa;.epub;.mobi;.deb;.sisx;.cab;.pxl;";
    QString url = m_linkInfo.url.split('?')[0].toLower();
    QStringList surlList = url.split('.');
    QString type = surlList[surlList.size()-1];
    if(jsonType.contains(type)){
        return type;
    }
    return "";
}
