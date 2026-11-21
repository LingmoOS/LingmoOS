// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/archivepassworddialog.h"
#include "model/foldermodel.h"

#include "base/i18n.h"
#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace fm {

class ArchivePasswordDialog::Content : public ui::ColumnLayout
{
public:
    explicit Content(ArchivePasswordDialog* dialog)
        : m_dialog(dialog)
    {
        setSpacing(12);
        // The dialog pads the sides and the bottom; the top takes 16 more.
        setPadding({0, 16, 0, 0});

        auto* message = addChild<ui::Label>(dialog->m_retry
                                                    ? format(tr("The password for “{name}” is incorrect. Try again."),
                                                             {{"name", dialog->m_name}})
                                                    : format(tr("Enter the password to extract “{name}”."),
                                                             {{"name", dialog->m_name}}));
        message->setWrap(true);
        message->setColor(ui::ColorId::DisabledText);
        message->setLayoutHints({.fillWidth = true});

        password = addChild<ui::TextField>();
        password->setPlaceholder(tr("Password"));
        password->setEchoMode(ui::TextField::EchoMode::Password);
        password->setLayoutHints({.fillWidth = true});
        password->onAccepted = [this] { accept(); };
        password->onTextChanged = [this] { extract->setEnabled(!password->text().empty()); };

        auto* actions = addChild<ui::RowLayout>();
        actions->setSpacing(8);
        ui::LayoutHints actionHints {.horizontalAlignment = ui::Alignment::End};
        actionHints.margins.top = 4;
        actions->setLayoutHints(actionHints);
        auto* cancel = actions->addChild<ui::DialogButton>(tr("Cancel"));
        cancel->onClicked = [this] { m_dialog->m_dialog->close(); };
        extract = actions->addChild<ui::DialogButton>(tr("Extract"), true);
        extract->onClicked = [this] { accept(); };
        extract->setEnabled(false);
    }

    void accept()
    {
        if (password->text().empty())
            return;
        m_dialog->m_answered = true;
        m_dialog->m_model.answerArchivePassword(m_dialog->m_request, password->text());
        m_dialog->m_dialog->close();
    }

    ui::TextField* password;
    ui::DialogButton* extract;

private:
    ArchivePasswordDialog* m_dialog;
};

ArchivePasswordDialog::ArchivePasswordDialog(ui::Window* parent, FolderModel& model, uint32_t request,
                                             std::string name, bool retry)
    : m_model(model)
    , m_request(request)
    , m_name(std::move(name))
    , m_retry(retry)
{
    m_dialog = std::make_unique<ui::DialogWindow>(parent, tr("Password Required"), 400);
    m_content = m_dialog->setContent<Content>(this);
    m_dialog->onClosed = [this] {
        // Closed without an answer: the extraction is given up.
        if (!m_answered)
            m_model.cancelArchive();
        if (onDone)
            onDone();
    };
    m_dialog->window()->setBackgroundColor(ui::ColorId::SecondBackground);
    m_dialog->window()->addShortcut(ui::NoModifier, ui::key::Escape, [this] { m_dialog->close(); });
    m_content->password->setFocus();
}

ArchivePasswordDialog::~ArchivePasswordDialog() = default;

void ArchivePasswordDialog::show()
{
    m_dialog->show();
}

void ArchivePasswordDialog::dismiss()
{
    m_answered = true;
    m_dialog->close();
}

void ArchivePasswordDialog::retranslate()
{
    // Closed: it goes once the event is over.
    if (!m_dialog->window())
        return;
    const std::string password = m_content->password->text();
    m_dialog->setTitle(tr("Password Required"));
    m_content = m_dialog->setContent<Content>(this);
    m_content->password->setText(password);
    m_content->password->setFocus();
}

} // namespace fm
