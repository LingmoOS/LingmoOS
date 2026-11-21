// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>

#include "pages/accounts/passwordchanger.h"
#include "pages/accounts/userdialogs.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "window/settingswindow.h"
#include "input_test_support.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

// Asks as passwd does through PAM on a terminal: the current password is
// "old", a new one of fewer than four characters is refused and asked again.
// What it is finally given goes to `changed` next to it.
const char* const kPasswd = R"(#!/bin/sh
[ -t 0 ] && stty -echo || exit 3
[ "$LC_ALL" = C ] || exit 4
printf 'Changing password for test.\n'
printf 'Current password: '
read -r current
printf '\n'
if [ "$current" != old ]; then
    printf 'passwd: Authentication token manipulation error\npasswd: password unchanged\n'
    exit 10
fi
while :; do
    printf 'New password: '
    read -r new
    printf '\n'
    if [ ${#new} -lt 4 ]; then
        printf 'You must choose a longer password.\n'
        continue
    fi
    printf 'Retype new password: '
    read -r again
    printf '\n'
    if [ "$new" != "$again" ]; then
        printf 'Sorry, passwords do not match.\n'
        continue
    fi
    printf '%s' "$new" > "$(dirname "$0")/changed"
    printf 'passwd: password updated successfully\n'
    exit 0
done
)";

// A passwd that asks for the current password and then dies.
const char* const kDyingPasswd = R"(#!/bin/sh
printf 'Current password: '
read -r current
kill -KILL $$
)";

// A passwd that asks for the current password and then hangs.
const char* const kHangingPasswd = R"(#!/bin/sh
printf 'Current password: '
read -r current
exec sleep 60
)";

class FakePasswd
{
public:
    explicit FakePasswd(const char* script)
    {
        char pattern[] = "/tmp/settings-passwd-XXXXXX";
        m_directory = mkdtemp(pattern);
        path = m_directory + "/passwd";
        std::ofstream(path) << script;
        chmod(path.c_str(), 0755);
    }
    ~FakePasswd() { std::filesystem::remove_all(m_directory); }

    std::string changed() const
    {
        std::ifstream file(m_directory + "/changed");
        return std::string(std::istreambuf_iterator<char>(file), {});
    }

    std::string path;

private:
    std::string m_directory;
};

struct Outcome
{
    PasswordChanger::Result result;
    std::string message;
};

std::optional<Outcome> change(const std::string& program, const std::string& current, const std::string& password,
                              std::chrono::milliseconds timeout = 5s)
{
    PasswordChanger changer(program, timeout);
    std::optional<Outcome> outcome;
    changer.onFinished = [&](PasswordChanger::Result result, const std::string& message) {
        outcome = Outcome{result, message};
    };
    if (!changer.start(current, password))
        return std::nullopt;
    EXPECT_TRUE(changer.isRunning());
    settings::testing::waitFor([&] { return outcome.has_value(); }, 5000ms);
    EXPECT_FALSE(changer.isRunning());
    return outcome;
}

template<typename T>
std::vector<T*> findAll(View* view)
{
    std::vector<T*> found;
    if (auto* match = dynamic_cast<T*>(view))
        found.push_back(match);
    for (const auto& child : view->children()) {
        for (T* match : findAll<T>(child.get()))
            found.push_back(match);
    }
    return found;
}

PointF centre(View* view)
{
    const PointF origin = view->mapToWindow({0, 0});
    return {origin.x + view->geometry().width / 2, origin.y + view->geometry().height / 2};
}

void typeInto(Window& window, TextField* field, const std::string& text)
{
    TestInput::click(window, centre(field));
    TestInput::typeText(window, text);
}

bool showsText(Window& window, const std::string& text)
{
    for (Label* label : findAll<Label>(window.contentView())) {
        if (label->isVisible() && label->text() == text)
            return true;
    }
    return false;
}

} // namespace

TEST(AccountsPassword, ChangesThroughPasswdsPrompts)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "old", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Changed);
    EXPECT_EQ(passwd.changed(), "new password");
}

TEST(AccountsPassword, ReportsAWrongCurrentPassword)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "wrong", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::WrongPassword);
    EXPECT_EQ(passwd.changed(), "");
}

TEST(AccountsPassword, ReportsWhyTheNewPasswordWasRefused)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "old", "abc");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Rejected);
    EXPECT_EQ(outcome->message, "You must choose a longer password.");
    EXPECT_EQ(passwd.changed(), "");
}

TEST(AccountsPassword, ReportsPasswdDying)
{
    FakePasswd passwd(kDyingPasswd);
    const auto outcome = change(passwd.path, "old", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Failed);
}

TEST(AccountsPassword, GivesUpOnASilentPasswd)
{
    FakePasswd passwd(kHangingPasswd);
    const auto outcome = change(passwd.path, "old", "new password", 300ms);
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::TimedOut);
}

TEST(AccountsPassword, AMissingPasswdDoesNotStart)
{
    PasswordChanger changer("/nonexistent/passwd");
    EXPECT_FALSE(changer.start("old", "new password"));
    EXPECT_FALSE(changer.isRunning());
}

TEST(AccountsPassword, TheOwnAccountsDialogAsksTheCurrentPassword)
{
    FakePasswd passwd(kPasswd);
    SettingsWindow window;
    ChangePasswordDialog dialog(window, "reion", std::make_unique<PasswordChanger>(passwd.path));
    dialog.show();
    Window& dialogWindow = *dialog.window();
    dialogWindow.grab();

    const auto fields = findAll<TextField>(dialogWindow.contentView());
    ASSERT_EQ(fields.size(), 3u);
    for (TextField* field : fields)
        EXPECT_EQ(field->echoMode(), TextField::EchoMode::Password);
    const auto buttons = findAll<DialogButton>(dialogWindow.contentView());
    ASSERT_EQ(buttons.size(), 2u);
    DialogButton* change = buttons[1];

    typeInto(dialogWindow, fields[0], "wrong");
    typeInto(dialogWindow, fields[1], "new password");
    typeInto(dialogWindow, fields[2], "new password");
    EXPECT_TRUE(change->isEnabled());
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(change));
    EXPECT_FALSE(change->isEnabled());
    ASSERT_TRUE(settings::testing::waitFor([&] { return change->isEnabled(); }, 5000ms));
    EXPECT_TRUE(dialog.isOpen());
    EXPECT_TRUE(showsText(dialogWindow, "The current password is incorrect"));
    EXPECT_EQ(fields[0]->text(), "");

    typeInto(dialogWindow, fields[0], "old");
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(change));
    ASSERT_TRUE(settings::testing::waitFor([&] { return !dialog.isOpen(); }, 5000ms));
    EXPECT_EQ(passwd.changed(), "new password");
}
