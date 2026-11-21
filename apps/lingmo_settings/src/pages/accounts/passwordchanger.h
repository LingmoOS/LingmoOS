// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <sys/types.h>

#include "base/timer.h"

namespace Lingmo::base {
class EventLoop;
}

namespace settings {

// Changes the user's own password through passwd(1) on a pseudo-terminal,
// answering its prompts. passwd asks PAM, so the system's password rules
// apply and the password modules, such as the login keyring's, follow the
// change. One change at a time.
class PasswordChanger
{
public:
    enum class Result {
        Changed,
        // The current password was not accepted.
        WrongPassword,
        // The new password was refused, by the password rules for instance.
        Rejected,
        Failed,
        // passwd stopped answering.
        TimedOut,
    };

    // `program` is passwd, or a stand-in that asks the same questions.
    // `timeout` is how long passwd may stay silent.
    explicit PasswordChanger(std::string program = "/usr/bin/passwd",
                             std::chrono::milliseconds timeout = std::chrono::seconds(20),
                             Lingmo::base::EventLoop* loop = nullptr);
    // Ends a passwd still running and waits for it.
    ~PasswordChanger();

    PasswordChanger(const PasswordChanger&) = delete;
    PasswordChanger& operator=(const PasswordChanger&) = delete;

    // False when passwd cannot be started; onFinished follows otherwise.
    bool start(std::string currentPassword, std::string newPassword);
    bool isRunning() const { return m_pid > 0; }

    // `message` is what passwd said, for Rejected and Failed; may be empty.
    // The object may be destroyed from it.
    std::function<void(Result result, const std::string& message)> onFinished;

private:
    enum class Prompt { Current, New, Retype };

    void drain();
    void readOutput();
    void handlePrompt(Prompt prompt);
    void answer(const std::string& text);
    void processEnded();
    // Ends passwd; its end then reports `result`.
    void stop(Result result, std::string message, int signal);
    void closeAll();

    Lingmo::base::EventLoop& m_loop;
    std::string m_program;
    std::chrono::milliseconds m_timeout;
    Lingmo::base::Timer m_silence;
    std::string m_current;
    std::string m_new;
    pid_t m_pid = -1;
    int m_pidfd = -1;
    int m_master = -1;
    int m_pidWatch = 0;
    int m_masterWatch = 0;
    // What passwd wrote since the last answer.
    std::string m_output;
    bool m_currentAnswered = false;
    bool m_newAnswered = false;
    bool m_stopping = false;
    Result m_stopResult = Result::Failed;
    std::string m_stopMessage;
};

} // namespace settings
