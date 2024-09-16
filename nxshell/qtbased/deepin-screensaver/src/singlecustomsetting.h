// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLECUSTOMSETTING_H
#define SINGLECUSTOMSETTING_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
class QCommandLineParser;
QT_END_NAMESPACE

class CustomConfig;

class SingleCustomSetting : public QObject
{
    Q_OBJECT
public:
    explicit SingleCustomSetting(QObject *parent = nullptr);
    ~SingleCustomSetting();

    bool isSingleConfig();
    bool startCustomConfig();

private:
    QLocalSocket *getNewClientConnect(const QByteArray &message);
    void userServerName();
    void handleNewClient();
    void initConnect();

public slots:
    void handleConnection();
    void readData();
    void closeServer();

private:
    QLocalServer *m_localServer { nullptr };
    CustomConfig *m_conf { nullptr };
    QString m_userKey;
};

#endif   // SINGLECUSTOMSETTING_H
