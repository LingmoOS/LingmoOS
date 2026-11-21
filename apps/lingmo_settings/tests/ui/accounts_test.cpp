// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <crypt.h>
#include <memory>
#include <string>

#include "lingmo/accounts/accounts_manager.h"
#include "pages/accounts/password.h"
#include "pages/accounts/userdetailsdialog.h"
#include "pages/accounts/userrow.h"
#include "pages/accounts/userdialogs.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"
#include "window/settingswindow.h"
#include "input_test_support.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace settings;

namespace {

template<typename T>
T* find(View* view)
{
    if (auto* found = dynamic_cast<T*>(view))
        return found;
    for (const auto& child : view->children()) {
        if (T* found = find<T>(child.get()))
            return found;
    }
    return nullptr;
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

View* findView(View* view, const std::string& name)
{
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
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

} // namespace

TEST(Accounts, PasswordsAreSha512WithAFreshSalt)
{
    const std::string first = cryptPassword("secret");
    const std::string second = cryptPassword("secret");
    ASSERT_TRUE(first.starts_with("$6$"));
    EXPECT_NE(first, second);
    crypt_data data = {};
    EXPECT_EQ(std::string(crypt_r("secret", first.c_str(), &data)), first);
}

TEST(Accounts, ChangePasswordWantsTheSamePasswordTwice)
{
    SettingsWindow window;
    ChangePasswordDialog dialog(window, "reion");
    std::string submitted;
    dialog.onPasswordSubmitted = [&](const std::string& password) { submitted = password; };
    dialog.show();
    Window& dialogWindow = *dialog.window();
    dialogWindow.grab();

    const auto fields = findAll<TextField>(dialogWindow.contentView());
    ASSERT_EQ(fields.size(), 2u);
    EXPECT_EQ(fields[0]->echoMode(), TextField::EchoMode::Password);
    const auto buttons = findAll<DialogButton>(dialogWindow.contentView());
    ASSERT_EQ(buttons.size(), 2u);
    DialogButton* change = buttons[1];
    EXPECT_FALSE(change->isEnabled());

    typeInto(dialogWindow, fields[0], "abc");
    typeInto(dialogWindow, fields[1], "abd");
    EXPECT_FALSE(change->isEnabled());
    TestInput::typeKey(dialogWindow, key::BackSpace);
    TestInput::typeText(dialogWindow, "c");
    EXPECT_TRUE(change->isEnabled());
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(change));
    EXPECT_EQ(submitted, "abc");
    EXPECT_FALSE(dialog.isOpen());
}

TEST(Accounts, AddUserShowsTheServicesAnswer)
{
    SettingsWindow window;
    accounts::AccountsManager manager(nullptr);
    AddUserDialog dialog(window, manager);
    dialog.show();
    Window& dialogWindow = *dialog.window();
    dialogWindow.grab();

    const auto fields = findAll<TextField>(dialogWindow.contentView());
    ASSERT_EQ(fields.size(), 3u);
    const auto buttons = findAll<DialogButton>(dialogWindow.contentView());
    ASSERT_EQ(buttons.size(), 2u);
    DialogButton* add = buttons[1];
    typeInto(dialogWindow, fields[0], "alice");
    typeInto(dialogWindow, fields[1], "pw");
    EXPECT_FALSE(add->isEnabled());
    typeInto(dialogWindow, fields[2], "pw");
    EXPECT_TRUE(add->isEnabled());
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(add));
    // Without the service the creation fails at once, and the dialog stays.
    EXPECT_TRUE(dialog.isOpen());
    bool shown = false;
    for (Label* label : findAll<Label>(dialogWindow.contentView()))
        shown = shown || (label->isVisible() && label->text() == "The accounts service cannot be reached");
    EXPECT_TRUE(shown);
}

TEST(Accounts, TheLoggedInAccountComesFirst)
{
    const auto make = [](int uid) {
        return std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User" + std::to_string(uid),
                                                       nullptr);
    };
    const std::vector<std::shared_ptr<accounts::UserAccount>> accounts {make(1001), make(1002), make(1000),
                                                                       make(1003)};
    std::vector<int64_t> uids;
    for (const auto& account : loggedInFirst(accounts, 1000))
        uids.push_back(account->userId());
    EXPECT_EQ(uids, (std::vector<int64_t> {1000, 1001, 1002, 1003}));
}

