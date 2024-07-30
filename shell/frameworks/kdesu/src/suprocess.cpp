/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    Sudo support added by Jonathan Riddell <jriddell@ ubuntu.com>
    SPDX-FileCopyrightText: 2005 Canonical Ltd // krazy:exclude=copyright (no email)

    SPDX-License-Identifier: GPL-2.0-only

    su.cpp: Execute a program as another user with "class SuProcess".
*/

#include "suprocess.h"

#include "kcookie_p.h"
#include "stubprocess_p.h"
#include <ksu_debug.h>

#include <QFile>
#include <QStandardPaths>
#include <qplatformdefs.h>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <kuser.h>

#if defined(KDESU_USE_SUDO_DEFAULT)
#define DEFAULT_SUPER_USER_COMMAND QStringLiteral("sudo")
#elif defined(KDESU_USE_DOAS_DEFAULT)
#define DEFAULT_SUPER_USER_COMMAND QStringLiteral("doas")
#else
#define DEFAULT_SUPER_USER_COMMAND QStringLiteral("su")
#endif

namespace KDESu
{
using namespace KDESuPrivate;

class SuProcessPrivate : public StubProcessPrivate
{
public:
    bool isPrivilegeEscalation() const;
    QString superUserCommand;
};

bool SuProcessPrivate::isPrivilegeEscalation() const
{
    return (superUserCommand == QLatin1String("sudo") || superUserCommand == QLatin1String("doas"));
}

SuProcess::SuProcess(const QByteArray &user, const QByteArray &command)
    : StubProcess(*new SuProcessPrivate)
{
    Q_D(SuProcess);

    m_user = user;
    m_command = command;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group(config, QStringLiteral("super-user-command"));
    d->superUserCommand = group.readEntry("super-user-command", DEFAULT_SUPER_USER_COMMAND);

    if (!d->isPrivilegeEscalation() && d->superUserCommand != QLatin1String("su")) {
        qCWarning(KSU_LOG) << "unknown super user command.";
        d->superUserCommand = DEFAULT_SUPER_USER_COMMAND;
    }
}

SuProcess::~SuProcess() = default;

QString SuProcess::superUserCommand()
{
    Q_D(SuProcess);

    return d->superUserCommand;
}

bool SuProcess::useUsersOwnPassword()
{
    Q_D(SuProcess);

    if (d->isPrivilegeEscalation() && m_user == "root") {
        return true;
    }

    KUser user;
    return user.loginName() == QString::fromUtf8(m_user);
}

int SuProcess::checkInstall(const char *password)
{
    return exec(password, Install);
}

int SuProcess::checkNeedPassword()
{
    return exec(nullptr, NeedPassword);
}

/*
 * Execute a command with su(1).
 */
int SuProcess::exec(const char *password, int check)
{
    Q_D(SuProcess);

    if (check) {
        setTerminal(true);
    }

    // since user may change after constructor (due to setUser())
    // we need to override sudo with su for non-root here
    if (m_user != QByteArray("root")) {
        d->superUserCommand = QStringLiteral("su");
    }

    QList<QByteArray> args;
    if (d->isPrivilegeEscalation()) {
        args += "-u";
    }

    if (m_scheduler != SchedNormal || m_priority > 50) {
        args += "root";
    } else {
        args += m_user;
    }

    if (d->superUserCommand == QLatin1String("su")) {
        args += "-c";
    }
    // Get the kdesu_stub and su command from a config file if set, used in test
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group(config, QStringLiteral("super-user-command"));
    const QString defaultPath = QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF) + QStringLiteral("/kdesu_stub");
    const QString kdesuStubPath = group.readEntry("kdesu_stub_path", defaultPath);
    args += kdesuStubPath.toLocal8Bit();
    args += "-"; // krazy:exclude=doublequote_chars (QList, not QString)

    const QString commandString = group.readEntry("command", QStandardPaths::findExecutable(d->superUserCommand));
    const QByteArray command = commandString.toLocal8Bit();
    if (command.isEmpty()) {
        return check ? SuNotFound : -1;
    }

