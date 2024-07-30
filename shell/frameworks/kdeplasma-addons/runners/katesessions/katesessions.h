/*
 *   SPDX-FileCopyrightText: 2008 Sebastian Kügler <sebas@kde.org>
 *   SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATESESSIONS_H
#define KATESESSIONS_H

#include "profilesmodel.h"
#include <KRunner/AbstractRunner>

using namespace KRunner;

class KateSessions : public AbstractRunner
{
    Q_OBJECT

public:
    explicit KateSessions(QObject *parent, const KPluginMetaData &metaData);

    void init() override;
    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private:
    const QLatin1String m_triggerWord = QLatin1String("kate");
    ProfilesModel *m_model = nullptr; // WARNING: must be on the correct thread. cannot live on stack!
};

#endif
