/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <powerdevilaction.h>
#include <powerdevilenums.h>

namespace PowerDevil::BundledActions
{
class RunScript : public PowerDevil::Action
{
    Q_OBJECT

public:
    explicit RunScript(QObject *parent);
    ~RunScript() override;

protected:
    void onProfileUnload() override;
    void onIdleTimeout(std::chrono::milliseconds timeout) override;
    void onProfileLoad(const QString &previousProfile, const QString &newProfile) override;
    void triggerImpl(const QVariantMap &args) override;

public:
    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;

private:
    void runCommand(const QString &command);
    QString m_profileLoadCommand;
    QString m_profileUnloadCommand;
    QString m_idleTimeoutCommand;
};

}
