/*
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dbusutils.h"
#include <Plugin.h>

class ActivityRunner : public Plugin
{
    Q_OBJECT

public:
    explicit ActivityRunner(QObject *parent);
    ~ActivityRunner() override;
    bool init(QHash<QString, QObject *> &modules) override;

    void Run(const QString &matchId, const QString &actionId);
    RemoteMatches Match(const QString &query);
    RemoteActions Actions();
    QVariantMap Config();

private:
    QObject *m_activitiesService;
    const QString m_keywordi18n;
    const QString m_keyword;
};
