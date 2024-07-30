/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUISERVERV2JOBTRACKER_P_H
#define KUISERVERV2JOBTRACKER_P_H

#include <memory>

#include <QDBusServiceWatcher>

#include "jobviewserverv2interface.h"

class Q_DECL_HIDDEN KSharedUiServerV2Proxy : public QObject
{
    Q_OBJECT

public:
    KSharedUiServerV2Proxy();
    ~KSharedUiServerV2Proxy() override;

    org::kde::JobViewServerV2 *uiserver();

Q_SIGNALS:
    void serverRegistered();
    void serverUnregistered();

private:
    void uiserverOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

    std::unique_ptr<org::kde::JobViewServerV2> m_uiserver;
    std::unique_ptr<QDBusServiceWatcher> m_watcher;
};

#endif
