/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MDNSD_SDEVENT_H
#define MDNSD_SDEVENT_H

#include <QEvent>
#include <QMap>
#include <QString>

namespace KDNSSD
{
enum Operation { SD_ERROR = 101, SD_ADDREMOVE, SD_PUBLISH, SD_RESOLVE };

class ErrorEvent : public QEvent
{
public:
    ErrorEvent()
        : QEvent((QEvent::Type)(QEvent::User + SD_ERROR))
    {
    }
};
class AddRemoveEvent : public QEvent
{
public:
    enum Operation { Add, Remove };
    AddRemoveEvent(Operation op, const QString &name, const QString &type, const QString &domain, bool last)
        : QEvent((QEvent::Type)(QEvent::User + SD_ADDREMOVE))
        , m_op(op)
        , m_name(name)
        , m_type(type)
        , m_domain(domain)
        , m_last(last)
    {
    }

    const Operation m_op;
    const QString m_name;
    const QString m_type;
    const QString m_domain;
    const bool m_last;
};

class PublishEvent : public QEvent
{
public:
    PublishEvent(const QString &name)
        : QEvent((QEvent::Type)(QEvent::User + SD_PUBLISH))
        , m_name(name)
    {
    }

    const QString m_name;
};

class ResolveEvent : public QEvent
{
public:
    ResolveEvent(const QString &hostname, unsigned short port, const QMap<QString, QByteArray> &txtdata)
        : QEvent((QEvent::Type)(QEvent::User + SD_RESOLVE))
        , m_hostname(hostname)
        , m_port(port)
        , m_txtdata(txtdata)
    {
    }

    const QString m_hostname;
    const unsigned short m_port;
    const QMap<QString, QByteArray> m_txtdata;
};

}

#endif
