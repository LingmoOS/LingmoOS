// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERJOB_H
#define TRANSFERJOB_H

#include <QObject>
#include <QQueue>
#include <QSharedPointer>
#include <service/rpc/remoteservice.h>
#include <ipc/proto/chan.h>
#include "common/constant.h"
#include "co/co.h"
#include "co/fs.h"
#include "co/time.h"
#include <QMutex>
#include <QReadWriteLock>

class RemoteServiceSender;
class TransferJob : public QObject
{
    Q_OBJECT

public:   
    explicit TransferJob(QObject *parent = nullptr);
    ~TransferJob() override;
    bool initRpc(fastring target, uint16 port);
    void initJob(fastring appname, fastring targetappname, int id, fastring path, bool sub, fastring savedir, bool write);
    bool createFile(const fastring fullpath, const bool isDir);

    void start();
    void stop(const bool notify = true);
    void waitFinish();
    bool ended();
    bool isRunning();
    bool isWriteJob();
    fastring getAppName();

    void cancel(bool notify = false);

    void pushQueque(const QSharedPointer<FSDataBlock> block);
    bool initSuccess() const { return _init_success; }
    void setDeviceNotenough();
    qint64 freeBytes() const;
    bool offlineCancel(const QString &ip);

signals:
    // 传输作业结果通知：文件（目录），结果，保存路径
    void notifyJobResult(QString appname, int jobid, int status, QString savedir);

    // 传输文件状态
    void notifyFileTransStatus(QString appname, int status, QString fileinfo);

    // 远程的服务器断开
    void notifyRemoteSeverClose(QString ip, uint16 port);

    // 文件传输完成 移除job
    void notifyJobFinished(const int jodid);

public slots:

private:
    fastring getSubdir(const char *path, const char *root);

    void handleBlockQueque();

    void handleUpdate(FileTransRe result, const char *path, const char *emsg);
    void handleJobStatus(int status);
    void handleTransStatus(int status, const FileInfo &info);
    QSharedPointer<FSDataBlock> popQueue();
    int queueCount() const;
    void setFileName(const fastring &name, const fastring &acName);
    fastring acName(const fastring &name);
    fastring getSaveFullpath(const fastring &rootdir, const fastring &filename);

    void scanPath(const fastring root,const fastring path, const bool acTotal);
    void readPath(fastring path, fastring root, const bool acTotal);
    bool readFile(fastring filepath, int fileid, fastring subdir, const bool acTotal);
    void readFileBlock(fastring filepath, int fileid, const fastring subname, const  bool acTotal);
    bool writeAndCreateFile(const QSharedPointer<FSDataBlock> block, const fastring fullpath);
    bool sendToRemote(const QSharedPointer<FSDataBlock> block);
    void createSendCounting();

private:
    int _jobid;
    std::atomic_int _fileid = 0;
    std::atomic_int _notify_fileid = 0;
    int _status = NONE;
    int _queue_empty_times = 0;
    std::atomic_int64_t _total_size = 0;
    std::atomic_int64_t _cur_size = 0;

    bool _sub;
    bool _writejob {false};
    bool _init_success { true };
    bool _mark_canceled { false };
    std::atomic_bool _device_not_enough{ false };
    std::atomic_bool _offlined {false};
    std::atomic_bool _not_notify {false};

    uint16 _tar_port{0};
    fastring _app_name; // //前端应用名
    fastring _path; // 目录或文件路径
    fastring _savedir; // 写作业，文件保存的目录
    fastring _save_fulldir; // 全路径
    fastring _tar_app_name; // 发送到目标的应用名称
    fastring _tar_ip;
    std::atomic_int64_t _device_free_size{ -1 };

    mutable QReadWriteLock _queque_mutex;
    QQueue<QSharedPointer<FSDataBlock>> _block_queue;
    QSharedPointer<RemoteServiceSender> _remote;
    QReadWriteLock _file_name_maps_lock;
    QMap<fastring, fastring> _file_name_maps;
    QMutex _send_mutex;
    fs::file *fx{ nullptr };
};

#endif   // TRANSFERJOB_H
