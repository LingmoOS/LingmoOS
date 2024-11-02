/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "filesess.h"


#include <QFileInfo>
#include <QDir>
#include <QTimerEvent>

#include "adapter.h"
#include "bluetoothobexagent.h"
#include "sessiondbusregister.h"
#include "device.h"
#include "config.h"
#include "app.h"

#include <file-utils.h>

LingmoFileSess::LingmoFileSess(QString addr, QStringList files)
{
    KyInfo() << addr << files;
    m_dest = addr;
    m_files = files;
    m_type = enum_filetransport_Type_send;
    m_user = CONFIG::getInstance()->active_user();

    //设置文件传输状态
    this->setSendfileStatus();
}

LingmoFileSess::LingmoFileSess(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    m_type = enum_filetransport_Type_receive;
    m_filePtr = transfer;
    m_sess = session;
    m_request = request;
    m_user = CONFIG::getInstance()->active_user();

    m_dest = session->destination();
    m_object_path = session->objectPath();
    KyInfo() << m_object_path.path() << session->destination();
    this->__init_Transfer();

    this->send_reveivesignal();

    //设置文件传输状态
    this->setSendfileStatus();
}

LingmoFileSess::~LingmoFileSess()
{
    KyInfo() << m_dest;
    if(m_opp)
    {
        delete m_opp;
        m_opp = nullptr;
    }

    if(0 != m_remove_Timer)
    {
        this->killTimer(m_remove_Timer);
        m_remove_Timer = 0;
    }

    //接收异常
    if(enum_filetransport_Type_receive == m_type)
    {

    }

    m_attr[FileStatusAttr(enum_filestatus_dev)] = m_dest;
    m_attr[FileStatusAttr(enum_filestatus_status)] = 0xff;
    m_attr[FileStatusAttr(enum_filestatus_transportType)] = m_type;
    this->__send_statuschanged();

    //设置文件传输状态
    this->setSendfileStatus(false);
}

void LingmoFileSess::receiveUpdate(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    m_filetype = "";
    m_filePtr = transfer;
    m_request = request;
    this->__init_Transfer();

    if(session->objectPath() != m_sess->objectPath())
    {
        KyInfo() << session->destination() << " sess changed, " << session->objectPath().path();
        m_sess = session;
        m_object_path = m_sess->objectPath();
        this->send_reveivesignal();
    }
    else
    {
        m_fileIndex++;
        m_request.accept(m_filePtr->name());
    }

    if(0 != m_remove_Timer)
    {
        this->killTimer(m_remove_Timer);
        m_remove_Timer = 0;
    }
}

void LingmoFileSess::stop()
{
    KyInfo();
    if(nullptr != m_filePtr && BluezQt::ObexTransfer::Status::Active == m_filePtr->status())
    {
        m_filePtr->cancel();
    }
    m_running = false;
}

int LingmoFileSess::replyFileReceiving(bool v, QString path)
{
    KyInfo() << v;
    if(v)
    {
        if(!path.isEmpty() && this->__isdir_exist(path))
        {
            m_savePathdir = path;
            if(!m_savePathdir.isEmpty() && m_savePathdir[m_savePathdir.length() - 1] != '/')
            {
                m_savePathdir += '/';
            }
        }
        m_request.accept(m_filePtr->name());
    }
    else
    {
        m_request.reject();
    }

    return 0;
}

void LingmoFileSess::CreateSessStart(BluezQt::PendingCall *call)
{
    QVariant v = call->value();
    m_object_path = v.value<QDBusObjectPath>();
    KyInfo() << m_object_path.path();
    if(call->error() != 0)
    {
        KyWarning()<< m_dest <<" CreateSessStart error, code: "
                  <<call->error() << ", msg: "<< call->errorText();

        m_attr[FileStatusAttr(enum_filestatus_dev)] = m_dest;
        m_attr[FileStatusAttr(enum_filestatus_fileFailedDisc)] = call->errorText();

        FILESESSMNG::getInstance()->removeSession(m_object_path);
        return;
    }

    if(!m_running)
    {
        KyInfo() << "sendfile stop";
        FILESESSMNG::getInstance()->removeSession(m_object_path);
        return;
    }

    this->__send_statuschanged();
    m_opp = new BluezQt::ObexObjectPush(m_object_path);
    this->sendfile();
}

