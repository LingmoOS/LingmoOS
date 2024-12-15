// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountstypes_p.h"

#include <DDBusInterface>

#include <QDBusPendingReply>
#include <QStringList>

DACCOUNTS_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;

class DSystemUserInterface : public QObject
{
    Q_OBJECT

public:
    explicit DSystemUserInterface(const QString &path, QObject *parent = nullptr);
    virtual ~DSystemUserInterface() = default;

    Q_PROPERTY(bool AutomaticLogin READ automaticLogin WRITE setAutomaticLogin NOTIFY
                       AutomaticLoginChanged)
    Q_PROPERTY(qint32 AccountType READ accountType NOTIFY AccountTypeChanged)
    Q_PROPERTY(QStringList Groups READ groups WRITE setGroups NOTIFY GroupsChanged)
    Q_PROPERTY(QStringList HistoryLayout READ historyLayout WRITE setHistoryLayout NOTIFY
                       HistoryLayoutChanged)
    Q_PROPERTY(QStringList IconList READ iconList NOTIFY IconListChanged)
    Q_PROPERTY(QString IconFile READ iconFile WRITE setIconFile NOTIFY IconFileChanged)
    Q_PROPERTY(QString Layout READ layout WRITE setLayout NOTIFY LayoutChanged)
    Q_PROPERTY(QString Locale READ locale WRITE setLocale NOTIFY LocaleChanged)
    Q_PROPERTY(qint32 MaxPasswordAge READ maxPasswordAge WRITE setMaxPasswordAge NOTIFY
                       MaxPasswordAgeChanged)
    Q_PROPERTY(qint32 PasswordLastChange READ passwordLastChange NOTIFY PasswordLastChangeChanged)
    Q_PROPERTY(bool NoPasswdLogin READ noPasswdLogin WRITE enableNoPasswdLogin NOTIFY
                       NoPasswdLoginChanged)
    Q_PROPERTY(
            QString PasswordHint READ passwordHint WRITE setPasswordHint NOTIFY PasswordHintChanged)
    Q_PROPERTY(bool Locked READ locked WRITE setLocked NOTIFY LockedChanged)
    Q_PROPERTY(QString UUID READ UUID NOTIFY UUIDChanged)
    Q_PROPERTY(quint64 createdTime READ createdTime NOTIFY CreatedTimeChanged)

    bool automaticLogin() const;
    qint32 accountType() const;
    QStringList iconList() const;
    QStringList historyLayout() const;
    QString iconFile() const;
    QString layout() const;
    QStringList groups() const;
    QString locale() const;
    bool locked() const;
    qint32 maxPasswordAge() const;
    qint32 passwordLastChange() const;
    bool noPasswdLogin() const;
    QString passwordHint() const;
    QString UUID() const;
    quint64 createdTime();

public slots:
    QDBusPendingReply<void> addGroup(const QString &group);
    QDBusPendingReply<void> deleteGroup(const QString &group);
    QDBusPendingReply<void> deleteIconFile(const QString &icon);
    QDBusPendingReply<void> enableNoPasswdLogin(const bool enabled);
    QDBusPendingReply<qint32, qint64> passwordExpiredInfo();
    QDBusPendingReply<ReminderInfo_p> getReminderInfo() const;
    QDBusPendingReply<QList<qint32>> getSecretQuestions() const;
    QDBusPendingReply<void> setAutomaticLogin(const bool enabled);
    QDBusPendingReply<void> setHistoryLayout(const QStringList &list);
    QDBusPendingReply<void> setIconFile(const QString &iconURI);
    QDBusPendingReply<void> setLayout(const QString &layout);
    QDBusPendingReply<void> setGroups(const QStringList &groups);
    QDBusPendingReply<void> setLocale(const QString &locale);
    QDBusPendingReply<void> setLocked(const bool locked);
    QDBusPendingReply<void> setMaxPasswordAge(qint32 nDays);
    QDBusPendingReply<void> setPassword(const QString &password);
    QDBusPendingReply<void> setPasswordHint(const QString &hint);
    QDBusPendingReply<void> setSecretQuestions(const QMap<qint32, QByteArray> &list);
    QDBusPendingReply<QList<qint32>> verifySecretQuestions(const QMap<qint32, QString> &anwsers);

signals:
    void AutomaticLoginChanged(const bool enabled);
    void AccountTypeChanged(const qint32 type);
    void GroupsChanged(const QStringList &groups);
    void HistoryLayoutChanged(const QStringList &list);
    void IconListChanged(const QStringList &list);
    void IconFileChanged(const QString &iconURI);
    void LayoutChanged(const QString &layout);
    void LocaleChanged(const QString &locale);
    void LockedChanged(const bool locked);
    void MaxPasswordAgeChanged(const qint32 nDays);
    void NoPasswdLoginChanged(const bool enabled);
    void PasswordHintChanged(const QString &passwordHint);
    void UUIDChanged(const QString &UUID);
    void PasswordLastChangeChanged(const qint32 lstch);
    void CreatedTimeChanged(const quint64 time);

private:
    DDBusInterface *m_inter;
};

DACCOUNTS_END_NAMESPACE
