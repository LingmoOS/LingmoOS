/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QObject>
#include <QSocketNotifier>

#include <systemd/sd-journal.h>

#include "memory.h"

class Coredump;
class CoredumpWatcher : public QObject
{
    Q_OBJECT
public:
    explicit CoredumpWatcher(std::unique_ptr<sd_journal> context_, QString bootId_, const QString &instance_, QObject *parent = nullptr);

    // must be called before start!
    void addMatch(const QString &str);
    void start();

Q_SIGNALS:
    void finished();
    void error(const QString &msg);
    void newDump(const Coredump &dump);
    /// Emitted when the current iteration has reached the log end. Roughly meaning that it has loaded all past entries.
    void atLogEnd();

private:
    void processLog();
    void errnoError(const QString &msg, int err);

    const std::unique_ptr<sd_journal> context = nullptr;
    std::unique_ptr<QSocketNotifier> notifier = nullptr;
    const QString bootId;
    const QString instance;
    const QString instanceFilter; // systemd-coredump@%1 instance name
    QStringList matches;
};
