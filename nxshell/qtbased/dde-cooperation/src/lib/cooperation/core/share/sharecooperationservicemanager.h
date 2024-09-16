// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECOOPERATIONSERVICEMANAGER_H
#define SHARECOOPERATIONSERVICEMANAGER_H

#include "sharecooperationservice.h"

#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QSharedPointer>

class ShareCooperationServiceManager : public QObject
{
    Q_OBJECT
public:
    ~ShareCooperationServiceManager() override;
    static ShareCooperationServiceManager *instance();

    QSharedPointer<ShareCooperationService> client();
    QSharedPointer<ShareCooperationService> server();
    void stop();
    bool startServer(const QString &msg);
    bool stopServer();

    QString barrierProfile();

signals:
    void startShareServer(const QString msg);
    void stopShareServer();
    void startServerResult(const bool result, const QString msg);
public slots:
    void handleStartShareSever(const QString msg);
    void handleStopShareSever();

private:
    explicit ShareCooperationServiceManager(QObject *parent = nullptr);

private:
    QSharedPointer<ShareCooperationService> _client { nullptr };
    QSharedPointer<ShareCooperationService> _server { nullptr };
};

#endif   // SHARECOOPERATIONSERVICEMANAGER_H