// One account's row in a list card, as the Users page shows it.
struct UserRowFixture
{
    UserRowFixture()
    {
        window.setScale(1.5);
        auto root = std::make_unique<Column>();
        root->setMargins({24, 56, 24, 24});
        Card* card = root->addChild<Card>(Card::Style::List);
        card->setLayoutHints({.fillWidth = true});
        row = card->addChild<UserRow>(account, 1000);
        window.setContentView(std::move(root));
        window.resize({670, 600});
        window.grab(1'000'000'000);
    }

    View* avatar() const { return find<Label>(row)->parent(); }

    std::shared_ptr<accounts::UserAccount> account =
            std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1000", nullptr);
    Window window;
    UserRow* row = nullptr;
};

TEST(Accounts, ARowIsThePictureAndNamesWithAnInfoButton)
{
    UserRowFixture fixture;
    // As much room above and below the picture as before it.
    const RectF row = fixture.row->geometry();
    const PointF rowOrigin = fixture.row->mapToWindow({0, 0});
    const PointF pictureOrigin = fixture.avatar()->mapToWindow({0, 0});
    EXPECT_EQ(row.height, 48 + 2 * 16);
    EXPECT_EQ(pictureOrigin.x - rowOrigin.x, 16);
    EXPECT_EQ(pictureOrigin.y - rowOrigin.y, 16);
    EXPECT_EQ(rowOrigin.y + row.height - (pictureOrigin.y + 48), 16);
    IconButton* info = find<IconButton>(fixture.row);
    ASSERT_TRUE(info);
    // At the end of the row, centred on it as the picture is.
    const PointF picture = centre(fixture.avatar());
    const PointF button = centre(info);
    EXPECT_DOUBLE_EQ(picture.y, button.y);
    EXPECT_GT(button.x, 670 - 24 - 16 - 22);

    int infoClicks = 0;
    fixture.row->onInfo = [&] { ++infoClicks; };
    TestInput::click(fixture.window, button);
    // The rest of the row opens the details too.
    TestInput::click(fixture.window, {button.x - 200, button.y});
    EXPECT_EQ(infoClicks, 2);
    // The picture is for changing the picture.
    int pictureClicks = 0;
    fixture.row->onPictureClicked = [&] { ++pictureClicks; };
    TestInput::click(fixture.window, picture);
    EXPECT_EQ(pictureClicks, 1);
    EXPECT_EQ(infoClicks, 2);
}

TEST(Accounts, TheDetailsShowTheTypeAndLeadToTheActions)
{
    SettingsWindow window;
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1001", nullptr);
    const auto open = [&](int64_t loggedIn) {
        auto details = std::make_unique<UserDetailsDialog>(window, account, loggedIn);
        details->show();
        details->dialog()->window()->grab();
        return details;
    };

    auto details = open(1000);
    View* content = details->dialog()->window()->contentView();
    bool typeShown = false;
    for (Label* label : findAll<Label>(content))
        typeShown = typeShown || (label->isVisible() && label->text() == "Standard");
    EXPECT_TRUE(typeShown);
    View* remove = findView(content, "user:delete");
    ASSERT_TRUE(remove);
    EXPECT_TRUE(remove->isEnabled());

    bool changing = false;
    details->onChangePassword = [&] { changing = true; };
    TestInput::click(*details->dialog()->window(), centre(findView(content, "user:changePassword")));
    EXPECT_TRUE(changing);

    // The account logged in cannot delete itself.
    auto own = open(1001);
    EXPECT_FALSE(findView(own->dialog()->window()->contentView(), "user:delete")->isEnabled());
}

TEST(Accounts, HoveringThePictureDimsItUnderAPencil)
{
    UserRowFixture fixture;
    Window& window = fixture.window;
    View* avatar = fixture.avatar();
    Label* initial = find<Label>(avatar);
    ASSERT_TRUE(initial);
    const PointF middle = centre(avatar);
    // Inside the circle, above both the initial and the pencil.
    const PointF probe {middle.x, middle.y - 18};
    const auto lightness = [&](const Image& image) {
        const Color pixel = image.pixel(int(probe.x * 1.5), int(probe.y * 1.5));
        return pixel.r + pixel.g + pixel.b;
    };
    const int plain = lightness(window.grab(1'000'000'000));

    TestInput::movePointer(window, middle);
    window.grab(1'050'000'000);
    const Image hovered = window.grab(1'400'000'000);
    EXPECT_LT(lightness(hovered), plain);
    // The initial makes way for the pencil.
    EXPECT_EQ(initial->opacity(), 0);

    TestInput::leavePointer(window);
    window.grab(1'450'000'000);
    EXPECT_EQ(lightness(window.grab(1'800'000'000)), plain);
    EXPECT_EQ(initial->opacity(), 1);
}

TEST(Accounts, ThePictureReplacesTheInitialOnceDecoded)
{
    UserRowFixture fixture;
    Window& window = fixture.window;
    const auto initialShown = [&] {
        for (Label* label : findAll<Label>(fixture.row)) {
            if (label->text() == "?")
                return label->isVisible();
        }
        return false;
    };
    EXPECT_TRUE(initialShown());

    fixture.row->showPicture(std::string(SETTINGS_SOURCE_DIR) + "/images/logo.svg");
    window.grab();
    ASSERT_TRUE(settings::testing::waitFor([&] {
        window.grab();
        return !initialShown();
    }));
}
