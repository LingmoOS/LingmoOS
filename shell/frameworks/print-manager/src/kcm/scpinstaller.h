/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
*/

#pragma once

#include <PackageKit/Daemon>
#include <PackageKit/Transaction>

class SCPInstaller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool installing READ isInstalling NOTIFY installingChanged)
    Q_PROPERTY(bool installed READ isInstalled NOTIFY installedChanged)
    Q_PROPERTY(bool failed READ hasFailed NOTIFY failedChanged)
public:
    using QObject::QObject;

public Q_SLOTS:
    void install();
    bool isInstalling() const;
    bool hasFailed() const;

    static bool isInstalled();

Q_SIGNALS:
    void installingChanged();
    void installedChanged();
    void failedChanged();
    void error(const QString &msg);

private Q_SLOTS:
    void packageFinished(PackageKit::Transaction::Exit status);

private:
    void setFailed(bool failed); // **un**sets installing if it is set as well
    void setInstalling(bool installing); // **un**sets failed if set

    bool m_installing = false;
    bool m_failed = false;
};
