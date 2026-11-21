// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::base {
class LocalSocket;
class Process;
}

// One PAM conversation through polkit's helper: the socket-activated service
// when polkit provides one, else the setuid binary. Deleting the session ends
// the conversation.
class AuthSession
{
public:
    AuthSession();
    ~AuthSession();

    AuthSession(const AuthSession &) = delete;
    AuthSession &operator=(const AuthSession &) = delete;

    void start(const std::string &user, const std::string &cookie);
    void respond(const std::string &response);

    std::function<void(const std::string &prompt, bool echo)> onRequest;
    std::function<void(const std::string &text, bool error)> onNotice;
    std::function<void(bool gainedAuthorization)> onCompleted;

    // For tests: the helper and its socket.
    static void setHelperForTesting(const std::string &helper, const std::string &socket);

private:
    bool connectSocket(const std::string &user);
    void startProcess(const std::string &user);
    void write(const std::string &line);
    void readOutput(const std::string &data);
    void processLine(const std::string &line);
    void finish(bool gainedAuthorization);

    std::unique_ptr<Lingmo::Base::LocalSocket> m_socket;
    std::unique_ptr<Lingmo::Base::Process> m_process;
    std::string m_buffer;
    bool m_succeeded = false;
    bool m_finished = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

#endif // AUTHSESSION_H
