// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>

#include "base/file.h"
#include "base/json.h"
#include "base/process.h"
#include "common/environment.h"
#include "testutil.h"
#include "xwaylandenvironment.h"

using Lingmo::Base::EventLoop;
using Lingmo::Base::file::TemporaryDirectory;
using SessionTest::runUntil;

namespace json = Lingmo::Base::json;

static void publish(const QString &path, const QString &display, const QString &authority)
{
    json::Value document = json::Value::object();
    document.insert("DISPLAY", display.toStdString());
    document.insert("XAUTHORITY", authority.toStdString());
    ASSERT_TRUE(Lingmo::Base::file::writeAtomically(path.toStdString(),
                                                     document.serialize(false)));
}

static bool isEmptyVariable(const char *name)
{
    const char *value = std::getenv(name);
    return !value || !*value;
}

static QString variable(const char *name)
{
    const char *value = std::getenv(name);
    return value ? QString::fromLocal8Bit(value) : QString();
}

static bool createAuthority(const QString &path, mode_t mode)
{
    if (!SessionTest::writeFile(path, QString()))
        return false;
    return ::chmod(path.toLocal8Bit().constData(), mode) == 0;
}

struct Resolved
{
    std::vector<bool> results;
    int lost = 0;
};

static void track(XwaylandEnvironment &environment, Resolved &resolved)
{
    environment.onResolved = [&resolved](bool available) { resolved.results.push_back(available); };
    environment.onLost = [&resolved] { ++resolved.lost; };
}

TEST(XwaylandEnvironment, arrivalOrdersAndLoss)
{
    for (bool early : {false, true}) {
        EventLoop loop;
        TemporaryDirectory directory(SessionTest::temporaryParent().toStdString());
        const QString authority = QString::fromStdString(directory.path()) + QStringLiteral("/auth");
        ASSERT_TRUE(createAuthority(authority, 0600));
        XwaylandEnvironment environment;
        Resolved resolved;
        track(environment, resolved);
        setenv("DISPLAY", ":old", 1);
        setenv("XAUTHORITY", "/old", 1);
        if (early)
            publish(QString::fromStdString(environment.readinessFile()), QStringLiteral(":123"), authority);
        environment.waitForReady(1000);
        if (!early) {
            EXPECT_TRUE(isEmptyVariable("DISPLAY"));
            publish(QString::fromStdString(environment.readinessFile()), QStringLiteral(":123"), authority);
        }
        ASSERT_TRUE(runUntil(loop, [&] { return resolved.results.size() == 1; }, 1500));
        EXPECT_TRUE(resolved.results.front());
        EXPECT_EQ(variable("DISPLAY"), QStringLiteral(":123"));
        EXPECT_EQ(variable("XAUTHORITY"), authority);
        ASSERT_TRUE(std::filesystem::remove(environment.readinessFile()));
        ASSERT_TRUE(runUntil(loop, [&] { return resolved.lost == 1; }, 1500));
        EXPECT_TRUE(isEmptyVariable("DISPLAY"));
        EXPECT_TRUE(isEmptyVariable("XAUTHORITY"));
    }
}

TEST(XwaylandEnvironment, malformedAndTimeout)
{
    EventLoop loop;
    XwaylandEnvironment environment;
    Resolved resolved;
    track(environment, resolved);
    publish(QString::fromStdString(environment.readinessFile()),
            QStringLiteral("remote:0"), QStringLiteral("/missing"));
    environment.waitForReady(30);
    ASSERT_TRUE(runUntil(loop, [&] { return resolved.results.size() == 1; }, 1000));
    EXPECT_FALSE(resolved.results.front());
    EXPECT_TRUE(isEmptyVariable("DISPLAY"));
}

static QString testCompositor()
{
    return variable("LINGMO_TEST_COMPOSITOR");
}

