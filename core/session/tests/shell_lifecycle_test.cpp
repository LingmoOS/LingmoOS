// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>

#include "base/process.h"
#include "common/environment.h"
#include "shellconnection.h"
#include "shellprocess.h"
#include "testutil.h"

using Lingmo::Base::EventLoop;
using SessionTest::runUntil;

static QString selfPath()
{
    char path[4096];
    const ssize_t length = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    return length > 0 ? QString::fromLocal8Bit(path, int(length)) : QString();
}

static QString controlSocketPath()
{
    const char *runtime = std::getenv("XDG_RUNTIME_DIR");
    return QString::fromLocal8Bit(runtime && *runtime ? runtime : "/tmp")
           + QStringLiteral("/lingmo-session-lifecycle-test.control");
}

static bool hasVariable(const char *name)
{
    return std::getenv(name) != nullptr;
}

static QString variable(const char *name)
{
    const char *value = std::getenv(name);
    return value ? QString::fromLocal8Bit(value) : QString();
}

namespace {

// A stand-in for the compositor's control channel: accepts one connection and
// collects the descriptors sent over it.
class FakeCompositor
{
public:
    bool listen(const QString &path)
    {
        m_path = path;
        ::unlink(path.toLocal8Bit().constData());

        sockaddr_un address {};
        address.sun_family = AF_UNIX;
        const QByteArray pathBytes = path.toLocal8Bit();
        std::memcpy(address.sun_path, pathBytes.constData(), size_t(pathBytes.size()));

        m_listenFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (m_listenFd < 0)
            return false;
        if (bind(m_listenFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0)
            return false;
        return ::listen(m_listenFd, 4) == 0;
    }

    bool accept()
    {
        m_clientFd = accept4(m_listenFd, nullptr, nullptr, SOCK_CLOEXEC);
        return m_clientFd >= 0;
    }

    // Blocks for the next descriptor, or -1.
    int receiveFd()
    {
        char payload = 0;
        iovec iov {};
        iov.iov_base = &payload;
        iov.iov_len = 1;

        union {
            char buffer[CMSG_SPACE(sizeof(int))];
            cmsghdr alignment;
        } control {};

        msghdr message {};
        message.msg_iov = &iov;
        message.msg_iovlen = 1;
        message.msg_control = control.buffer;
        message.msg_controllen = sizeof(control.buffer);

        if (recvmsg(m_clientFd, &message, MSG_CMSG_CLOEXEC) <= 0)
            return -1;

        for (cmsghdr *header = CMSG_FIRSTHDR(&message); header;
             header = CMSG_NXTHDR(&message, header)) {
            if (header->cmsg_level == SOL_SOCKET && header->cmsg_type == SCM_RIGHTS) {
                int fd = -1;
                std::memcpy(&fd, CMSG_DATA(header), sizeof(fd));
                return fd;
            }
        }
        return -1;
    }

    ~FakeCompositor()
    {
        if (m_clientFd >= 0)
            ::close(m_clientFd);
        if (m_listenFd >= 0)
            ::close(m_listenFd);
        if (!m_path.isEmpty())
            ::unlink(m_path.toLocal8Bit().constData());
    }

private:
    QString m_path;
    int m_listenFd = -1;
    int m_clientFd = -1;
};

struct Finished
{
    int count = 0;
    int code = -1;
    bool crashed = false;
};

} // namespace

TEST(ShellLifecycle, environmentAndSingleStart)
{
    EventLoop loop;
    setenv("SHELL_TEST_CHILD", "1", 1);
    ShellProcess process;
    Finished finished;
    process.onFinished = [&finished](int code, bool crashed) {
        ++finished.count;
        finished.code = code;
        finished.crashed = crashed;
    };
    EXPECT_TRUE(process.startForSession(selfPath(), QStringLiteral("shell-test-socket"), nullptr));
    EXPECT_FALSE(process.startForSession(
        QStringLiteral("/does-not-exist/duplicate-shell"),
        QStringLiteral("wrong-socket"), nullptr));
    ASSERT_TRUE(runUntil(loop, [&] { return finished.count == 1; }));
    EXPECT_FALSE(finished.crashed);
    EXPECT_EQ(finished.code, 0);
}

TEST(ShellLifecycle, stopAndMissingExecutable)
{
    EventLoop loop;
    setenv("SHELL_TEST_CHILD", "hang", 1);
    ShellProcess process;
    EXPECT_TRUE(process.startForSession(selfPath(), QStringLiteral("shell-test-socket"), nullptr));
    EXPECT_TRUE(process.isRunning());
    process.stop();
    EXPECT_FALSE(process.isRunning());
    process.stop();

    EXPECT_FALSE(process.startForSession(
        QStringLiteral("/does-not-exist/lingmo-filemanager"),
        QStringLiteral("shell-test-socket"), nullptr));
    EXPECT_FALSE(process.isRunning());
}

// The socketpair the session creates really reaches both sides: the
// compositor gets one end over SCM_RIGHTS, the shell the other.
TEST(ShellLifecycle, theCompositorReceivesTheShellConnection)
{
    FakeCompositor compositor;
    const QString path = controlSocketPath();
    ASSERT_TRUE(compositor.listen(path));

    ShellConnection connection;
    ASSERT_TRUE(connection.connectToCompositor(path, 2000));
    ASSERT_TRUE(compositor.accept());

    const int shellEnd = connection.createShellConnection();
    ASSERT_GE(shellEnd, 0);

    const int compositorEnd = compositor.receiveFd();
    ASSERT_GE(compositorEnd, 0);

    EXPECT_EQ(::write(shellEnd, "x", 1), ssize_t(1));
    char received = 0;
    EXPECT_EQ(::read(compositorEnd, &received, 1), ssize_t(1));
    EXPECT_EQ(received, 'x');

    ::close(shellEnd);
    ::close(compositorEnd);
}

// The daemon is started with a usable --desktop-socket descriptor and the
// public WAYLAND_DISPLAY, and neither the descriptor nor WAYLAND_SOCKET
// reaches anything it launches.
TEST(ShellLifecycle, thePrivilegedConnectionDoesNotLeakToChildren)
{
    EventLoop loop;
    FakeCompositor compositor;
    const QString path = controlSocketPath();
    ASSERT_TRUE(compositor.listen(path));

    ShellConnection connection;
    ASSERT_TRUE(connection.connectToCompositor(path, 2000));
    ASSERT_TRUE(compositor.accept());

    setenv("SHELL_TEST_CHILD", "privileged", 1);
    ShellProcess process;
    Finished finished;
    process.onFinished = [&finished](int code, bool crashed) {
        ++finished.count;
        finished.code = code;
        finished.crashed = crashed;
    };
    ASSERT_TRUE(process.startForSession(selfPath(), QStringLiteral("shell-test-socket"), &connection));
    EXPECT_GE(compositor.receiveFd(), 0);
    ASSERT_TRUE(runUntil(loop, [&] { return finished.count == 1; }, 15000));
    EXPECT_FALSE(finished.crashed);
    EXPECT_EQ(finished.code, 0);
}

// Runs as the file manager daemon: verifies its own privileged command line,
// then starts a grandchild the way the application runtime starts an
// application and verifies that nothing privileged came along.
static int runPrivilegedChild(int argc, char **argv)
{
    if (variable("WAYLAND_DISPLAY") != QLatin1String("shell-test-socket"))
        return 3;
    if (hasVariable("WAYLAND_SOCKET"))
        return 4;

    const QStringList arguments = QStringList(
        QList<QString>(argv + 1, argv + argc));
    if (arguments.size() != 3
        || arguments.at(0) != QLatin1String("--daemon")
        || arguments.at(1) != QLatin1String("--desktop-socket"))
        return 5;
    char *end = nullptr;
    const long fd = std::strtol(arguments.at(2).toLocal8Bit().constData(), &end, 10);
    if (*end || fd < 0)
        return 5;
    if (fcntl(int(fd), F_GETFD) < 0)
        return 6;
    if (fcntl(int(fd), F_SETFD, FD_CLOEXEC) < 0)
        return 7;

    EventLoop loop;
    Lingmo::Base::Process grandchild(&loop);
    Lingmo::Core::Environment environment = Lingmo::Core::processEnvironment();
    environment.erase("WAYLAND_SOCKET");
    environment["SHELL_TEST_CHILD"] = "grandchild";
    environment["SHELL_TEST_PRIVILEGED_FD"] = std::to_string(fd);
    grandchild.setEnvironment(Lingmo::Core::environmentEntries(environment));
    int exitCode = 8;
    grandchild.onFinished = [&exitCode](int code, bool) { exitCode = code; };
    if (!grandchild.start(selfPath()) || !grandchild.waitForFinished(std::chrono::seconds(10)))
        return 8;

    return exitCode;
}

// Runs as an application the shell launched.
static int runGrandchild()
{
    if (hasVariable("WAYLAND_SOCKET"))
        return 20;
    if (variable("WAYLAND_DISPLAY") != QLatin1String("shell-test-socket"))
        return 21;

    const int fd = std::atoi(variable("SHELL_TEST_PRIVILEGED_FD").toLocal8Bit().constData());
    if (fd > 0 && fcntl(fd, F_GETFD) >= 0)
        return 22;

    return 0;
}

int main(int argc, char **argv)
{
    const QString mode = variable("SHELL_TEST_CHILD");

    if (mode == QLatin1String("grandchild"))
        return runGrandchild();

    if (mode == QLatin1String("privileged"))
        return runPrivilegedChild(argc, argv);

    if (!mode.isEmpty()) {
        if (variable("WAYLAND_DISPLAY") != QLatin1String("shell-test-socket")
            || variable("DISPLAY") != QLatin1String(":99")
            || variable("XAUTHORITY") != QLatin1String("/test-authority")
            || hasVariable("WAYLAND_SOCKET")
            || argc != 2
            || QLatin1String(argv[1]) != QLatin1String("--daemon"))
            return 2;
        if (mode != QLatin1String("hang")) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return 0;
        }
        for (;;)
            pause();
    }

    setenv("DISPLAY", ":99", 1);
    setenv("XAUTHORITY", "/test-authority", 1);
    setenv("WAYLAND_SOCKET", "999", 1);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
