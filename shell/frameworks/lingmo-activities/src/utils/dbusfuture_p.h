/*
    SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_DBUSFUTURE_P_H
#define ACTIVITIES_DBUSFUTURE_P_H

#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcherBase>

#include "debug_p.h"

namespace DBusFuture
{
namespace detail
{ //_

template<typename _Result>
class DBusCallFutureInterface : public QObject, public QFutureInterface<_Result>
{
public:
    DBusCallFutureInterface(QDBusPendingReply<_Result> reply)
        : reply(reply)
        , replyWatcher(nullptr)
    {
    }

    ~DBusCallFutureInterface() override
    {
        delete replyWatcher;
    }

    void callFinished();

    QFuture<_Result> start()
    {
        replyWatcher = new QDBusPendingCallWatcher(reply);

        QObject::connect(replyWatcher, &QDBusPendingCallWatcher::finished, [this]() {
            callFinished();
        });

        this->reportStarted();

        if (reply.isFinished()) {
            this->callFinished();
        }

        return this->future();
    }

private:
    QDBusPendingReply<_Result> reply;
    QDBusPendingCallWatcher *replyWatcher;
};

template<typename _Result>
void DBusCallFutureInterface<_Result>::callFinished()
{
    deleteLater();

    if (!reply.isError()) {
        this->reportResult(reply.value());
    }

    this->reportFinished();
}

template<>
void DBusCallFutureInterface<void>::callFinished();

template<typename _Result>
class ValueFutureInterface : public QObject, QFutureInterface<_Result>
{
public:
    ValueFutureInterface(const _Result &value)
        : value(value)
    {
    }

    QFuture<_Result> start()
    {
        auto future = this->future();

        this->reportResult(value);
        this->reportFinished();

        deleteLater();

        return future;
    }

private:
    _Result value;
};

template<>
class ValueFutureInterface<void> : public QObject, QFutureInterface<void>
{
public:
    ValueFutureInterface();

    QFuture<void> start();
    // {
    //     auto future = this->future();
    //     this->reportFinished();
    //     deleteLater();
    //     return future;
    // }
};

} //^ namespace detail

template<typename _Result, typename... Args>
QFuture<_Result> asyncCall(QDBusAbstractInterface *interface, const QString &method, Args &&...args)
{
    using namespace detail;

    auto callFutureInterface = new DBusCallFutureInterface<_Result>(interface->asyncCall(method, std::forward<Args>(args)...));

    return callFutureInterface->start();
}

template<typename _Result>
QFuture<_Result> fromValue(const _Result &value)
{
    using namespace detail;

    auto valueFutureInterface = new ValueFutureInterface<_Result>(value);

    return valueFutureInterface->start();
}

template<typename _Result>
QFuture<_Result> fromReply(const QDBusPendingReply<_Result> &reply)
{
    using namespace detail;

    auto callFutureInterface = new DBusCallFutureInterface<_Result>(reply);

    return callFutureInterface->start();
}

QFuture<void> fromVoid();

} // namespace DBusFuture

#endif /* ACTIVITIES_DBUSFUTURE_P_H */
