// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/userdetailsdialog.h"

#include "base/i18n.h"
#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kWidth = 400;

UserDetailsDialog::UserDetailsDialog(SettingsWindow& window, std::shared_ptr<accounts::UserAccount> account,
                                     int64_t loggedInUid)
    : m_account(std::move(account))
    , m_loggedInUid(loggedInUid)
    , m_dialog(std::make_unique<DialogWindow>(&window.window(), m_account->userName(), kWidth))
{
    auto content = std::make_unique<ColumnLayout>();
    content->setPadding({0, metrics::dialogPadding, 0, 0});
    content->setSpacing(metrics::dialogSpacing);

    Card* card = content->addChild<Card>(Card::Style::List);
    m_type = card->addChild<InfoRow>(tr("Account type"));
    m_password = card->addChild<ListRow>(tr("Change password…"));
    m_password->setName("user:changePassword");
    m_password->onClicked = [this] {
        close();
        if (onChangePassword)
            onChangePassword();
    };

    RowLayout* buttons = content->addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    m_delete = buttons->addChild<DialogButton>(tr("Delete this user"), true);
    m_delete->setName("user:delete");
    m_delete->setDestructive(true);
    m_delete->onClicked = [this] {
        close();
        if (onDelete)
            onDelete();
    };

    m_dialog->setContentView(std::move(content));
    m_dialog->onClosed = [this] {
        m_changed.reset();
        if (onClosed)
            onClosed();
    };
    m_dialog->window()->addShortcut(NoModifier, key::Escape, [this] { close(); });

    m_changed = m_account->changed.connect([this] { update(); });
    update();
}

UserDetailsDialog::~UserDetailsDialog() = default;

void UserDetailsDialog::show()
{
    m_dialog->show();
}

void UserDetailsDialog::close()
{
    m_dialog->close();
}

void UserDetailsDialog::update()
{
    m_dialog->setTitle(m_account->userName());
    m_type->setValue(m_account->accountType() == accounts::UserAccount::Standard ? tr("Standard")
                                                                                 : tr("Administrator"));
    m_delete->setEnabled(m_account->userId() != m_loggedInUid);
}

} // namespace settings