void LingmoFileSess::TransferStart(BluezQt::PendingCall *call)
{
    KyInfo();
    if(call->error() != 0)
    {
        KyWarning()<< m_dest <<" TransferStart error, code: "
                  <<call->error() << ", msg: "<< call->errorText();

        m_attr[FileStatusAttr(enum_filestatus_dev)] = m_dest;
        m_attr[FileStatusAttr(enum_filestatus_fileFailedDisc)] = call->errorText();
        this->__send_statuschanged();

        FILESESSMNG::getInstance()->removeSession(m_object_path);
        return;
    }

    QVariant v = call->value();
    m_filePtr = v.value<BluezQt::ObexTransferPtr>();
    this->__init_Transfer();
}

void LingmoFileSess::transferredChanged(quint64 transferred)
{
    int percent =  transferred * 100.0 / m_transfer_file_size;
    if(percent != m_percent)
    {
        m_attr[FileStatusAttr(enum_filestatus_progress)] = percent;
        this->send_statuschanged();
    }
    m_percent = percent;
}

void LingmoFileSess::statusChanged(BluezQt::ObexTransfer::Status status)
{
    KyDebug() << status;

    if(BluezQt::ObexTransfer::Status::Active == status)
    {
        if(enum_filetransport_Type_receive == m_type)
        {
            if(m_filetype.isEmpty())
            {
                QString src = "/root/.cache/obexd/" + m_filePtr->name();
                GError *error;
                GFile *file = g_file_new_for_path(src.toStdString().c_str());
                if(nullptr != file)
                {
                    GFileInfo *file_info = g_file_query_info(file,"*",G_FILE_QUERY_INFO_NONE,NULL,&error);
                    if(file_info)
                    {
                        m_filetype = g_file_info_get_content_type(file_info);
                        KyInfo() << m_filePtr->name() << ", filetype: "<<m_filetype;
                    }
                }

            }
        }
    }
    else if(BluezQt::ObexTransfer::Status::Queued == status)
    {
    }
    else if(BluezQt::ObexTransfer::Status::Suspended == status)
    {

    }
    else if(BluezQt::ObexTransfer::Status::Complete == status)
    {
        if(enum_filetransport_Type_send == m_type)
        {
            m_fileIndex++;
            if(m_files.size() > m_fileIndex)
            {
                this->sendfile();
            }
            else
            {
                FILESESSMNG::getInstance()->removeSession(m_object_path);
            }
        }
        //接收文件，改变文件所属
        else
        {
            QString dest;
            this->movefile(m_filePtr->name(), dest);
            m_attr[FileStatusAttr(enum_filestatus_savepath)] = dest;

            //10s内未收到文件传输信号，则断开文件连接连接
            if(0 == m_remove_Timer)
            {
                m_remove_Timer = this->startTimer(10 * 1000);
            }
        }
    }
    else if(BluezQt::ObexTransfer::Status::Error == status)
    {
        this->send_statuschanged();
        FILESESSMNG::getInstance()->removeSession(m_object_path);
        return;
    }
    else if(BluezQt::ObexTransfer::Status::Unknown == status)
    {
        this->send_statuschanged();
        FILESESSMNG::getInstance()->removeSession(m_object_path);
        return;
    }

    this->send_statuschanged();
}

void LingmoFileSess::cancelReceive()
{
    m_attr[FileStatusAttr(enum_filestatus_dev)] = m_dest;
    m_attr[FileStatusAttr(enum_filestatus_status)] = BluezQt::ObexTransfer::Error;
    m_attr[FileStatusAttr(enum_filestatus_transportType)] = m_type;
    this->__send_statuschanged();
}

void LingmoFileSess::fileNameChanged(const QString &fileName)
{
    KyDebug() << fileName;
}

void LingmoFileSess::addNewFiles(QStringList files)
{
    m_files.append(files);
}

int LingmoFileSess::sendfile()
{
    int index = m_fileIndex;
    QString filename = m_files[index];
    BluezQt::PendingCall *transfer = m_opp->sendFile(filename);
    connect(transfer, &BluezQt::PendingCall::finished, this, &LingmoFileSess::TransferStart);
    return 0;
}

