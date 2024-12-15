// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H

#include <QString>
#include <QObject>
#include <QSharedPointer>

namespace cooperation_core {

class NetworkUtilPrivate;
class NetworkUtil : public QObject
{
    Q_OBJECT
public:
    static NetworkUtil *instance();

    // setting
    void setStorageFolder(const QString &folder);
    QString getStorageFolder() const;
    QString getConfirmTargetAddress() const;

    void pingTarget(const QString &ip);
    void reqTargetInfo(const QString &ip, bool compat);
    void disconnectRemote(const QString &ip);

    // login & combi request
    void compatLogin(const QString &ip);
    void doNextCombiRequest(const QString &ip, bool compat = false);

    //transfer
    void tryTransApply(const QString &ip);
    void sendTransApply(const QString &ip, bool compat);
    void replyTransRequest(bool agree);
    void cancelTrans();
    void doSendFiles(const QStringList &fileList);

    //Keymouse sharing
    void tryShareApply(const QString &ip, const QString &selfprint);
    void sendShareApply(const QString &ip, bool compat);
    void sendDisconnectShareEvents(const QString &ip);
    void replyShareRequest(bool agree, const QString &selfprint);
    void cancelApply(const QString &type);

    QString deviceInfoStr();

#ifdef ENABLE_COMPAT
    //compat share
    void compatSendStartShare(const QString &screenName);
    void stop();
#endif

public slots:
#ifdef ENABLE_COMPAT
    void handleCompatConnectResult(int result, const QString &ip);
    void handleCompatRegister(bool reg, const QString &infoJson);
    void handleCompatDiscover();
#endif

    void trySearchDevice(const QString &ip);
    void updateStorageConfig(const QString &value);

private:
    explicit NetworkUtil(QObject *parent = nullptr);
    ~NetworkUtil();

private:
    QSharedPointer<NetworkUtilPrivate> d { nullptr };
    QString _selfFingerPrint;
    QPair<int, QString> _nextCombi;
};

}   // namespace cooperation_core

#endif   // NETWORKUTIL_H
