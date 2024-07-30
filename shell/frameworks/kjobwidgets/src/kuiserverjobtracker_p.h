/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006-2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUISERVERJOBTRACKER_P_H
#define KUISERVERJOBTRACKER_P_H

#include <memory>

#include <QDBusServiceWatcher>

#include "jobviewserverinterface.h"

class Q_DECL_HIDDEN KSharedUiServerProxy : public QObject
{
    Q_OBJECT

public:
    KSharedUiServerProxy();
    ~KSharedUiServerProxy() override;

    org::kde::JobViewServer *uiserver();

Q_SIGNALS:
    void serverRegistered();
    void serverUnregistered();

private:
    void uiserverOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

    std::unique_ptr<org::kde::JobViewServer> m_uiserver;
    std::unique_ptr<QDBusServiceWatcher> m_watcher;
};

#endif
