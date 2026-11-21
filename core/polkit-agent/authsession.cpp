// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "authsession.h"

#include <chrono>
#include <cstdio>
#include <sys/stat.h>

#include "base/event_loop.h"
#include "base/local_socket.h"
#include "base/logging.h"
#include "base/process.h"
#include "base/strings.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::strings;

static std::string s_helperPath = "/usr/lib/polkit-1/polkit-agent-helper-1";
static std::string s_helperSocketPath = "/run/polkit/agent-helper.socket";

static const LogCategory &log()
{
    static const LogCategory category("lingmo.polkit");
    return category;
}

// The helper escapes each message the way g_strescape() does: C escapes and
// octal for control and non-ASCII bytes.
static std::string unescape(std::string_view text)
{
    std::string bytes;
    bytes.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\' || i + 1 == text.size()) {
            bytes += text[i];
            continue;
        }

        const char c = text[++i];

        if (c >= '0' && c <= '7') {
            int value = 0;
            int digits = 0;

            while (digits < 3 && i < text.size() && text[i] >= '0' && text[i] <= '7') {
                value = value * 8 + (text[i] - '0');
                ++i;
                ++digits;
            }

            --i;
            bytes += char(value);
            continue;
        }

        switch (c) {
        case 'b': bytes += '\b'; break;
        case 'f': bytes += '\f'; break;
        case 'n': bytes += '\n'; break;
        case 'r': bytes += '\r'; break;
        case 't': bytes += '\t'; break;
        case 'v': bytes += '\v'; break;
        default: bytes += c; break;
        }
    }

    return sanitizedUtf8(bytes);
}

void AuthSession::setHelperForTesting(const std::string &helper, const std::string &socket)
{
    s_helperPath = helper;
    s_helperSocketPath = socket;
}

AuthSession::AuthSession() = default;

AuthSession::~AuthSession()
{
    m_finished = true;

    if (m_socket) {
        m_socket->onReadyRead = nullptr;
        m_socket->onDisconnected = nullptr;
        m_socket->onError = nullptr;
        m_socket->abort();
    }

    if (m_process) {
        m_process->onStandardOutput = nullptr;
        m_process->onStandardError = nullptr;
        m_process->onFinished = nullptr;
        // Kills a helper still running and waits for it.
        m_process.reset();
    }
}

void AuthSession::start(const std::string &user, const std::string &cookie)
{
    struct stat status {};
    if (::stat(s_helperSocketPath.c_str(), &status) != 0 || !connectSocket(user))
        startProcess(user);

    // The cookie goes through the channel so other processes cannot read it.
    write(cookie + '\n');
}

void AuthSession::respond(const std::string &response)
{
    write(replaceAll(response, "\n", " ") + '\n');
}

void AuthSession::write(const std::string &line)
{
    if (m_socket)
        m_socket->write(line);
    else if (m_process)
        m_process->writeStandardInput(line);
}

bool AuthSession::connectSocket(const std::string &user)
{
    m_socket = std::make_unique<LocalSocket>();

    if (!m_socket->connectToPath(s_helperSocketPath)) {
        LM_WARNING(log()) << "Cannot connect to" << s_helperSocketPath << m_socket->errorText()
                          << "- falling back to" << s_helperPath;
        m_socket.reset();
        return false;
    }

    m_socket->onReadyRead = [this] {
        readOutput(m_socket->readAll());
    };
    const auto ended = [this] {
        readOutput(m_socket->readAll());
        // The socket is closed now; the session ends on the next turn.
        EventLoop::current()->post([this, alive = std::weak_ptr<bool>(m_alive)] {
            if (alive.lock())
                finish(m_succeeded);
        });
    };
    m_socket->onDisconnected = ended;
    m_socket->onError = ended;

    // The service learns who is asking from the socket, and whom to
    // authenticate from this first line.
    m_socket->write(user + '\n');

    return true;
}

void AuthSession::startProcess(const std::string &user)
{
    m_process = std::make_unique<Process>();
    m_process->setStandardInputOpen(true);

    m_process->onStandardOutput = [this](std::string_view data) {
        readOutput(std::string(data));
    };
    // The helper's own messages go where this process's go.
    m_process->onStandardError = [](std::string_view data) {
        std::fwrite(data.data(), 1, data.size(), stderr);
    };
    m_process->onFinished = [this](int, bool) {
        EventLoop::current()->post([this, alive = std::weak_ptr<bool>(m_alive)] {
            if (alive.lock())
                finish(m_succeeded);
        });
    };

    if (!m_process->start(s_helperPath, {user})) {
        LM_WARNING(log()) << "Cannot start" << s_helperPath;
        EventLoop::current()->post([this, alive = std::weak_ptr<bool>(m_alive)] {
            if (alive.lock())
                finish(false);
        });
    }
}

void AuthSession::readOutput(const std::string &data)
{
    m_buffer += data;

    size_t newline;

    while ((newline = m_buffer.find('\n')) != std::string::npos) {
        const std::string line = m_buffer.substr(0, newline);
        m_buffer.erase(0, newline + 1);

        processLine(line);
        if (m_finished)
            return;
    }
}

void AuthSession::processLine(const std::string &line)
{
    static const std::string echoOff = "PAM_PROMPT_ECHO_OFF ";
    static const std::string echoOn = "PAM_PROMPT_ECHO_ON ";
    static const std::string errorMessage = "PAM_ERROR_MSG ";
    static const std::string textInfo = "PAM_TEXT_INFO ";

    if (startsWith(line, echoOff)) {
        if (onRequest)
            onRequest(unescape(std::string_view(line).substr(echoOff.size())), false);
    } else if (startsWith(line, echoOn)) {
        if (onRequest)
            onRequest(unescape(std::string_view(line).substr(echoOn.size())), true);
    } else if (startsWith(line, errorMessage)) {
        if (onNotice)
            onNotice(unescape(std::string_view(line).substr(errorMessage.size())), true);
    } else if (startsWith(line, textInfo)) {
        if (onNotice)
            onNotice(unescape(std::string_view(line).substr(textInfo.size())), false);
    } else if (startsWith(line, "SUCCESS")) {
        // Reported once the helper exits, a moment after this line.
        m_succeeded = true;
    } else if (startsWith(line, "FAILURE")) {
        m_succeeded = false;
    } else if (startsWith(line, "polkit-agent-helper-1: ")) {
        // The socket-activated helper may send its stderr down the socket too.
        LM_INFO(log()) << line;
    } else {
        LM_WARNING(log()) << "Unknown line from the polkit helper" << line;
    }
}

void AuthSession::finish(bool gainedAuthorization)
{
    if (m_finished)
        return;

    m_finished = true;

    if (onCompleted)
        onCompleted(gainedAuthorization);
}
