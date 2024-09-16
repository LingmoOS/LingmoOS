// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "authentication_manager.h"

#include "session.h"

#include <QDebug>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QApplication>

namespace ddc
{

class AuthenticationManagerPrivate
{
public:
    explicit AuthenticationManagerPrivate(AuthenticationManager *p)
        : q_ptr(p)
    {
        ddc::Session::connect(&sess, &Session::authorizeFinished, p, [=](const AuthorizeResponse &resp)
        {
//            QMutexLocker locker(&mutex);

            if (this->authQueue.isEmpty()) {
                qCritical() << "should not empty";
                return;
            }

            // remove from queue;
            if (resp.success) {
                this->authQueue.pop_front();
                qDebug() << "authorizeFinished" << this->authQueue.length();
                Q_EMIT p->authorizeFinished(resp);

                // 如果还有请求，继续
                if (this->authQueue.length() > 0) {
                    auto authReq = this->authQueue.first();
                    this->sess.authorize(authReq);
                }
            }
            else {
                auto authReq = this->authQueue.first();
                Q_EMIT p->requestLogin(authReq);
            }
        });
    }

    Session sess;
    QMutex mutex;

    QQueue<AuthorizeRequest> authQueue;

    AuthenticationManager *q_ptr;
    Q_DECLARE_PUBLIC(AuthenticationManager)
};

AuthenticationManager::AuthenticationManager(QObject *parent)
    : QObject(parent), dd_ptr(new AuthenticationManagerPrivate(this))
{

}

void AuthenticationManager::requestAuthorize(const AuthorizeRequest &authReq)
{
    // lock and wait
    // push req to queue, and deal when login
    Q_D(AuthenticationManager);
//    QMutexLocker locker(&d->mutex);
    if(!d->authQueue.contains(authReq)){
        d->authQueue.push_back(authReq);
    }

    qDebug() << "queue length" << d->authQueue.length();
    // first req, call authorize
    if (d->authQueue.length() == 1) {
        d->sess.authorize(authReq);
    }
}

bool AuthenticationManager::onLogin(const QString &sessionID,
                                    const QString &clientID,
                                    const QString &code,
                                    const QString &state)
{
    // save config
    Q_D(AuthenticationManager);
    // TODO: need crypt

//    QMutexLocker locker(&d->mutex);

    if(d->authQueue.isEmpty())
    {
        qDebug() << "authQueue is empty";
        return false;
    }

    auto authReq = d->authQueue.first();
    if (authReq.clientID != clientID) {
        qCritical() << "error id" << authReq.clientID << clientID;
    }
    AuthorizeResponse resp;
    resp.success = true;
    resp.req = authReq;
    resp.state = authReq.state;
    resp.code = code;
    Q_EMIT d->sess.authorizeFinished(resp);

    return true;
}

bool AuthenticationManager::hasRequest() const
{
    Q_D(const AuthenticationManager);
    return !d->authQueue.isEmpty();
}

void AuthenticationManager::cancel()
{
    Q_D(AuthenticationManager);
    qDebug() << "clean auth queue";
    d->authQueue.clear();
}

void AuthenticationManager::delAuthReq(const QString &clientID)
{
    Q_D(AuthenticationManager);
    int size = d->authQueue.size();

    for (int pos = 0; pos < size; pos++) {
        if(d->authQueue.at(pos).clientID == clientID){
            d->authQueue.removeAt(pos);
        }
    }
}

AuthenticationManager::~AuthenticationManager() = default;

}
