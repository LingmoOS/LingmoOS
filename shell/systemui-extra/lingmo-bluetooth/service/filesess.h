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

#ifndef FILESESS_H
#define FILESESS_H

#include <QObject>
#ifdef BATTERY
#include <KF5/BluezQt/bluezqt/battery.h>
#endif
#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obexobjectpush.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>

#include <QStandardPaths>
#include <QDBusObjectPath>
#include <QThread>
#include <QEventLoop>
#include <QProcess>

#include "common.h"
#include "lingmo-log4qt.h"
#include "CSingleton.h"

class LingmoFileSess : public QObject
{
    Q_OBJECT    
public:
    explicit LingmoFileSess(QString, QStringList);
    explicit LingmoFileSess(BluezQt::ObexTransferPtr transfer,BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);
    ~LingmoFileSess();

    void ObexSessionPtr(BluezQt::ObexSessionPtr p){ m_sess = p; }
    BluezQt::ObexSessionPtr ObexSessionPtr(void) { return m_sess; }

    enum_filetransport_Type type(void){ return m_type; }

    void receiveUpdate(BluezQt::ObexTransferPtr transfer,BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);

    QDBusObjectPath object_path(void){ return m_object_path; }

    void stop(void);

    int replyFileReceiving(bool, QString);

    void addNewFiles(QStringList files);

    void cancelReceive();
public slots:
    void CreateSessStart(BluezQt::PendingCall *call);

protected slots:
    void TransferStart(BluezQt::PendingCall *call);

    void transferredChanged(quint64 transferred);
    void statusChanged(BluezQt::ObexTransfer::Status status);
    void fileNameChanged(const QString &fileName);

protected:
    int sendfile(void);

    QString getProperFilePath(QString, QString);

    int movefile(QString, QString &);

    virtual void timerEvent( QTimerEvent *event);

    void setSendfileStatus(bool status = true);
private:
    int send_statuschanged(void);

    int send_reveivesignal(void);

    int __init_Transfer();

    int __send_statuschanged(void);

    bool __isdir_exist(QString);

    bool __isfile_exit(QString);
protected:
    bool m_running = true;

    QString m_dest;
    QStringList m_files;
    int m_fileIndex = 0;

    QDBusObjectPath m_object_path;

    quint64 m_transfer_file_size = 0;
    QString m_transferPath;
    int m_percent = 0;

    BluezQt::ObexTransferPtr m_filePtr = nullptr;

    BluezQt::ObexSessionPtr m_sess = nullptr;
    BluezQt::ObexObjectPush * m_opp = nullptr;
    BluezQt::Request<QString> m_request;
    QMap<QString, QVariant> m_attr;

    QString m_savePathdir;

    QString m_user;

    QString m_filetype;

    int m_remove_Timer = 0;

    enum_filetransport_Type m_type = enum_filetransport_Type_send;
};


typedef QSharedPointer<LingmoFileSess> LingmoFileSessPtr;

////////////////////////////////////////

class FileSessMng : public QObject
{
    Q_OBJECT
public:
    explicit FileSessMng();

    int init(void);

    int sendFiles(QString, QStringList);

    int receiveFiles(BluezQt::ObexTransferPtr transfer,BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);

    int removeSession(const QDBusObjectPath &session);

    int stopFiles(QString, int);

    int replyFileReceiving(QString, bool, QString);

    void sessionCanceled(BluezQt::ObexSessionPtr session);
protected slots:
    //obex
    void operationalChanged(bool operational);
    void sessionAdded(BluezQt::ObexSessionPtr session);
    void sessionRemoved(BluezQt::ObexSessionPtr session);

private:
    void wait_for_finish(BluezQt::PendingCall *call);

    void start_obexservice();

    bool isFileExists(const QString& filePath);
protected:
    QMap<QString, LingmoFileSessPtr> m_sess;

    QMap<QString, LingmoFileSessPtr> m_sess_recv;

    BluezQt::ObexManager  * m_obex = nullptr;

    QProcess * m_process = nullptr;

    friend class SingleTon<FileSessMng>;
};

typedef SingleTon<FileSessMng>  FILESESSMNG;

#endif // FILESESS_H
