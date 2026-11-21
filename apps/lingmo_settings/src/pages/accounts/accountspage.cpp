// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <unistd.h>

#include "lingmo/accounts/accounts_manager.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "pages/accounts/password.h"
#include "pages/accounts/passwordchanger.h"
#include "pages/accounts/userdetailsdialog.h"
#include "pages/accounts/userrow.h"
#include "pages/accounts/userdialogs.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class AccountsPage : public Page
{
public:
    explicit AccountsPage(SettingsWindow& window)
        : Page(window, tr("Users"))
    {
        m_users = content()->addChild<Card>(Card::Style::List);
        m_users->setLayoutHints({.fillWidth = true});
        StandardButton* add = content()->addChild<StandardButton>(tr("Add user"));
        add->onClicked = [this] {
            m_addUser = std::make_unique<AddUserDialog>(settingsWindow(), m_manager);
            m_addUser->onClosed = [this] { dispose(m_addUser); };
            m_addUser->show();
        };

        m_accountsChanged = m_manager.accountsChanged.connect([this] { updateUsers(); });
        updateUsers();
    }

private:
    // Lets a dialog finish the event that closed it before it goes.
    template<typename T>
    static void dispose(std::unique_ptr<T>& dialog)
    {
        std::shared_ptr<T> closing(dialog.release());
        base::EventLoop::current()->post([closing] {});
    }

    void updateUsers()
    {
        // Rows that are still there are kept; the one logged in goes first.
        std::vector<std::unique_ptr<View>> previous;
        while (!m_users->children().empty())
            previous.push_back(m_users->takeChild(m_users->children().front().get()));
        const auto accounts = loggedInFirst(m_manager.accounts(), int64_t(getuid()));
        for (const auto& account : accounts) {
            std::unique_ptr<View> row;
            for (auto& view : previous) {
                if (view && static_cast<UserRow*>(view.get())->account() == account) {
                    row = std::move(view);
                    break;
                }
            }
            if (!row)
                row = makeRow(account);
            m_users->addChild(std::move(row));
        }
        m_users->setVisible(!accounts.empty());
        // The details of an account that is gone close with it.
        if (m_details && std::find(accounts.begin(), accounts.end(), m_details->account()) == accounts.end())
            m_details->close();
    }

    std::unique_ptr<View> makeRow(const accounts::AccountsManager::AccountPtr& account)
    {
        auto row = std::make_unique<UserRow>(account, int64_t(getuid()));
        row->onPictureClicked = [this, account] {
            m_chooser = std::make_unique<FileChooser>(&settingsWindow().window());
            m_chooser->setMode(FileChooser::Mode::OpenFile);
            // As QML's FileDialog, window-modal.
            m_chooser->setModal(true);
            m_chooser->setFilters(
                    {FileChooser::Filter::fromNameFilter(
                             format(tr("Image files ({patterns})"),
                                    {{"patterns", "*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.avif"}})),
                     FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
            m_chooser->setCurrentFolder(base::paths::userDirectory(base::paths::UserDirectory::Pictures));
            m_chooser->onAccepted = [this, account](const std::vector<std::string>& paths, const std::string&) {
                if (paths.empty())
                    return;
                account->setIconFileName(paths.front());
                // Shown from the file picked while the service copies it.
                for (const auto& child : m_users->children()) {
                    auto* row = static_cast<UserRow*>(child.get());
                    if (row->account() == account)
                        row->showPicture(paths.front());
                }
            };
            m_chooser->open();
        };
        row->onInfo = [this, account] {
            m_details = std::make_unique<UserDetailsDialog>(settingsWindow(), account, int64_t(getuid()));
            m_details->onChangePassword = [this, account] { changePassword(account); };
            m_details->onDelete = [this, account] { deleteUser(account); };
            m_details->onClosed = [this] { dispose(m_details); };
            m_details->show();
        };
        return row;
    }

    void changePassword(const accounts::AccountsManager::AccountPtr& account)
    {
        // The user's own goes through PAM, which asks for the current
        // password and updates the login keyring with it.
        const bool own = account->userId() == int64_t(getuid());
        m_changePassword = std::make_unique<ChangePasswordDialog>(
                settingsWindow(), account->userName(), own ? std::make_unique<PasswordChanger>() : nullptr);
        m_changePassword->onPasswordSubmitted = [account](const std::string& password) {
            account->setPassword(cryptPassword(password));
        };
        m_changePassword->onClosed = [this] { dispose(m_changePassword); };
        m_changePassword->show();
    }

    void deleteUser(const accounts::AccountsManager::AccountPtr& account)
    {
        const int64_t uid = account->userId();
        if (uid == int64_t(getuid()))
            return;
        m_confirm = std::make_unique<ConfirmDialog>(
                &settingsWindow().window(), tr("Delete this user"),
                format(tr("Delete user “{name}”? Their home directory and its contents will be permanently deleted. "
                          "This cannot be undone."),
                       {{"name", account->userName()}}));
        m_confirm->setCancelText(tr("Cancel"));
        m_confirm->setConfirmText(tr("Delete"));
        m_confirm->setDestructive(true);
        m_confirm->onAccepted = [this, uid] { m_manager.deleteUser(uid, true); };
        m_confirm->show();
    }

    accounts::AccountsManager m_manager;
    base::Subscription m_accountsChanged;
    Card* m_users;
    std::unique_ptr<UserDetailsDialog> m_details;
    std::unique_ptr<AddUserDialog> m_addUser;
    std::unique_ptr<ChangePasswordDialog> m_changePassword;
    std::unique_ptr<ConfirmDialog> m_confirm;
    std::unique_ptr<FileChooser> m_chooser;
};

} // namespace

static const PageRegistration registration("accounts", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AccountsPage>(window);
});

} // namespace settings
