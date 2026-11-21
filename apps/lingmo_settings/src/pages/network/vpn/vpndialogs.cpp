// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/vpn/vpndialogs.h"

#include <algorithm>

#include "base/i18n.h"
#include "pages/network/common/formscroll.h"
#include "pages/network/common/networklist.h"
#include "pages/network/vpn/vpneditor.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/formdialog.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

static constexpr double kWidth = 460;
// FishUI's dialog header, which the details dialog's content sits below.
static constexpr double kHeaderHeight = 44;

namespace {

// Red while it reports a failure, secondary text otherwise.
class MessageLabel : public Label
{
public:
    MessageLabel()
    {
        setWrap(true);
        setLayoutHints({.fillWidth = true});
    }

    void setMessage(const std::string& text, bool problem)
    {
        m_problem = problem;
        setText(text);
        setVisible(!text.empty());
        requestLayout();
    }

protected:
    void layout() override
    {
        setColor(m_problem ? accent::red : Theme::current().disabledText);
        Label::layout();
    }

private:
    bool m_problem = true;
};

} // namespace

AddVpnDialog::AddVpnDialog(SettingsWindow& window, nm::Handler& handler)
    : m_window(window)
    , m_handler(handler)
{
    m_subscriptions.push_back(m_handler.vpnAdded.connect([this](const std::string&) {
        if (!m_adding)
            return;
        m_adding = false;
        if (m_dialog)
            m_dialog->close();
    }));
    m_subscriptions.push_back(m_handler.vpnAddFailed.connect([this](const std::string& message) {
        if (!m_adding)
            return;
        m_adding = false;
        m_error = message;
        update();
    }));
}

AddVpnDialog::~AddVpnDialog() = default;

void AddVpnDialog::show()
{
    if (m_dialog && m_dialog->isOpen())
        return;
    m_adding = false;
    m_error.clear();
    m_dialog = std::make_unique<FormDialog>(m_window, tr("Add VPN"), kWidth);
    ColumnLayout* content = m_dialog->content();

    RowLayout* kindRow = content->addChild<RowLayout>();
    kindRow->setSpacing(metrics::dialogSpacing);
    kindRow->setLayoutHints({.fillWidth = true});
    m_kindLabel = kindRow->addChild<Label>(tr("Type"));
    m_kinds = m_handler.vpnKinds();
    std::vector<std::string> labels;
    for (const nm::vpn::Kind& kind : m_kinds)
        labels.push_back(kind.installed ? kind.label : format(tr("{type} (Not Installed)"), {{"type", kind.label}}));
    m_kindBox = kindRow->addChild<ComboBox>(labels);
    m_kindBox->setLayoutHints({.fillWidth = true});
    m_kindBox->setCurrentIndex(0);
    m_kindBox->onActivated = [this](int) { loadKind(); };

    auto editor = std::make_unique<VpnEditor>();
    m_editor = editor.get();
    m_editor->onAccepted = [this] { add(); };
    m_editor->onChanged = [this] { update(); };
    m_scroll = content->addChild<FormScroll>(std::move(editor), metrics::dialogPadding);
    // A long form scrolls instead of outgrowing the screen.
    m_scroll->maximumHeight = [this, kindRow] {
        const double width = kWidth - 2 * metrics::dialogPadding;
        double height = m_window.window().size().height - 2 * metrics::xxl - 2 * metrics::dialogPadding
                        - m_dialog->titleLabel()->heightForWidth(width - metrics::xxl) - kindRow->heightForWidth(width)
                        - m_addButton->sizeHint().height - 3 * metrics::dialogSpacing;
        if (m_errorLabel->isVisible())
            height -= m_errorLabel->heightForWidth(width) + metrics::dialogSpacing;
        return height;
    };

    m_errorLabel = content->addChild<MessageLabel>();
    RowLayout* buttons = m_dialog->addButtonRow();
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { m_dialog->close(); };
    m_addButton = buttons->addChild<DialogButton>(tr("Add"), true);
    m_addButton->onClicked = [this] { add(); };

    loadKind();
    m_dialog->show();
    if (m_editor->installed())
        m_editor->focusFirst();
}

