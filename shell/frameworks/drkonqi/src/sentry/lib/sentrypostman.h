// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#pragma once

#include "sentryconnection.h"

// Collects not sent envelopes and sends them off to sentry every once in a while.
// Will just fail when there is no network connectivity.
// Should things go wrong we'll eventually get killed by systemd.
class SentryPostman : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    void run();

private:
    void post(const QString &path, const QString &filename, const QDateTime &mtime);
    SentryNetworkConnection m_connection;
};
