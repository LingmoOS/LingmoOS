
/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <QEventLoop>
#include <QThread>
#include <QTimer>

using namespace KRunner;

class SuspendedRunner : public AbstractRunner
{
public:
    explicit SuspendedRunner(QObject *parent, const KPluginMetaData &metadata)
        : AbstractRunner(parent, metadata)
    {
    }
    void reloadConfiguration() override
    {
        QThread::msleep(3000);
    }

    void match(RunnerContext &context) override
    {
        QueryMatch m(this);
        m.setText("bla");
        context.addMatch(m);
    }
};

K_PLUGIN_CLASS_WITH_JSON(SuspendedRunner, "metadatafile1.json")

#include "suspendedrunner.moc"
