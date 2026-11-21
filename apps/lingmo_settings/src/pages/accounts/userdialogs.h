// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "pages/accounts/passwordchanger.h"
#include "widgets/formdialog.h"

namespace Lingmo::Accounts {
class AccountsManager;
}

namespace Lingmo::ui {
class ComboBox;
class DialogButton;
class GridLayout;
class Label;
class TextField;
}

namespace settings {

// A new password for an account, typed twice. With a changer it is the
// user's own: the current password is asked too, and the dialog stays open
// until the change is made, saying why it was not.
class ChangePasswordDialog : public FormDialog
{
public:
    ChangePasswordDialog(SettingsWindow& parent, const std::string& userName,
                         std::unique_ptr<PasswordChanger> changer = nullptr);
    ~ChangePasswordDialog() override;

    // Without a changer: the password as typed; the caller hashes it.
    std::function<void(const std::string& password)> onPasswordSubmitted;

private:
    void update();
    void change();
    void showError(const std::string& error);

    std::unique_ptr<PasswordChanger> m_changer;
    Lingmo::ui::GridLayout* m_form;
    Lingmo::ui::TextField* m_current = nullptr;
    Lingmo::ui::TextField* m_password;
    Lingmo::ui::TextField* m_verify;
    Lingmo::ui::Label* m_error = nullptr;
    Lingmo::ui::DialogButton* m_cancel;
    Lingmo::ui::DialogButton* m_change;
};

// A new account: its name, password and type. Stays open, and refuses to
// close, until the service has answered.
class AddUserDialog : public FormDialog
{
public:
    AddUserDialog(SettingsWindow& parent, Lingmo::Accounts::AccountsManager& manager);

private:
    void update();
    void add();
    void setWaiting(bool waiting);

    Lingmo::Accounts::AccountsManager& m_manager;
    Lingmo::ui::GridLayout* m_form;
    Lingmo::ui::TextField* m_name;
    Lingmo::ui::TextField* m_password;
    Lingmo::ui::TextField* m_verify;
    Lingmo::ui::ComboBox* m_type;
    Lingmo::ui::Label* m_error;
    Lingmo::ui::DialogButton* m_cancel;
    Lingmo::ui::DialogButton* m_add;
    bool m_waiting = false;
};

} // namespace settings
