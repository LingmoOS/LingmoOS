// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMPRISMONITOR_H
#define DMPRISMONITOR_H

#include "dbusinterface.h"

#include <QObject>

class MediaMonitor : public QObject
{
    Q_OBJECT

public:
    explicit MediaMonitor(QObject *parent = 0);

    void init();

Q_SIGNALS:
    void mediaAcquired(const QString &path) const;
    void mediaLost(const QString &path) const;

private Q_SLOTS:
    void onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    DBusInterface *m_dbusInter;
};

#endif // DMPRISMONITOR_H
