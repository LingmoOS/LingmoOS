// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H

#include <QString>
#include <QObject>
#include <QSharedPointer>

class NetworkUtilPrivate;
class NetworkUtil : public QObject
{
    Q_OBJECT
public:
    static NetworkUtil *instance();

    // setting
    bool doConnect(const QString &ip, const QString &password);
    void disConnect();
    void updatePassword(const QString &code);
    void updateStorageConfig();
    bool sendMessage(const QString &msg);

    //transfer
    void cancelTrans();
    void doSendFiles(const QStringList &fileList);

    // compat login
    void compatLogin();

public slots:
    void handleMiscMessage(const QString &msg);
    // compat old protocol
#ifdef ENABLE_COMPAT
    void handleCompatConnectResult(int result, const QString &ip);
    void compatTransJobStatusChanged(int id, int result, const QString &msg);
    void compatFileTransStatusChanged(const QString &path, quint64 total, quint64 current, quint64 millisec);
    void stop();
#endif

private:
    explicit NetworkUtil(QObject *parent = nullptr);
    ~NetworkUtil();

private:
    QSharedPointer<NetworkUtilPrivate> d { nullptr };
    QPair<QString, QString> _loginCombi;
};

#endif   // NETWORKUTIL_H
