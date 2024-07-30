/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef KDESUSUPROCESS_H
#define KDESUSUPROCESS_H

#include <kdesu/kdesu_export.h>

#include "stubprocess.h"

namespace KDESu
{
class SuProcessPrivate;

/** \class SuProcess suprocess.h KDESu/SuProcess
 * Executes a command under elevated privileges, using su.
 */

class KDESU_EXPORT SuProcess : public StubProcess
{
public:
    enum Errors {
        SuNotFound = 1,
        SuNotAllowed,
        SuIncorrectPassword,
    };

    /**
     * Executes the command. This will wait for the command to finish.
     */
    enum checkMode {
        NoCheck = 0,
        Install = 1,
        NeedPassword = 2,
    };

    explicit SuProcess(const QByteArray &user = nullptr, const QByteArray &command = nullptr);
    ~SuProcess() override;

    int exec(const char *password, int check = NoCheck);

    /**
     * Checks if the stub is installed and the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Checks if a password is needed.
     */
    int checkNeedPassword();

    /**
     * Checks what the default super user command is, e.g. sudo, su, etc
     * @return the default super user command
     */
    QString superUserCommand();

    /**
     * Checks whether or not the user's password is being asked for or another
     * user's password. Due to usage of systems such as sudo, even when attempting
     * to switch to another user one may need to enter their own password.
     */
    bool useUsersOwnPassword();

protected:
    void virtual_hook(int id, void *data) override;

private:
    enum SuErrors {
        error = -1,
        ok = 0,
        killme = 1,
        notauthorized = 2,
    };

    KDESU_NO_EXPORT int converseSU(const char *password);

private:
    Q_DECLARE_PRIVATE(SuProcess)
};

}

#endif // KDESUSUPROCESS_H
