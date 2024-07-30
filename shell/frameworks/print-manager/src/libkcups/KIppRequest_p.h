/*
    SPDX-FileCopyrightText: 2010-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIPPREQUEST_P_H
#define KIPPREQUEST_P_H

#include <KCupsConnection.h>

class KCupsRawRequest
{
public:
    ipp_tag_t group;
    ipp_tag_t valueTag;
    QString name;
    QVariant value;
};

class KIppRequestPrivate
{
public:
    void addRequest(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QVariant &value);
    void addRawRequestsToIpp(ipp_t *ipp) const;

    ipp_op_t operation;
    QString resource;
    QString filename;
    mutable QList<KCupsRawRequest> rawRequests;
};

static const char **qStringListToCharPtrPtr(const QStringList &list, QList<QByteArray> &qbaList)
{
    const char **ptr = new const char *[list.size() + 1];
    qbaList.reserve(qbaList.size() + list.size());
    QByteArray qba;
    for (int i = 0; i < list.size(); ++i) {
        qba = list.at(i).toUtf8();
        qbaList.append(qba);
        ptr[i] = qba.constData();
    }
    ptr[list.size()] = nullptr;
    return ptr;
}

#endif // KIPPREQUEST_P_H
