/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLIENTPIDMAP_H
#define CLIENTPIDMAP_H

#include "kcontacts_export.h"

#include <QMap>
#include <QSharedDataPointer>
#include <QString>

class ClientPidMapTest;

namespace KContacts
{
class ParameterMap;

/** @short Class that holds a ClientPidMap for a contact.
 *  @since 5.4
 */
class KCONTACTS_EXPORT ClientPidMap
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &, const ClientPidMap &);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &, ClientPidMap &);
    friend class VCardTool;
    friend class ::ClientPidMapTest;

public:
    ClientPidMap();
    ClientPidMap(const ClientPidMap &other);
    ClientPidMap(const QString &clientpidmap);

    ~ClientPidMap();

    typedef QList<ClientPidMap> List;

    void setClientPidMap(const QString &clientpidmap);
    Q_REQUIRED_RESULT QString clientPidMap() const;

    Q_REQUIRED_RESULT bool isValid() const;

    Q_REQUIRED_RESULT bool operator==(const ClientPidMap &other) const;
    Q_REQUIRED_RESULT bool operator!=(const ClientPidMap &other) const;

    ClientPidMap &operator=(const ClientPidMap &other);

    Q_REQUIRED_RESULT QString toString() const;

private:
    // exported for ClientPidMapTest
    void setParams(const ParameterMap &params);
    Q_REQUIRED_RESULT ParameterMap params() const;

    class Private;
    QSharedDataPointer<Private> d;
};
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const ClientPidMap &object);

KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, ClientPidMap &object);
}
Q_DECLARE_TYPEINFO(KContacts::ClientPidMap, Q_RELOCATABLE_TYPE);
#endif // CLIENTPIDMAP_H
