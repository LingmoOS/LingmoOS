/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only

    client.h: client to access kdesud.
*/

#ifndef KDESUCLIENT_H
#define KDESUCLIENT_H

#include <kdesu/kdesu_export.h>

#include <QByteArray>
#include <QList>
#include <memory>

#ifdef Q_OS_UNIX

namespace KDESu
{
/** \class Client client.h KDESu/Client
 * A client class to access kdesud, the KDE su daemon. Kdesud can assist in
 * password caching in two ways:
 *
 * @li For high security passwords, like for su and ssh, it executes the
 * password requesting command for you. It feeds the password to the
 * command, without ever returning it to you, the user.
 * See exec, setPass, delCommand.
 *
 * @li For lower security passwords, like web and ftp passwords, it can act
 * as a persistent storage for string variables. These variables are
 * returned to the user.
 * See setVar, delVar, delGroup.
 *
 * Porting from KF5 to KF6:
 *
 * The class KDESu::KDEsuClient was renamed to KDESu::Client.
 *
 * @since 6.0
 */
class KDESU_EXPORT Client
{
public:
    Client();
    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    /**
     * Lets kdesud execute a command. If the daemon does not have a password
     * for this command, this will fail and you need to call setPass().
     *
     * @param command The command to execute.
     * @param user The user to run the command as.
     * @param options Extra options.
     * @param env Extra environment variables.
     * @return Zero on success, -1 on failure.
     */
    int exec(const QByteArray &command, const QByteArray &user, const QByteArray &options = nullptr, const QList<QByteArray> &env = QList<QByteArray>());

    /**
     * Wait for the last command to exit and return the exit code.
     * @return Exit code of last command, -1 on failure.
     */
    int exitCode();

    /**
     * Set root's password, lasts one session.
     *
     * @param pass Root's password.
     * @param timeout The time that a password will live.
     * @return Zero on success, -1 on failure.
     */
    int setPass(const char *pass, int timeout);

    /**
     * Set the target host (optional).
     */
    int setHost(const QByteArray &host);

    /**
     * Set the desired priority (optional), see StubProcess.
     */
    int setPriority(int priority);

    /**
     * Set the desired scheduler (optional), see StubProcess.
     */
    int setScheduler(int scheduler);

    /**
     * Remove a password for a user/command.
     * @param command The command.
     * @param user The user.
     * @return zero on success, -1 on an error
     */
    int delCommand(const QByteArray &command, const QByteArray &user);

    /**
     * Set a persistent variable.
     * @param key The name of the variable.
     * @param value Its value.
     * @param timeout The timeout in seconds for this key. Zero means
     * no timeout.
     * @param group Make the key part of a group. See delGroup.
     * @return zero on success, -1 on failure.
     */
    int setVar(const QByteArray &key, const QByteArray &value, int timeout = 0, const QByteArray &group = nullptr);

    /**
     * Get a persistent variable.
     * @param key The name of the variable.
     * @return Its value.
     */
    QByteArray getVar(const QByteArray &key);

    /**
     * Gets all the keys that are membes of the given group.
     * @param group the group name of the variables.
     * @return a list of the keys in the group.
     */
    QList<QByteArray> getKeys(const QByteArray &group);

    /**
     * Returns true if the specified group exists is
     * cached.
     *
     * @param group the group key
     * @return true if the group is found
     */
    bool findGroup(const QByteArray &group);

    /**
     * Delete a persistent variable.
     * @param key The name of the variable.
     * @return zero on success, -1 on failure.
     */
    int delVar(const QByteArray &key);

    /**
     * Delete all persistent variables with the given key.
     *
     * A specicalized variant of delVar(QByteArray) that removes all
     * subsets of the cached variables given by @p key. In order for all
     * cached variables related to this key to be deleted properly, the
     * value given to the @p group argument when the setVar function
     * was called, must be a subset of the argument given here and the key
     *
     * @note Simply supplying the group key here WILL not necessarily
     * work. If you only have a group key, then use delGroup instead.
     *
     * @param special_key the name of the variable.
     * @return zero on success, -1 on failure.
     */
    int delVars(const QByteArray &special_key);

    /**
     * Delete all persistent variables in a group.
     *
     * @param group the group name. See setVar.
     * @return
     */
    int delGroup(const QByteArray &group);

    /**
     * Ping kdesud. This can be used for diagnostics.
     * @return Zero on success, -1 on failure
     */
    int ping();

    /**
     * Stop the daemon.
     */
    int stopServer();

    /**
     * Try to start up kdesud
     */
    int startServer();

private:
    KDESU_NO_EXPORT int connect();

    KDESU_NO_EXPORT int command(const QByteArray &cmd, QByteArray *result = nullptr);
    KDESU_NO_EXPORT QByteArray escape(const QByteArray &str);

private:
    std::unique_ptr<class ClientPrivate> const d;
};

} // END namespace KDESu

#endif // Q_OS_UNIX

#endif // KDESUCLIENT_H
