// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountstypes.h"

#include <DDBusInterface>

#include <QDBusPendingReply>

DACCOUNTS_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;

class DUserInterface : public QObject
{
    Q_OBJECT

public:
    explicit DUserInterface(const QString &path, QObject *parent = nullptr);
    ~DUserInterface() = default;

    Q_PROPERTY(bool automaticLogin READ automaticLogin)
    Q_PROPERTY(bool localAccount READ localAccount)
    Q_PROPERTY(bool locked READ locked)
    Q_PROPERTY(bool systemAccount READ systemAccount)
    Q_PROPERTY(qint32 accountType READ accountType)
    Q_PROPERTY(qint32 passwordMode READ passwordMode)
    Q_PROPERTY(qint64 loginTime READ loginTime)
    Q_PROPERTY(QString email READ email)
    Q_PROPERTY(QString homeDirectory READ homeDirectory)
    Q_PROPERTY(QString iconFile READ iconFile)
    Q_PROPERTY(QString language READ language)
    Q_PROPERTY(QString location READ location)
    Q_PROPERTY(QString passwordHint READ passwordHint)
    Q_PROPERTY(QString realName READ realName)
    Q_PROPERTY(QString shell READ shell)
    Q_PROPERTY(QString userName READ userName)
    Q_PROPERTY(QString xSession READ xSession)
    Q_PROPERTY(quint64 loginFrequency READ loginFrequency)
    Q_PROPERTY(quint64 UID READ UID)

    bool automaticLogin() const;
    bool localAccount() const;
    bool locked() const;
    bool systemAccount() const;
    qint32 accountType() const;
    qint32 passwordMode() const;
    qint64 loginTime() const;
    QString email() const;
    QString homeDirectory() const;
    QString iconFile() const;
    QString language() const;
    QString location() const;
    QString passwordHint() const;
    QString realName() const;
    QString shell() const;
    QString userName() const;
    QString xSession() const;
    quint64 loginFrequency() const;
    quint64 UID() const;

signals:
    void DataChanged();

public slots:
    QDBusPendingReply<qint64, // expiration time
                      qint64, // last_change_time
                      qint64, // min days between changes
                      qint64, // max days between changes
                      qint64, // days to warn
                      qint64  // days after expiration until lock
                      >
    getPasswordExpirationPolicy();
    QDBusPendingReply<void> setAccountType(qint32 accountType);
    QDBusPendingReply<void> setAutomaticLogin(bool enabled);
    QDBusPendingReply<void> setEmail(const QString &email);
    QDBusPendingReply<void> setHomeDirectory(const QString &homedir);
    QDBusPendingReply<void> setIconFile(const QString &filename);
    QDBusPendingReply<void> setLanguage(const QString &language);
    QDBusPendingReply<void> setLocation(const QString &location);
    QDBusPendingReply<void> setLocked(bool locked);
    QDBusPendingReply<void> setPassword(const QString &password, const QString &hint);
    QDBusPendingReply<void> setPasswordHint(const QString &hint);
    QDBusPendingReply<void> setPasswordMode(qint32 mode);
    QDBusPendingReply<void> setRealName(const QString &name);
    QDBusPendingReply<void> setShell(const QString &shell);
    QDBusPendingReply<void> setUserName(const QString &name);
    QDBusPendingReply<void> setXSession(const QString &x_session);

private:
    DDBusInterface *m_inter;
};

DACCOUNTS_END_NAMESPACE
