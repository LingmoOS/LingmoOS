// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "permissionmanager.h"
#include "permissionprompt.h"
#include "support/wait.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo;
using namespace Lingmo::base;
using namespace std::chrono_literals;

namespace {

struct Answer {
    bool error = false;
    std::string errorName;
    uint32_t response = 0;
    dbus::VariantMap results;
};

class PermissionPromptTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        static char name[] = "permission_prompt_test";
        static char *argv[] = {name, nullptr};
        s_application = new ui::Application(1, argv, ui::Application::Platform::Headless);
    }

    EventLoop &loop() { return s_application->loop(); }

    void SetUp() override
    {
        service = dbus::Connection::open(dbus::Connection::Bus::Session, &loop());
        manager = std::make_unique<PermissionManager>(service.get());
        ASSERT_TRUE(manager->publish());
        client = dbus::Connection::open(dbus::Connection::Bus::Session, &loop());
    }

    void TearDown() override
    {
        calls.clear();
        manager.reset();
        client.reset();
        service.reset();
    }

    // Prompt from the client; the answer arrives in `answer`.
    void prompt(dbus::Connection &from, const std::string &token, const std::string &kind,
                const dbus::VariantMap &options, std::optional<Answer> &answer)
    {
        dbus::Message call = from.newMethodCall(service->uniqueName(), "/org/lingmo/Services/Permissions",
                                                "org.lingmo.Services.Permissions", "Prompt");
        call.append(token, kind, std::string("lingmo-system-monitor"), std::string(), options);
        calls.push_back(from.callAsync(call, [&answer](const dbus::Reply &reply) {
            Answer result;
            result.error = reply.isError();
            result.errorName = reply.error().name;
            reply.read(result.response, result.results);
            answer = result;
        }));
    }

    void prompt(const std::string &token, const std::string &kind, const dbus::VariantMap &options,
                std::optional<Answer> &answer)
    {
        prompt(*client, token, kind, options, answer);
    }

    PermissionPrompt *shown(const std::string &token)
    {
        testsupport::waitFor(loop(), [&] { return manager->prompt(token) && manager->prompt(token)->window(); });
        return manager->prompt(token);
    }

    dbus::Reply dismiss(dbus::Connection &from, const std::string &token)
    {
        dbus::Message call = from.newMethodCall(service->uniqueName(), "/org/lingmo/Services/Permissions",
                                                "org.lingmo.Services.Permissions", "Dismiss");
        call.append(token);
        std::optional<dbus::Reply> result;
        calls.push_back(from.callAsync(call, [&result](const dbus::Reply &reply) { result = reply; }));
        testsupport::waitFor(loop(), [&] { return result.has_value(); });
        return result.value_or(dbus::Reply());
    }

    static dbus::VariantMap screenRecording(bool windows)
    {
        dbus::VariantMap monitor {{"name", dbus::Variant(std::string("eDP-1"))},
                                  {"description", dbus::Variant(std::string("Built-in Display"))},
                                  {"x", dbus::Variant(int32_t(0))},
                                  {"y", dbus::Variant(int32_t(0))},
                                  {"width", dbus::Variant(int32_t(1440))},
                                  {"height", dbus::Variant(int32_t(960))}};
        dbus::VariantMap options {{"types", dbus::Variant(uint32_t(windows ? 3 : 1))},
                                  {"monitors", dbus::Variant(std::vector<dbus::VariantMap> {monitor})}};
        if (windows) {
            dbus::VariantMap window {{"identifier", dbus::Variant(std::string("7"))},
                                     {"title", dbus::Variant(std::string("Files"))},
                                     {"app_id", dbus::Variant(std::string("lingmo-filemanager"))}};
            options["windows"] = dbus::Variant(std::vector<dbus::VariantMap> {window});
        }
        return options;
    }

    bool answered(const std::optional<Answer> &answer)
    {
        return testsupport::waitFor(loop(), [&] { return answer.has_value(); });
    }

    static ui::Application *s_application;
    std::unique_ptr<dbus::Connection> service;
    std::unique_ptr<dbus::Connection> client;
    std::unique_ptr<PermissionManager> manager;
    std::vector<dbus::PendingCall> calls;
};

ui::Application *PermissionPromptTest::s_application = nullptr;

std::vector<std::string> stringList(const dbus::VariantMap &results, const std::string &key)
{
    const auto it = results.find(key);
    return it == results.end() ? std::vector<std::string>() : it->second.value<std::vector<std::string>>();
}

} // namespace

TEST_F(PermissionPromptTest, RefusesUnknownKindsAndTokensInUse)
{
    std::optional<Answer> unknown;
    prompt("a", "camera", {}, unknown);
    ASSERT_TRUE(answered(unknown));
    EXPECT_TRUE(unknown->error);
    EXPECT_EQ(unknown->errorName, "org.freedesktop.DBus.Error.InvalidArgs");

    std::optional<Answer> empty;
    prompt("", "computer-control", {}, empty);
    ASSERT_TRUE(answered(empty));
    EXPECT_EQ(empty->errorName, "org.freedesktop.DBus.Error.InvalidArgs");

    std::optional<Answer> first;
    prompt("b", "computer-control", {}, first);
    ASSERT_TRUE(shown("b"));
    std::optional<Answer> second;
    prompt("b", "computer-control", {}, second);
    ASSERT_TRUE(answered(second));
    EXPECT_EQ(second->errorName, "org.freedesktop.DBus.Error.InvalidArgs");
    EXPECT_FALSE(first.has_value());
}

