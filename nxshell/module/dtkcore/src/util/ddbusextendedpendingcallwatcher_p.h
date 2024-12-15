// SPDX-FileCopyrightText: 2015 Jolla Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef DDBUSEXTENDEDPENDINGCALLWATCHER_P_H
#define DDBUSEXTENDEDPENDINGCALLWATCHER_P_H
#include "dtkcore_global.h"
#include <QDBusError>
#include <QDBusPendingCallWatcher>

DCORE_BEGIN_NAMESPACE
class DDBusExtendedPendingCallWatcher : public QDBusPendingCallWatcher
{
    Q_OBJECT

public:
    explicit DDBusExtendedPendingCallWatcher(const QDBusPendingCall &call,
                                             const QString &asyncProperty,
                                             const QVariant &previousValue,
                                             QObject *parent = 0);
    ~DDBusExtendedPendingCallWatcher();

    Q_PROPERTY(QString AsyncProperty READ asyncProperty)
    inline QString asyncProperty() const { return m_asyncProperty; }

    Q_PROPERTY(QVariant PreviousValue READ previousValue)
    inline QVariant previousValue() const { return m_previousValue; }

private:
    QString m_asyncProperty;
    QVariant m_previousValue;
};
DCORE_END_NAMESPACE

#endif /* DDBUSEXTENDEDPENDINGCALLWATCHER_P_H */