QString LingmoFileSess::getProperFilePath(QString dir, QString filename)
{
    QString dest;
    QString dbusid = APPMNG::getInstance()->getUserDbusid(m_user, enum_app_type_bluetooth_tray);
    if(!dbusid.isEmpty())
    {
        QDBusInterface iface(dbusid, "/", "com.lingmo.bluetooth", QDBusConnection::systemBus());
        QDBusPendingCall pcall = iface.asyncCall("getProperFilePath", m_user, filename);
        pcall.waitForFinished();
        QDBusMessage res = pcall.reply();
        if(res.type() == QDBusMessage::ReplyMessage)
        {
            KyInfo() << res;
            if(res.arguments().size() > 0)
            {
                dest = res.arguments().takeFirst().toString();
            }
        }
        else
        {
            KyWarning()<< res.errorName() << ": "<< res.errorMessage();
        }
    }

    if(dest.isEmpty())
    {
        dest = "/home/" + m_user + "/" + filename;
        KyDebug() << dest;
        while (QFile::exists(dest))
        {
            QStringList newUrl = dest.split("/");
            newUrl.pop_back();
            newUrl.append(Peony::FileUtils::handleDuplicateName(Peony::FileUtils::urlDecode(dest)));
            dest = newUrl.join("/");
            KyDebug() << dest << newUrl;
        }
    }

    return dest;
}

int LingmoFileSess::movefile(QString filename, QString & dest)
{
    QString src = "/root/.cache/obexd/" + filename;
    dest = this->getProperFilePath(m_savePathdir, filename);

    GError *error = NULL;
    GFile *source = g_file_new_for_path(src.toStdString().c_str());
    GFile *destination = g_file_new_for_path(dest.toStdString().c_str());
    bool flag = g_file_move(source,destination,G_FILE_COPY_BACKUP,NULL,NULL,NULL,&error);
    KyInfo() << "move file" << "target_path =" << dest << " source_path =" << src << "flag =" << flag;
    if(flag)
    {
        QString cmd;
        cmd = "chgrp " + m_user + " \'" + dest + "\'";
        KyInfo() << cmd;
        system(cmd.toStdString().c_str());
        cmd = "chown " + m_user + " \'" + dest + "\'";;
        KyInfo() << cmd;
        system(cmd.toStdString().c_str());
    }

    return 0;
}

void LingmoFileSess::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_remove_Timer)
    {
        KyInfo() << "remove filesess";
        this->killTimer(event->timerId());
        m_remove_Timer = 0;
        FILESESSMNG::getInstance()->removeSession(m_object_path);
    }
    else
    {
        KyInfo() << "other timer, delete";
        this->killTimer(event->timerId());
    }
}

void LingmoFileSess::setSendfileStatus(bool status /*= true*/)
{
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(ptr)
    {
        d = ptr->getDevPtr(m_dest);
    }

    if(d)
    {
        d->setSendfileStatus(status);
    }
}

int LingmoFileSess::send_statuschanged()
{
    if(enum_filetransport_Type_send == m_type)
    {
        m_attr[FileStatusAttr(enum_filestatus_allfilenum)] = m_files.size();
    }
    else
    {
        if(m_filetype.isEmpty())
        {
            m_attr[FileStatusAttr(enum_filestatus_filetype)] = m_filePtr->type();
        }
        else
        {
            m_attr[FileStatusAttr(enum_filestatus_filetype)] = m_filetype;
        }
        m_attr[FileStatusAttr(enum_filestatus_filesize)] = m_filePtr->size();
    }
    m_attr[FileStatusAttr(enum_filestatus_filename)] = m_filePtr->name();
    m_attr[FileStatusAttr(enum_filestatus_fileseq)] = m_fileIndex;
    m_attr[FileStatusAttr(enum_filestatus_dev)] = m_dest;
    m_attr[FileStatusAttr(enum_filestatus_status)] = m_filePtr->status();
    m_attr[FileStatusAttr(enum_filestatus_transportType)] = m_type;
    this->__send_statuschanged();
    return 0;
}

int LingmoFileSess::__send_statuschanged()
{
    Q_EMIT SYSDBUSMNG::getInstance()->fileStatusChanged(m_attr);
    m_attr.clear();
    return 0;
}

bool LingmoFileSess::__isdir_exist(QString dirpath)
{
    QDir dir(dirpath);
    return dir.exists();
}

