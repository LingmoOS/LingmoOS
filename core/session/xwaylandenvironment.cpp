// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xwaylandenvironment.h"

#include <cstdlib>
#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

#include "base/json.h"
#include "base/paths.h"
#include "logs.h"

namespace Lingmo {

static bool isLocalDisplay(const QString &display)
{
    if (display.size() < 2 || display.front() != QLatin1Char(':'))
        return false;
    for (int i = 1; i < display.size(); ++i) {
        if (display.at(i) < QLatin1Char('0') || display.at(i) > QLatin1Char('9'))
            return false;
    }
    return true;
}

XwaylandEnvironment::XwaylandEnvironment()
    : m_directory(Lingmo::Base::paths::runtimeDirectory(),
                  QStringLiteral("lingmo-session-x11-").toStdString())
{
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        m_waiting = false;
        LM_WARNING(sessionLog()) << "Xwayland readiness timed out; continuing with native Wayland";
        if (onResolved)
            onResolved(false);
    };
    if (m_directory.isValid())
        m_watch = Lingmo::Base::FileWatch::directory(m_directory.path(), [this](const std::string &) {
            check();
        });
}

QString XwaylandEnvironment::readinessFile() const
{
    return m_directory.isValid()
        ? QString::fromStdString(m_directory.path() + "/ready.json")
        : QString();
}

void XwaylandEnvironment::waitForReady(int timeoutMs)
{
    unsetenv("DISPLAY");
    unsetenv("XAUTHORITY");
    m_waiting = true;
    m_timeout.start(std::chrono::milliseconds(timeoutMs));
    check();
}

void XwaylandEnvironment::check()
{
    const QString path = readinessFile();
    const QByteArray pathBytes = path.toLocal8Bit();
    if (path.isEmpty() || ::access(pathBytes.constData(), F_OK) != 0) {
        if (m_available) {
            m_available = false;
            unsetenv("DISPLAY");
            unsetenv("XAUTHORITY");
            if (onLost)
                onLost();
        }
        return;
    }
    if (!m_waiting)
        return;

    const int fd = ::open(pathBytes.constData(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return;

    QByteArray bytes(4097, '\0');
    size_t length = 0;
    while (length < size_t(bytes.size())) {
        const ssize_t n = ::read(fd, bytes.data() + length, bytes.size() - length);
        if (n <= 0)
            break;
        length += size_t(n);
    }
    ::close(fd);
    if (length > 4096)
        return;
    bytes.resize(length);

    const auto document = Lingmo::Base::json::Value::parse(
        std::string_view(bytes.constData(), size_t(bytes.size())));
    if (!document || !document->isObject())
        return;

    const QString display = QString::fromStdString((*document)["DISPLAY"].toString());
    const QString authority = QString::fromStdString((*document)["XAUTHORITY"].toString());

    struct stat st {};
    const QByteArray authBytes = authority.toLocal8Bit();
    if (!isLocalDisplay(display)
        || !authority.startsWith(QLatin1Char('/'))
        || authority.contains(QLatin1Char('\0'))
        || lstat(authBytes.constData(), &st) != 0
        || !S_ISREG(st.st_mode)
        || st.st_uid != getuid()
        || (st.st_mode & 0777) != 0600)
        return;

    setenv("DISPLAY", display.toLocal8Bit().constData(), 1);
    setenv("XAUTHORITY", authBytes.constData(), 1);
    m_waiting = false;
    m_available = true;
    m_timeout.stop();
    if (onResolved)
        onResolved(true);
}

} // namespace Lingmo
