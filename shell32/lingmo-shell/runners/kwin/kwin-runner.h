/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <krunner/abstractrunner.h>

using namespace Plasma;

class KWinRunner : public AbstractRunner
{
    Q_OBJECT

public:
    explicit KWinRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KWinRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &action) override;

private:
    void checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    bool m_enabled = false;
};
