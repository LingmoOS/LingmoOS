// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
class QCommandLineParser;
QT_END_NAMESPACE

class SingleInstance : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstance(const QString &name, QObject *parent = nullptr);
    ~SingleInstance();
    bool isSingle();
    void closeServer();
    void sendNewClient();
    QLocalSocket *getNewClientConnect(const QByteArray &message);
signals:
    void handArguments(const QStringList &);
private slots:
    void handleConnection();
    void readData();
private:
    void initConnect();

protected:
    QLocalServer *m_localServer { nullptr };
    QString m_userKey;
};

#endif // SINGLEINSTANCE_H