    // Turn echo off for conversion with kdesu_stub. Needs to be done before
    // it's started so that sudo copies this option to its internal PTY.
    enableLocalEcho(false);

    if (StubProcess::exec(command, args) < 0) {
        return check ? SuNotFound : -1;
    }

    SuErrors ret = (SuErrors)converseSU(password);

    if (ret == error) {
        if (!check) {
            qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                                << "Conversation with" << d->superUserCommand << "failed.";
        }
        return ret;
    }
    if (check == NeedPassword) {
        if (ret == killme) {
            if (d->isPrivilegeEscalation()) {
                // sudo can not be killed, just return
                return ret;
            }
            if (kill(m_pid, SIGKILL) < 0) {
                // FIXME SIGKILL doesn't work for sudo,
                // why is this different from su?
                // A: because sudo runs as root. Perhaps we could write a Ctrl+C to its stdin, instead?
                ret = error;
            } else {
                int iret = waitForChild();
                if (iret < 0) {
                    ret = error;
                }
            }
        }
        return ret;
    }

    if (m_erase && password) {
        memset(const_cast<char *>(password), 0, qstrlen(password));
    }

    if (ret != ok) {
        kill(m_pid, SIGKILL);
        if (d->isPrivilegeEscalation()) {
            waitForChild();
        }
        return SuIncorrectPassword;
    }

    int iret = converseStub(check);
    if (iret < 0) {
        if (!check) {
            qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                                << "Conversation with kdesu_stub failed.";
        }
        return iret;
    } else if (iret == 1) {
        kill(m_pid, SIGKILL);
        waitForChild();
        return SuIncorrectPassword;
    }

    if (check == Install) {
        waitForChild();
        return 0;
    }

    iret = waitForChild();
    return iret;
}

/*
 * Conversation with su: feed the password.
 * Return values: -1 = error, 0 = ok, 1 = kill me, 2 not authorized
 */
int SuProcess::converseSU(const char *password)
{
    enum {
        WaitForPrompt,
        CheckStar,
        HandleStub,
    } state = WaitForPrompt;
    int colon;
    unsigned i;
    unsigned j;

    QByteArray line;
    while (true) {
        line = readLine();
        // return if problem. sudo checks for a second prompt || su gets a blank line
        if ((line.contains(':') && state != WaitForPrompt) || line.isNull()) {
            return (state == HandleStub ? notauthorized : error);
        }

        if (line == "kdesu_stub") {
            unreadLine(line);
            return ok;
        }

        switch (state) {
        case WaitForPrompt: {
            if (waitMS(fd(), 100) > 0) {
                // There is more output available, so this line
                // couldn't have been a password prompt (the definition
                // of prompt being that  there's a line of output followed
                // by a colon, and then the process waits).
                continue;
            }

            const uint len = line.length();
            // Match "Password: " with the regex ^[^:]+:[\w]*$.
            for (i = 0, j = 0, colon = 0; i < len; ++i) {
                if (line[i] == ':') {
                    j = i;
                    colon++;
                    continue;
                }
                if (!isspace(line[i])) {
                    j++;
                }
            }
            if (colon == 1 && line[j] == ':') {
                if (password == nullptr) {
                    return killme;
                }
                if (waitSlave()) {
                    return error;
                }
                write(fd(), password, strlen(password));
                write(fd(), "\n", 1);
                state = CheckStar;
            }
            break;
        }
        //////////////////////////////////////////////////////////////////////////
        case CheckStar: {
            const QByteArray s = line.trimmed();
            if (s.isEmpty()) {
                state = HandleStub;
                break;
            }
            const bool starCond = std::any_of(s.cbegin(), s.cend(), [](const char c) {
                return c != '*';
            });
            if (starCond) {
                return error;
            }
            state = HandleStub;
            break;
        }
        //////////////////////////////////////////////////////////////////////////
        case HandleStub:
            break;
            //////////////////////////////////////////////////////////////////////////
        } // end switch
    } // end while (true)
    return ok;
}

void SuProcess::virtual_hook(int id, void *data)
{
    StubProcess::virtual_hook(id, data);
}

} // namespace KDESu
