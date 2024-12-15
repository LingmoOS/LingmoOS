// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "session.h"

Session::Session(QString name, QString session, QString signal, QObject *parent)
    : QObject(parent)
    , _name(name)
    , _sessionid(session)
    , _signalname(signal)
{
    _jobs.clear();

    _pingOK = true;
}

Session::~Session()
{
}

bool Session::alive()
{
    return _pingOK;
}

QString Session::getName()
{
    return _name;
}

QString Session::getSession()
{
    return _sessionid;
}

void Session::addJob(int jobid)
{
    _jobs.push_back(jobid);
}

bool Session::removeJob(int jobid)
{
    int i = hasJob(jobid);
    if (i >= 0) {
        _jobs.remove(i);
        return true;
    }
    return false;
}

int Session::hasJob(int jobid)
{
    for (int i = 0; i < _jobs.size(); ++i) {
        if (_jobs[i] == jobid) {
            return i;
        }
    }
    return -1;
}

QString Session::signal()
{
    return _signalname;
}

