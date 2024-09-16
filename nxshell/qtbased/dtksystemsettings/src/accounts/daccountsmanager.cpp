// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountsmanager.h"

#include "daccountsmanager_p.h"
#include "daccountsuser.h"

#include <QDebug>

#include <grp.h>

DACCOUNTS_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DAccountsManagerPrivate::DAccountsManagerPrivate(DAccountsManager *parent)
    : q_ptr(parent)
    , m_dAccountsInter(new DAccountsInterface(this))
    , m_dSystemAccountsInter(new DSystemAccountsInterface(this))
{
}

DAccountsManager::DAccountsManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DAccountsManagerPrivate(this))
{
    Q_D(const DAccountsManager);
    connect(d->m_dSystemAccountsInter,
            &DSystemAccountsInterface::ReceivedUserAdded,
            this,
            [this, &d](const QString &user) {
                emit this->UserAdded(d->getUIDFromObjectPath(user));
            });
    connect(d->m_dSystemAccountsInter,
            &DSystemAccountsInterface::ReceivedUserDeleted,
            this,
            [this, &d](const QString &user) {
                emit this->UserDeleted(d->getUIDFromObjectPath(user));
            });
}

DAccountsManager::~DAccountsManager() { }

DExpected<QList<quint64>> DAccountsManager::userList() const
{
    Q_D(const DAccountsManager);
    QList<quint64> list;
    auto reply = d->m_dAccountsInter->listCachedUsers();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DCORE_NAMESPACE::emplace_tag::USE_EMPLACE,
                            reply.error().type(),
                            reply.error().message() };
    }
    for (const auto &user : reply.value()) {
        list.append(d->getUIDFromObjectPath(user.path()));
    }
    return list;
}

DExpected<QSharedPointer<DAccountsUser>> DAccountsManager::createUser(const QString &name,
                                                                      const QString &fullName,
                                                                      const AccountTypes &type)
{
    Q_D(const DAccountsManager);
    auto reply = d->m_dSystemAccountsInter->createUser(name, fullName, static_cast<qint32>(type));
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    auto uid = d->getUIDFromObjectPath(reply.value().path());
    QSharedPointer<DAccountsUser> ptr(new DAccountsUser(uid, nullptr));
    return ptr;
}

DExpected<void> DAccountsManager::deleteUser(const QString &name, bool rmFiles)
{
    Q_D(const DAccountsManager);
    auto reply = d->m_dSystemAccountsInter->deleteUser(name, rmFiles);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<QSharedPointer<DAccountsUser>> DAccountsManager::findUserByName(const QString &name)
{
    Q_D(const DAccountsManager);
    auto reply = d->m_dAccountsInter->findUserByName(name);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    auto uid = d->getUIDFromObjectPath(reply.value().path());
    QSharedPointer<DAccountsUser> ptr(new DAccountsUser(uid, nullptr));
    return ptr;
}

DExpected<QSharedPointer<DAccountsUser>> DAccountsManager::findUserById(const qint64 uid)
{
    Q_D(const DAccountsManager);
    auto reply = d->m_dAccountsInter->findUserById(uid);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    QSharedPointer<DAccountsUser> ptr(new DAccountsUser(uid, nullptr));
    return ptr;
}

DExpected<QStringList> DAccountsManager::groups()
{
    QStringList list;
    struct group *grp = nullptr;
    errno = 0;
    while ((grp = getgrent()) != nullptr)
        list.push_back(grp->gr_name);
    if (errno != 0) {
        list.clear();
        endgrent();
        return DUnexpected<>{ DError{ errno, strerror(errno) } };
    }
    endgrent();
    return list;
}

DExpected<QStringList> DAccountsManager::presetGroups(const AccountTypes &type)
{
    Q_D(const DAccountsManager);
    if (type == AccountTypes::Unknown) {
        return DUnexpected<>{ DError{ -1, QString("Unknown account type") } };
    }
    auto reply = d->m_dSystemAccountsInter->getPresetGroups(static_cast<qint32>(type));
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return reply.value();
}

DExpected<ValidMsg> DAccountsManager::isPasswordValid(const QString &password)
{
    Q_D(const DAccountsManager);
    ValidMsg msg;
    auto reply = d->m_dSystemAccountsInter->isPasswordValid(password);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }

    const auto &valid = reply.argumentAt(0);
    if (!valid.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: valid is invalid") } };
    } else {
        msg.valid = valid.toBool();
    }

    const auto &errmsg = reply.argumentAt(1);
    if (!msg.valid and !errmsg.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: errmsg is invalid") } };
    } else {
        msg.msg = errmsg.toString();
    }

    const auto &errcode = reply.argumentAt(2);
    if (!msg.valid and !errcode.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: errcode is invalid") } };
    } else {
        msg.code = errcode.toInt();
    }

    return msg;
}

DExpected<ValidMsg> DAccountsManager::isUsernameValid(const QString &username)
{
    Q_D(const DAccountsManager);
    ValidMsg msg;
    auto reply = d->m_dSystemAccountsInter->isUsernameValid(username);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }

    const auto &valid = reply.argumentAt(0);
    if (!valid.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: valid is invalid") } };
    }
    msg.valid = valid.toBool();

    const auto &errmsg = reply.argumentAt(1);
    if (!msg.valid and !errmsg.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: errmsg is invalid") } };
    }
    msg.msg = errmsg.toString();

    const auto &errcode = reply.argumentAt(2);
    if (!msg.valid and !errcode.isValid()) {
        return DUnexpected<>{ DError{ -1, QString("can't get ValidMsg: errcode is invalid") } };
    }
    msg.code = errcode.toInt();

    return msg;
}

quint64 DAccountsManagerPrivate::getUIDFromObjectPath(const QString &path) const
{
    auto name = *(path.split("/").rbegin());
    int index = name.indexOf("r");
    return name.mid(index + 1).toUInt();
}

DACCOUNTS_END_NAMESPACE
