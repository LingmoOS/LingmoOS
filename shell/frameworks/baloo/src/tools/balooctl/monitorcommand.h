/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MONITOR_H
#define MONITOR_H

#include "command.h"
#include "fileindexerinterface.h"
#include "schedulerinterface.h"
#include <QObject>
#include <QTextStream>
#include <KLocalizedString>

namespace Baloo {

class MonitorCommand : public QObject, public Command
{
    Q_OBJECT
public:
    explicit MonitorCommand(QObject* parent = nullptr);

    QString command() override {
        return QStringLiteral("monitor");
    }

    QString description() override {
        return i18n("CLI interface for monitoring Baloo");
    }

    int exec(const QCommandLineParser& parser) override;

private Q_SLOTS:
    void startedIndexingFile(const QString& filePath);
    void finishedIndexingFile(const QString& filePath);
    void stateChanged(int state);
    void balooIsAvailable();
    void balooIsNotAvailable();

private:
    QTextStream m_out;
    QTextStream m_err;
    org::kde::baloo::fileindexer* m_indexerDBusInterface;
    org::kde::baloo::scheduler* m_schedulerDBusInterface;
    QString m_currentFile;
    QDBusServiceWatcher* m_dbusServiceWatcher;
};
}
#endif // MONITOR_H
