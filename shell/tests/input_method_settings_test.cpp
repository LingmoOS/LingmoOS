// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/input_method_settings.h"

#include <filesystem>
#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "base/paths.h"
#include "input_test_util.h"

using Lingmo::base::EventLoop;
using Lingmo::input::findInputMethodProviders;
using Lingmo::input::InputMethodSettings;
using Lingmo::input::parseDesktopExec;
using input_test::RemoteObject;
using input_test::TemporaryDirectory;
using input_test::writeFile;
using Strings = std::vector<std::string>;

namespace {
std::string provider(const std::string &exec, const std::string &extra = {})
{
    return "[Desktop Entry]\nType=Application\nName=IME\nName[de]=Eingabe\nExec=" + exec
            + "\nX-KDE-Wayland-VirtualKeyboard=true\n" + extra + "[Desktop Action x]\nExec=no\n";
}
} // namespace

TEST(InputMethodSettings, ParsesDesktopExec)
{
    EXPECT_EQ(Strings({"/usr/libexec/fcitx5-wayland-launcher", "--reopen"}),
              parseDesktopExec("/usr/libexec/fcitx5-wayland-launcher --reopen"));
    EXPECT_EQ(Strings({"/opt/My IME/ime", "--x", "a\"b", "100%"}),
              parseDesktopExec(R"("/opt/My IME/ime" --x %U "a\\"b" 100%%)"));
    EXPECT_EQ(Strings({"ime", "a b"}), parseDesktopExec(R"(ime "a\sb")"));
    EXPECT_TRUE(parseDesktopExec(R"("unterminated)").empty());
}

TEST(InputMethodSettings, FindsProvidersWithXdgPrecedence)
{
    TemporaryDirectory user, system;
    ASSERT_TRUE(user.isValid() && system.isValid());
    writeFile(system.filePath("fcitx.desktop"), provider("sh --flag %f"));
    writeFile(system.filePath("vendor/ime.desktop"), provider("/bin/sh"));
    writeFile(system.filePath("shadowed.desktop"), provider("sh"));
    writeFile(user.filePath("shadowed.desktop"), "[Desktop Entry]\nType=Application\nExec=sh\n");
    writeFile(system.filePath("hidden.desktop"), provider("sh", "Hidden=true\n"));
    writeFile(system.filePath("tryexec.desktop"), provider("sh", "TryExec=/no/such/ime\n"));
    writeFile(system.filePath("missing.desktop"), provider("/no/such/ime"));
    writeFile(system.filePath("plain.desktop"), "[Desktop Entry]\nType=Application\nExec=sh\n");

    const auto providers = findInputMethodProviders({user.path(), system.path()});
    ASSERT_EQ(2u, providers.size());
    EXPECT_EQ("fcitx.desktop", providers.at(0).id);
    EXPECT_EQ(Lingmo::base::paths::findExecutable("sh"), providers.at(0).command.front());
    EXPECT_EQ(Strings({"--flag"}), Strings(providers.at(0).command.begin() + 1,
                                           providers.at(0).command.end()));
    EXPECT_EQ("vendor-ime.desktop", providers.at(1).id);
    EXPECT_EQ(system.filePath("vendor/ime.desktop"), providers.at(1).path);
}

TEST(InputMethodSettings, PersistsValidSelectionsOnly)
{
    TemporaryDirectory directory, applications;
    ASSERT_TRUE(directory.isValid() && applications.isValid());
    writeFile(applications.filePath("ime.desktop"), provider("/bin/sh -c true"));
    const std::string file = directory.filePath("compositor.conf");
    {
        InputMethodSettings settings(file, {applications.path()});
        EXPECT_TRUE(settings.inputMethod().empty());
        EXPECT_TRUE(settings.command().empty());
        EXPECT_EQ(Strings({"ime.desktop"}), settings.availableInputMethods());
        int changed = 0;
        auto changedCounter = settings.inputMethodChanged.connect([&changed] { ++changed; });
        int command = 0;
        auto commandCounter = settings.commandChanged.connect([&command] { ++command; });
        EXPECT_FALSE(settings.setInputMethod("other.desktop"));
        EXPECT_TRUE(settings.setInputMethod("ime.desktop"));
        EXPECT_TRUE(settings.setInputMethod("ime.desktop"));
        EXPECT_EQ(1, changed);
        EXPECT_EQ(2, command);
        EXPECT_EQ(Strings({"/bin/sh", "-c", "true"}), settings.command());
    }
    InputMethodSettings restored(file, {applications.path()});
    EXPECT_EQ("ime.desktop", restored.inputMethod());
    std::filesystem::remove(applications.filePath("ime.desktop"));
    EXPECT_TRUE(restored.inputMethod().empty());
    EXPECT_TRUE(restored.setInputMethod({}));
}

TEST(InputMethodSettings, ExportsTheSessionService)
{
    EventLoop loop;
    TemporaryDirectory directory, applications;
    writeFile(applications.filePath("ime.desktop"), provider("/bin/sh"));
    InputMethodSettings settings(directory.filePath("compositor.conf"), {applications.path()});
    ASSERT_TRUE(settings.registerService());
    RemoteObject remote(loop, "org.lingmo.InputMethod1", "/org/lingmo/InputMethod1",
                        "org.lingmo.InputMethod1");
    ASSERT_TRUE(remote.isValid());
    EXPECT_EQ(Strings({"ime.desktop"}), remote.call("availableInputMethods").value<Strings>());
    EXPECT_EQ(false, remote.call("setInputMethod", std::string("nope.desktop")).value<bool>());
    EXPECT_EQ(true, remote.call("setInputMethod", std::string("ime.desktop")).value<bool>());
    EXPECT_EQ("ime.desktop", remote.property<std::string>("inputMethod"));
    settings.setRunning(true);
    EXPECT_EQ(true, remote.property<bool>("running"));
}
