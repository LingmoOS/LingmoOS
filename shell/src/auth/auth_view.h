// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <ctime>
#include <memory>
#include <optional>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Animation;
class ColumnLayout;
class Window;
} // namespace Lingmo::UI

namespace Lingmo::Auth {

class AuthController;
class ChoiceMenu;
class ErrorLabel;
class PasswordField;
class RaisedLabel;
class UserGrid;
class UserNameField;
class UserView;

// The lock screen and the greeter: the clock at the top and the login column
// at the bottom, as tall as the output. The compositor draws the wallpaper and
// the blur behind the password field; this is what goes over them.
// Upstream: LockContentsView.
class AuthScreen final : public ui::View
{
public:
    AuthScreen(ui::Window *window, AuthController *controller);
    ~AuthScreen() override;

    // Set while the session is blocked; the clock only ticks then, so a
    // prepared, unseen panel never wakes the compositor.
    void setBlocked(bool blocked);
    // Whether the panel has the keyboard: the password field looks focused
    // and its caret blinks only then.
    void setKeyboardFocused(bool focused);
    void focusPassword();

    // The password field in panel coordinates, for the blur behind it; it
    // moves with the shake.
    base::RectF fieldBounds() const;
    // How visible the password field is: it fades out while the accounts
    // are shown, and its blur with it.
    double fieldOpacity() const;
    // Seconds since the epoch the clock shows instead of the time, for tests.
    void setClockForTesting(std::time_t now);

protected:
    void layout() override;
    void childrenLaidOut() override;

private:
    void tick();
    void retranslate();
    void closeMenus(ui::PointF position);
    bool canSwitchUser() const;
    void updateSwappable();
    void setPickerOpen(bool open);
    void updatePicker();

    ui::Window *m_window;
    AuthController *m_controller;
    base::Timer m_clockTimer;
    std::vector<base::Subscription> m_subscriptions;
    std::optional<std::time_t> m_fixedClock;
    bool m_blocked = false;
    RaisedLabel *m_date = nullptr;
    RaisedLabel *m_time = nullptr;
    ui::ColumnLayout *m_pod = nullptr;
    UserView *m_user = nullptr;
    UserNameField *m_userName = nullptr;
    PasswordField *m_password = nullptr;
    ErrorLabel *m_error = nullptr;
    ChoiceMenu *m_sessions = nullptr;
    UserGrid *m_picker = nullptr;
    std::unique_ptr<ui::Animation> m_pickerFade;
    bool m_pickerOpen = false;
    double m_pickerProgress = 0;
    double m_pickerFrom = 0;
};

} // namespace Lingmo::Auth

