// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERWORKER_H
#define TRANSFERWORKER_H

#include "session/asioservice.h"
#include "httpweb/fileserver.h"
#include "httpweb/fileclient.h"

#include <QObject>
#include <QTimer>

class TransferWorker : public QObject, public ProgressCallInterface
{
    Q_OBJECT
    struct file_stats_s {
        std::string path;
        uint64_t total {0};   // 总量
        std::atomic<uint64_t> secsize {0};   // 每秒传输量
    };

public:
    explicit TransferWorker(QString id, QObject *parent = nullptr);

    bool onProgress(uint64_t size) override;

    void onWebChanged(int state, std::string msg = "", uint64_t size = 0) override;

    void stop();

    bool tryStartSend(QStringList paths, int port, std::vector<std::string> *nameVector, std::string *token);
    bool tryStartReceive(QStringList names, QString &ip, int port, QString &token, QString &dirname);

    bool isSyncing();
    void setEveryFileNotify(bool every);
    bool isServe();

signals:
    void notifyChanged(int status, const QString &path = "", quint64 size = 0);

    void speedTimerTick(bool stop = false);

    // IO exception
    void onException(const QString id, const QString reason);

public slots:
    void handleTimerTick(bool stop);
    void doCalculateSpeed();

private:
    bool startWeb(int port);
    bool startGet(const std::string &address, int port);

    void sendTranEndNotify();

    std::shared_ptr<AsioService> _asioService;

    // file sync service and client
    std::shared_ptr<FileServer> _file_server { nullptr };
    std::shared_ptr<FileClient> _file_client { nullptr };

    QTimer _speedTimer;
    int _noDataCount = 0;

    file_stats_s _status;
    bool _canceled { false };
    bool _singleFile { false }; //send single file

    // notify process for every file
    bool _everyNotify { false };

    // files receive path
    QString _recvPath { "" };

    // bind target ip, which as the worker's id
    QString _bindId;
};

#endif // TRANSFERWORKER_H