TEST_F(PermissionPromptTest, ReturnSharesTheFirstDisplay)
{
    std::optional<Answer> answer;
    prompt("t", "screen-recording", screenRecording(true), answer);
    PermissionPrompt *prompt = shown("t");
    ASSERT_TRUE(prompt);
    ui::TestInput::typeKey(*prompt->window(), ui::key::Return);
    ASSERT_TRUE(answered(answer));
    EXPECT_FALSE(answer->error);
    EXPECT_EQ(answer->response, 0u);
    EXPECT_EQ(stringList(answer->results, "monitors"), std::vector<std::string> {"eDP-1"});
    EXPECT_FALSE(answer->results.contains("windows"));
    EXPECT_FALSE(manager->prompt("t"));
}

TEST_F(PermissionPromptTest, ADoubleClickedWindowIsShared)
{
    std::optional<Answer> answer;
    prompt("t", "screen-recording", screenRecording(true), answer);
    PermissionPrompt *prompt = shown("t");
    ASSERT_TRUE(prompt);
    ui::Window &window = *prompt->window();
    // Laid out as shown.
    window.grab();
    const std::string dump = window.layoutDump();
    // The second row is the window's: "ChoiceRow logical ... device left top width height".
    double rowTop = -1;
    size_t rows = 0;
    for (size_t at = dump.find("ChoiceRow"); at != std::string::npos; at = dump.find("ChoiceRow", at + 1)) {
        if (++rows == 2) {
            const size_t device = dump.find("device ", at);
            const size_t top = dump.find(' ', device + 7);
            rowTop = std::stod(dump.substr(top + 1));
        }
    }
    ASSERT_GE(rowTop, 0) << dump;
    const ui::PointF row {200, rowTop + 26};
    ui::TestInput::click(window, row);
    ui::TestInput::press(window, row);
    ui::TestInput::release(window, row);
    ASSERT_TRUE(answered(answer));
    EXPECT_EQ(answer->response, 0u);
    EXPECT_EQ(stringList(answer->results, "windows"), std::vector<std::string> {"7"});
    EXPECT_FALSE(answer->results.contains("monitors"));
}

TEST_F(PermissionPromptTest, EscapeDenies)
{
    std::optional<Answer> answer;
    prompt("t", "computer-control", {{"agent_name", dbus::Variant(std::string("Claude"))}}, answer);
    PermissionPrompt *prompt = shown("t");
    ASSERT_TRUE(prompt);
    EXPECT_EQ(prompt->content()->title(), "Let “Claude” control this computer?");
    ui::TestInput::typeKey(*prompt->window(), ui::key::Escape);
    ASSERT_TRUE(answered(answer));
    EXPECT_EQ(answer->response, 1u);
    EXPECT_TRUE(answer->results.empty());
}

TEST_F(PermissionPromptTest, AllowingComputerControlChoosesNothing)
{
    std::optional<Answer> answer;
    prompt("t", "computer-control", {}, answer);
    PermissionPrompt *prompt = shown("t");
    ASSERT_TRUE(prompt);
    EXPECT_EQ(prompt->content()->title(), "Let “An app” control this computer?");
    prompt->allow();
    ASSERT_TRUE(answered(answer));
    EXPECT_EQ(answer->response, 0u);
    EXPECT_TRUE(answer->results.empty());
}

TEST_F(PermissionPromptTest, OnlyTheCallerDismisses)
{
    std::optional<Answer> answer;
    prompt("t", "computer-control", {}, answer);
    ASSERT_TRUE(shown("t"));

    auto other = dbus::Connection::open(dbus::Connection::Bus::Session, &loop());
    const dbus::Reply refused = dismiss(*other, "t");
    EXPECT_EQ(refused.error().name, "org.freedesktop.DBus.Error.AccessDenied");
    EXPECT_FALSE(answer.has_value());

    const dbus::Reply dismissed = dismiss(*client, "t");
    EXPECT_FALSE(dismissed.isError());
    ASSERT_TRUE(answered(answer));
    EXPECT_EQ(answer->response, 1u);
    EXPECT_FALSE(manager->prompt("t"));
}

TEST_F(PermissionPromptTest, ACallerLeavingTakesItsPromptWithIt)
{
    auto caller = dbus::Connection::open(dbus::Connection::Bus::Session, &loop());
    std::optional<Answer> answer;
    prompt(*caller, "t", "computer-control", {}, answer);
    ASSERT_TRUE(shown("t"));

    calls.clear();
    caller.reset();
    EXPECT_TRUE(testsupport::waitFor(loop(), [&] { return !manager->prompt("t"); }));
}

TEST_F(PermissionPromptTest, AManagerThatGoesFailsItsPrompts)
{
    std::optional<Answer> answer;
    prompt("t", "computer-control", {}, answer);
    ASSERT_TRUE(shown("t"));
    manager.reset();
    ASSERT_TRUE(answered(answer));
    EXPECT_EQ(answer->response, 2u);
}

TEST_F(PermissionPromptTest, EmptyPartsOfThePromptMoveIt)
{
    std::optional<Answer> answer;
    prompt("t", "computer-control", {}, answer);
    PermissionPrompt *prompt = shown("t");
    ASSERT_TRUE(prompt);
    ui::Window &window = *prompt->window();
    // Laid out as shown.
    window.grab();
    ui::TestInput::press(window, {5, 5});
    ui::TestInput::movePointer(window, {30, 30});
    EXPECT_EQ(ui::TestInput::moveRequests(window), 1);
    ui::TestInput::release(window, {30, 30});
}