TEST(XwaylandEnvironment, realCompositor)
{
    const QString binary = testCompositor();
    if (binary.isEmpty())
        GTEST_SKIP() << "Set LINGMO_TEST_COMPOSITOR for real Xwayland integration";
    EventLoop loop;
    TemporaryDirectory runtime(SessionTest::temporaryParent().toStdString());
    XwaylandEnvironment environment;
    Resolved resolved;
    track(environment, resolved);
    Lingmo::Base::Process compositor(&loop);
    Lingmo::Core::Environment childEnvironment = Lingmo::Core::processEnvironment();
    childEnvironment["XDG_RUNTIME_DIR"] = runtime.path();
    compositor.setEnvironment(Lingmo::Core::environmentEntries(childEnvironment));
    ASSERT_TRUE(compositor.start(binary, {QStringLiteral("--no-drm"),
                                          QStringLiteral("--socket"), QStringLiteral("session-test"),
                                          QStringLiteral("--xwayland-shm"),
                                          QStringLiteral("--xwayland-ready-file"),
                                          QString::fromStdString(environment.readinessFile())}));
    environment.waitForReady(10000);
    ASSERT_TRUE(runUntil(loop, [&] { return resolved.results.size() == 1; }, 12000));
    EXPECT_TRUE(resolved.results.front()) << compositor.standardError();
    Lingmo::Base::Process client(&loop);
    int clientCode = -1;
    client.onFinished = [&clientCode](int code, bool) { clientCode = code; };
    ASSERT_TRUE(client.start(QStringLiteral("xdpyinfo")));
    EXPECT_TRUE(client.waitForFinished(std::chrono::seconds(3)));
    EXPECT_EQ(clientCode, 0);
    int compositorCode = -1;
    compositor.onFinished = [&compositorCode](int code, bool) { compositorCode = code; };
    compositor.terminate();
    EXPECT_TRUE(compositor.waitForFinished(std::chrono::seconds(5)));
    EXPECT_EQ(compositorCode, 0);
}

TEST(XwaylandEnvironment, disabledXwaylandKeepsNativeCompositor)
{
    const QString binary = testCompositor();
    if (binary.isEmpty())
        GTEST_SKIP() << "Set LINGMO_TEST_COMPOSITOR for real integration";
    EventLoop loop;
    TemporaryDirectory runtime(SessionTest::temporaryParent().toStdString());
    XwaylandEnvironment environment;
    Resolved resolved;
    track(environment, resolved);
    Lingmo::Base::Process compositor(&loop);
    Lingmo::Core::Environment childEnvironment = Lingmo::Core::processEnvironment();
    childEnvironment["XDG_RUNTIME_DIR"] = runtime.path();
    compositor.setEnvironment(Lingmo::Core::environmentEntries(childEnvironment));
    ASSERT_TRUE(compositor.start(binary, {QStringLiteral("--no-drm"),
                                          QStringLiteral("--socket"), QStringLiteral("session-test"),
                                          QStringLiteral("--no-xwayland"),
                                          QStringLiteral("--xwayland-ready-file"),
                                          QString::fromStdString(environment.readinessFile())}));
    ASSERT_TRUE(runUntil(loop, [&] {
        return std::filesystem::exists(runtime.path() + "/session-test");
    }, 3000));
    environment.waitForReady(30);
    ASSERT_TRUE(runUntil(loop, [&] { return resolved.results.size() == 1; }, 1000));
    EXPECT_FALSE(resolved.results.front());
    EXPECT_TRUE(compositor.isRunning());
    int compositorCode = -1;
    compositor.onFinished = [&compositorCode](int code, bool) { compositorCode = code; };
    compositor.terminate();
    EXPECT_TRUE(compositor.waitForFinished(std::chrono::seconds(5)));
    EXPECT_EQ(compositorCode, 0);
}

TEST(XwaylandEnvironment, unsafeAuthority)
{
    EventLoop loop;
    TemporaryDirectory directory(SessionTest::temporaryParent().toStdString());
    const QString authority = QString::fromStdString(directory.path()) + QStringLiteral("/auth");
    ASSERT_TRUE(createAuthority(authority, 0604));
    XwaylandEnvironment environment;
    Resolved resolved;
    track(environment, resolved);
    publish(QString::fromStdString(environment.readinessFile()),
            QStringLiteral(":123"), authority);
    environment.waitForReady(30);
    ASSERT_TRUE(runUntil(loop, [&] { return resolved.results.size() == 1; }, 1000));
    EXPECT_FALSE(resolved.results.front());
}