bool LingmoFileSess::__isfile_exit(QString filefullpath)
{
    QFile file(filefullpath);
    return file.exists();
}

int LingmoFileSess::send_reveivesignal()
{
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(!ptr)
    {
        KyWarning() << "getDefaultAdapter nullptr";
        return -1;
    }
    d = ptr->getDevPtr(m_dest);
    if(!d)
    {
        KyWarning() << "getDevPtr nullptr";
        return -1;
    }

    QMap<QString, QVariant> attrs;
    attrs[ReceiveFileAttr(enum_receivefile_dev)] = m_dest;
    attrs[ReceiveFileAttr(enum_receivefile_name)] = d->Name();
    attrs[ReceiveFileAttr(enum_receivefile_filetype)] = m_filePtr->type();
    attrs[ReceiveFileAttr(enum_receivefile_filename)] = m_filePtr->name();
    attrs[ReceiveFileAttr(enum_receivefile_filesize)] = m_filePtr->size();
    Q_EMIT SYSDBUSMNG::getInstance()->fileReceiveSignal(attrs);

    //m_request.accept(m_filePtr->name());
    return 0;
}

int LingmoFileSess::__init_Transfer()
{
    m_transfer_file_size = m_filePtr->size();
    m_transferPath = m_filePtr->objectPath().path();
    KyInfo() << m_transferPath << m_transfer_file_size << m_filePtr->name() << m_filePtr->fileName()
             << m_filePtr->type() << m_filePtr->status();
    connect(m_filePtr.data(), &BluezQt::ObexTransfer::transferredChanged, this, &LingmoFileSess::transferredChanged);
    connect(m_filePtr.data(), &BluezQt::ObexTransfer::statusChanged, this, &LingmoFileSess::statusChanged);
    connect(m_filePtr.data(), &BluezQt::ObexTransfer::fileNameChanged, this, &LingmoFileSess::fileNameChanged);

    m_percent = 0;
    m_attr[FileStatusAttr(enum_filestatus_progress)] = m_percent;
    this->send_statuschanged();
    return 0;
}


////////////////////////////////////////////////////

FileSessMng::FileSessMng()
{
    KyInfo();
}

int FileSessMng::init()
{
    m_obex = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *obex_job = m_obex->init();
    obex_job->exec();

    connect(m_obex, &BluezQt::ObexManager::operationalChanged, this, &FileSessMng::operationalChanged);
    connect(m_obex, &BluezQt::ObexManager::sessionAdded, this, &FileSessMng::sessionAdded);
    connect(m_obex, &BluezQt::ObexManager::sessionRemoved, this, &FileSessMng::sessionRemoved);

    if(!m_obex->isOperational())
    {
        this->start_obexservice();
    }
    else
    {
        Q_EMIT m_obex->operationalChanged(m_obex->isOperational());
    }

    return 0;
}

int FileSessMng::sendFiles(QString addr, QStringList files)
{
    KyInfo()  << addr << files;
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(!ptr || !m_obex->isOperational() || addr.isEmpty()  || files.size() == 0)
    {
        return -1;
    }
    if (m_sess.contains(addr))
    {
        m_sess[addr]->addNewFiles(files);
        return 0;
    }

    LingmoFileSessPtr s(new LingmoFileSess(addr, files));
    m_sess[addr] = s;
    QMap<QString,QVariant> map;
    map["Source"] = ptr->addr();
    map["Target"] = "OPP";
    BluezQt::PendingCall *target = m_obex->createSession(addr,map);
    connect(target, &BluezQt::PendingCall::finished, s.data(), &LingmoFileSess::CreateSessStart);
    this->wait_for_finish(target);

    if(0 != target->error())
    {
        m_sess.remove(addr);
    }

    return target->error();
}

int FileSessMng::receiveFiles(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    if(m_sess_recv.contains(session->destination()))
    {
        m_sess_recv[session->destination()]->receiveUpdate(transfer, session, request);
        return 0;
    }

    LingmoFileSessPtr s(new LingmoFileSess(transfer, session, request));
    m_sess_recv[session->destination()] = s;
    return 0;
}

int FileSessMng::removeSession(const QDBusObjectPath &session)
{
    KyInfo() << session.path();
    m_obex->removeSession(session);
    return 0;
}