void AddVpnDialog::loadKind()
{
    const int index = m_kindBox->currentIndex();
    if (index < 0 || size_t(index) >= m_kinds.size())
        return;
    m_editor->load(m_handler.vpnForm(m_kinds[size_t(index)].kind, {}));
    m_editor->measure(m_kindLabel->sizeHint().width);
    m_scroll->scrollToTop();
    m_error.clear();
    update();
    if (m_editor->installed() && m_dialog->isOpen())
        m_editor->focusFirst();
}

void AddVpnDialog::add()
{
    if (m_adding || !m_editor->complete())
        return;
    m_adding = true;
    m_error.clear();
    update();
    m_handler.saveVpn({}, m_editor->collect());
}

void AddVpnDialog::update()
{
    if (!m_dialog || !m_editor)
        return;
    LayoutHints hints = m_kindLabel->layoutHints();
    hints.preferredWidth = m_editor->labelWidth();
    m_kindLabel->setLayoutHints(hints);
    static_cast<MessageLabel*>(m_errorLabel)->setMessage(m_error, true);
    m_addButton->setEnabled(!m_adding && m_editor->complete());
    m_scroll->requestLayout();
}

VpnDetailsDialog::VpnDetailsDialog(SettingsWindow& window, NetworkList& list, nm::Handler& handler,
                                   std::string connectionPath)
    : m_window(window)
    , m_list(list)
    , m_handler(handler)
    , m_connectionPath(std::move(connectionPath))
    , m_dialog(std::make_unique<DialogWindow>(&window.window(), std::string(), kWidth))
{
    auto content = std::make_unique<ColumnLayout>();
    m_content = content.get();
    m_content->setPadding({0, metrics::dialogPadding, 0, 0});
    m_content->setSpacing(metrics::dialogSpacing);

    Card* formCard = m_content->addChild<Card>();
    formCard->setPadding(Margins::all(metrics::cardHorizontalPadding));
    auto editor = std::make_unique<VpnEditor>();
    m_editor = editor.get();
    m_editor->onAccepted = [this] { save(); };
    m_editor->onChanged = [this] { update(); };
    m_scroll = formCard->addChild<FormScroll>(std::move(editor), metrics::cardHorizontalPadding);

    m_statusCard = m_content->addChild<Card>(Card::Style::List);
    m_type = m_statusCard->addChild<InfoRow>(tr("Type"));
    m_status = m_statusCard->addChild<InfoRow>(tr("Status"));
    m_address = m_statusCard->addChild<InfoRow>(tr("IPv4 address"));
    m_export = m_statusCard->addChild<ListRow>(tr("Export to file…"));
    m_export->onClicked = [this] {
        m_exportChooser = std::make_unique<FileChooser>(m_dialog->window());
        m_exportChooser->setMode(FileChooser::Mode::SaveFile);
        m_exportChooser->setTitle(tr("Export VPN Configuration"));
        std::string patterns;
        for (const std::string& pattern : m_editor->form().exportPatterns)
            patterns += (patterns.empty() ? "" : " ") + pattern;
        m_exportChooser->setFilters(
                {FileChooser::Filter::fromNameFilter(format(tr("VPN configurations ({patterns})"),
                                                            {{"patterns", patterns}})),
                 FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
        m_exportChooser->onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
            if (paths.empty())
                return;
            m_error.clear();
            update();
            m_handler.exportVpn(m_connectionPath, paths.front());
        };
        m_exportChooser->open();
    };

    m_message = m_content->addChild<MessageLabel>();

    RowLayout* buttons = m_content->addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    DialogButton* remove = buttons->addChild<DialogButton>(tr("Remove VPN"));
    remove->setDestructive(true);
    remove->onClicked = [this] {
        m_handler.removeConnection(m_connectionPath);
        close();
    };
    buttons->addChild<View>()->setLayoutHints({.fillWidth = true});
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { close(); };
    m_saveButton = buttons->addChild<DialogButton>(tr("Save"), true);
    m_saveButton->onClicked = [this] { save(); };

    // A long form scrolls instead of outgrowing the main window.
    m_scroll->maximumHeight = [this, buttons] {
        const double width = kWidth - 2 * metrics::dialogPadding;
        double height = m_window.window().size().height - 2 * metrics::xxl - kHeaderHeight
                        - 2 * metrics::dialogPadding - m_statusCard->heightForWidth(width)
                        - buttons->sizeHint().height - 2 * metrics::dialogSpacing;
        if (m_message->isVisible())
            height -= m_message->heightForWidth(width) + metrics::dialogSpacing;
        return height;
    };

    m_dialog->setContentView(std::move(content));
    m_dialog->window()->addShortcut(NoModifier, key::Escape, [this] { close(); });

    m_subscriptions.push_back(m_handler.vpnSecretsLoaded.connect(
        [this](const std::string& connection, const nm::VpnSecrets& secrets) {
            if (connection == m_connectionPath)
                m_editor->setSecrets(secrets);
        }));
    m_subscriptions.push_back(m_handler.vpnSaved.connect([this](const std::string& connection) {
        if (connection != m_connectionPath || !m_saving)
            return;
        m_saving = false;
        close();
    }));
    m_subscriptions.push_back(m_handler.vpnSaveFailed.connect([this](const std::string& message) {
        if (!m_saving)
            return;
        m_saving = false;
        m_error = message;
        update();
    }));
    m_subscriptions.push_back(m_handler.vpnExportFailed.connect([this](const std::string& message) {
        m_error = message;
        update();
    }));
    m_subscriptions.push_back(m_list.changed.connect([this] { update(); }));

    m_editor->load(m_handler.vpnForm({}, m_connectionPath));
    m_handler.loadVpnSecrets(m_connectionPath);
    update();
}

VpnDetailsDialog::~VpnDetailsDialog() = default;

void VpnDetailsDialog::show()
{
    m_dialog->show();
}

void VpnDetailsDialog::save()
{
    if (m_saving || !m_editor->complete())
        return;
    m_saving = true;
    m_error.clear();
    update();
    m_handler.saveVpn(m_connectionPath, m_editor->collect());
}

void VpnDetailsDialog::close()
{
    if (!m_dialog->window())
        return;
    m_subscriptions.clear();
    m_dialog->close();
}

void VpnDetailsDialog::update()
{
    std::optional<nm::NetworkItem> row;
    for (const nm::NetworkItem& item : m_list.rows(nm::ConnectionType::Vpn)) {
        if (item.connectionPath == m_connectionPath)
            row = item;
    }
    if (!row) {
        if (m_found)
            close();
        return;
    }
    m_found = true;

    m_dialog->setTitle(row->name);
    m_type->setValue(row->vpnType);
    m_status->setValue(row->connectionState == nm::ConnectionStatus::Activated    ? tr("Connected")
                       : row->connectionState == nm::ConnectionStatus::Activating ? tr("Connecting…")
                                                                                   : tr("Not connected"));
    m_address->setValue(row->connectionState == nm::ConnectionStatus::Activated ? row->ipV4Address : std::string());
    m_export->setVisible(m_editor->form().exportable);
    static_cast<MessageLabel*>(m_message)
        ->setMessage(!m_error.empty() ? m_error : row->statusMessage, !m_error.empty());
    m_saveButton->setText(m_saving ? tr("Saving…") : tr("Save"));
    m_saveButton->setEnabled(!m_saving && m_editor->complete());
    m_scroll->requestLayout();
}

} // namespace settings::network
