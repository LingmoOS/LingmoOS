// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/userdialogs.h"

#include "base/i18n.h"
#include "lingmo/accounts/accounts_manager.h"
#include "pages/accounts/password.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kWidth = 400;

static GridLayout* addForm(ColumnLayout* content)
{
    GridLayout* grid = content->addChild<GridLayout>(2);
    grid->setColumnSpacing(metrics::dialogSpacing);
    grid->setRowSpacing(metrics::formRowSpacing);
    grid->setLayoutHints({.fillWidth = true});
    return grid;
}

static TextField* addField(GridLayout* grid, const std::string& label, bool password)
{
    grid->addChild<Label>(label);
    TextField* field = grid->addChild<TextField>();
    field->setPlaceholder(label);
    if (password)
        field->setEchoMode(TextField::EchoMode::Password);
    field->setLayoutHints({.fillWidth = true});
    return field;
}

// Secondary text that wraps, recoloured with the theme.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

ChangePasswordDialog::ChangePasswordDialog(SettingsWindow& parent, const std::string& userName,
                                           std::unique_ptr<PasswordChanger> changer)
    : FormDialog(parent, tr("Change password"), kWidth)
    , m_changer(std::move(changer))
{
    Label* name = content()->addChild<SecondaryLabel>(userName);
    name->setWrap(true);
    name->setLayoutHints({.fillWidth = true});

    m_form = addForm(content());
    if (m_changer) {
        m_current = addField(m_form, tr("Current password"), true);
        m_password = addField(m_form, tr("New password"), true);
    } else {
        m_password = addField(m_form, tr("Password"), true);
    }
    m_verify = addField(m_form, tr("Verify password"), true);

    if (m_changer) {
        m_error = content()->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});
        m_error->setVisible(false);
        m_changer->onFinished = [this](PasswordChanger::Result result, const std::string& message) {
            switch (result) {
            case PasswordChanger::Result::Changed:
                close();
                return;
            case PasswordChanger::Result::WrongPassword:
                m_current->setText({});
                m_current->setFocus();
                showError(tr("The current password is incorrect"));
                break;
            case PasswordChanger::Result::Rejected:
                m_password->setText({});
                m_verify->setText({});
                m_password->setFocus();
                showError(message.empty() ? tr("The new password was not accepted") : message);
                break;
            case PasswordChanger::Result::Failed:
                showError(message.empty() ? tr("The password could not be changed") : message);
                break;
            case PasswordChanger::Result::TimedOut:
                showError(tr("The password change did not finish in time"));
                break;
            }
            update();
        };
    }

    RowLayout* buttons = addButtonRow();
    m_cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    m_cancel->onClicked = [this] { requestClose(); };
    m_change = buttons->addChild<DialogButton>(tr("Change password"), true);
    m_change->onClicked = [this] { change(); };

    for (TextField* field : {m_current, m_password, m_verify}) {
        if (field)
            field->onTextChanged = [this] { update(); };
    }
    canClose = [this] { return !m_changer || !m_changer->isRunning(); };
    onShown = [this] { (m_current ? m_current : m_password)->setFocus(); };
    update();
}

ChangePasswordDialog::~ChangePasswordDialog() = default;

void ChangePasswordDialog::update()
{
    const bool waiting = m_changer && m_changer->isRunning();
    m_form->setEnabled(!waiting);
    m_cancel->setEnabled(!waiting);
    m_change->setEnabled(!waiting && !m_password->text().empty() && m_password->text() == m_verify->text());
}

void ChangePasswordDialog::change()
{
    if (!m_changer) {
        const std::string password = m_password->text();
        if (onPasswordSubmitted)
            onPasswordSubmitted(password);
        close();
        return;
    }

    showError({});
    if (!m_changer->start(m_current->text(), m_password->text()))
        showError(tr("The password could not be changed"));
    update();
}

void ChangePasswordDialog::showError(const std::string& error)
{
    m_error->setText(error);
    m_error->setVisible(!error.empty());
}

AddUserDialog::AddUserDialog(SettingsWindow& parent, accounts::AccountsManager& manager)
    : FormDialog(parent, tr("Add new user"), kWidth)
    , m_manager(manager)
{
    m_form = addForm(content());
    m_name = addField(m_form, tr("User name"), false);
    m_password = addField(m_form, tr("Password"), true);
    m_verify = addField(m_form, tr("Verify password"), true);
    m_form->addChild<Label>(tr("Account type"));
    m_type = m_form->addChild<ComboBox>(std::vector<std::string>{tr("Standard"), tr("Administrator")});
    m_type->setCurrentIndex(0);
    m_type->setLayoutHints({.fillWidth = true});

    m_error = content()->addChild<Label>();
    m_error->setWrap(true);
    m_error->setLayoutHints({.fillWidth = true});
    m_error->setVisible(false);

    RowLayout* buttons = addButtonRow();
    m_cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    m_cancel->onClicked = [this] { requestClose(); };
    m_add = buttons->addChild<DialogButton>(tr("Add"), true);
    m_add->onClicked = [this] { add(); };

    for (TextField* field : {m_name, m_password, m_verify})
        field->onTextChanged = [this] { update(); };
    canClose = [this] { return !m_waiting; };
    onShown = [this] { m_name->setFocus(); };
    update();
}

void AddUserDialog::update()
{
    m_form->setEnabled(!m_waiting);
    m_cancel->setEnabled(!m_waiting);
    m_add->setEnabled(!m_waiting && !m_name->text().empty() && !m_password->text().empty()
                      && m_password->text() == m_verify->text());
}

void AddUserDialog::setWaiting(bool waiting)
{
    m_waiting = waiting;
    update();
}

void AddUserDialog::add()
{
    m_error->setVisible(false);
    m_error->setText({});
    const std::string password = m_password->text();
    setWaiting(true);
    m_manager.createUser(m_name->text(), {},
                         m_type->currentIndex() == 1 ? accounts::UserAccount::Administrator
                                                     : accounts::UserAccount::Standard,
                         [this, password](const accounts::AccountsManager::AccountPtr& account,
                                          const std::string& error) {
                             setWaiting(false);
                             if (!account) {
                                 m_error->setText(error);
                                 m_error->setVisible(!error.empty());
                                 return;
                             }
                             account->setPasswordMode(accounts::UserAccount::RegularPassword);
                             account->setPassword(cryptPassword(password));
                             close();
                         });
}

} // namespace settings