int FileSessMng::stopFiles(QString addr, int type)
{
    KyInfo() << addr << type;
    if(enum_filetransport_Type_send == type)
    {
        if(m_sess.contains(addr))
        {
            m_sess[addr]->stop();
            if(!m_sess[addr]->object_path().path().isEmpty())
            {
                this->removeSession(m_sess[addr]->object_path());
            }
        }
    }
    else if(enum_filetransport_Type_receive == type)
    {
        if(m_sess_recv.contains(addr))
        {
            m_sess_recv[addr]->stop();
            this->removeSession(m_sess_recv[addr]->object_path());
        }
    }

    return 0;
}

int FileSessMng::replyFileReceiving(QString addr, bool v, QString path)
{
    KyInfo() << addr << v;
    if(m_sess_recv.contains(addr))
    {
        return m_sess_recv[addr]->replyFileReceiving(v, path);
    }
    return 0;
}

void FileSessMng::operationalChanged(bool operational)
{
    if(operational)
    {
        if(!BLUETOOTHOBEXAGENT::getInstance())
        {
            BLUETOOTHOBEXAGENT::instance(this);
            BluezQt::PendingCall * call = m_obex->registerAgent(BLUETOOTHOBEXAGENT::getInstance());
            this->wait_for_finish(call);
        }
    }
    else
    {
        if(BLUETOOTHOBEXAGENT::getInstance())
        {
            BluezQt::PendingCall * call = m_obex->unregisterAgent(BLUETOOTHOBEXAGENT::getInstance());
            this->wait_for_finish(call);
            BLUETOOTHOBEXAGENT::destroyInstance();
        }
        QTimer::singleShot(5 * 1000,this,[=]{
            KyInfo() << "restart obex";
            this->start_obexservice();
        });
    }
}

void FileSessMng::sessionAdded(BluezQt::ObexSessionPtr session)
{
    KyInfo();
    if(session && m_sess.contains(session->destination()))
    {
        m_sess[session->destination()]->ObexSessionPtr(session);
    }
}

void FileSessMng::sessionRemoved(BluezQt::ObexSessionPtr session)
{
    KyInfo() << ((nullptr == session) ? "null session" : session->destination())
             << ((nullptr == session) ? "null session" : session->objectPath().path());
    if(session && m_sess.contains(session->destination()))
    {
        BluezQt::ObexSessionPtr ptr = m_sess[session->destination()]->ObexSessionPtr();
        if(ptr && ptr->objectPath() == session->objectPath())
        {
            m_sess.remove(session->destination());
        }
    }

    if(session && m_sess_recv.contains(session->destination()))
    {
        BluezQt::ObexSessionPtr ptr = m_sess_recv[session->destination()]->ObexSessionPtr();
        if(ptr && ptr->objectPath() == session->objectPath())
        {
           m_sess_recv.remove(session->destination());
        }
    }
}

void FileSessMng::sessionCanceled(BluezQt::ObexSessionPtr session)
{
    if (!m_sess_recv.contains(session->destination()))
        return;
    m_sess_recv[session->destination()]->cancelReceive();
}


void FileSessMng::wait_for_finish(BluezQt::PendingCall *call)
{
    QEventLoop eventloop;
    connect(call, &BluezQt::PendingCall::finished, this, [&](BluezQt::PendingCall *callReturn)
    {
        eventloop.exit();
    });
    eventloop.exec();
}

void FileSessMng::start_obexservice()
{
    KyInfo() << "start obex";
    if(nullptr == m_process)
    {
        m_process = new QProcess;
        QString cmd = "/usr/lib/bluetooth/obexd";
        if (!this->isFileExists(cmd)) {
            cmd = "/usr/libexec/bluetooth/obexd";
        }
        KyInfo()<< cmd;
        m_process->setProgram(cmd);
        m_process->setArguments(QStringList{"-d"});
        m_process->startDetached();
    }
    else
    {
        QString cmd = "/usr/lib/bluetooth/obexd";
        if (!this->isFileExists(cmd)) {
            cmd = "/usr/libexec/bluetooth/obexd";
        }
        KyInfo()<< cmd;
        m_process->setProgram(cmd);
        m_process->setArguments(QStringList{"-d"});
        m_process->startDetached();
    }
}

bool FileSessMng::isFileExists(const QString &filePath)
{
    QFile file(filePath);
    return file.exists();
}




