// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>

#include "base/signal.h"
#include "lingmo/accounts/user_account.h"

namespace Lingmo::ui {
class DialogButton;
class DialogWindow;
}

namespace settings {

class InfoRow;
class ListRow;
class SettingsWindow;

// An account's details: its type, the way to a new password, and deleting
// it, which the account logged in cannot do to itself. Either action closes
// the details first.
class UserDetailsDialog
{
public:
    UserDetailsDialog(SettingsWindow& window, std::shared_ptr<Lingmo::Accounts::UserAccount> account,
                      int64_t loggedInUid);
    ~UserDetailsDialog();

    UserDetailsDialog(const UserDetailsDialog&) = delete;
    UserDetailsDialog& operator=(const UserDetailsDialog&) = delete;

    const std::shared_ptr<Lingmo::Accounts::UserAccount>& account() const { return m_account; }
    Lingmo::ui::DialogWindow* dialog() const { return m_dialog.get(); }

    void show();
    void close();

    std::function<void()> onChangePassword;
    std::function<void()> onDelete;
    std::function<void()> onClosed;

private:
    void update();

    std::shared_ptr<Lingmo::Accounts::UserAccount> m_account;
    int64_t m_loggedInUid;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_dialog;
    Lingmo::base::Subscription m_changed;
    InfoRow* m_type = nullptr;
    ListRow* m_password = nullptr;
    Lingmo::ui::DialogButton* m_delete = nullptr;
};

} // namespace settings
