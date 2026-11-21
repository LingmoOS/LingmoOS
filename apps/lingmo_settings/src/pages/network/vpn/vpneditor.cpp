// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/vpn/vpneditor.h"

#include <algorithm>
#include <climits>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "lingmo/networkmanager/types.h"
#include "pages/network/common/textchecks.h"
#include "pages/network/common/rows.h"
#include "ui/button.h"
#include "ui/combo_box.h"
#include "ui/file_chooser.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;
namespace schema = Lingmo::Networkmanager::vpn::schema;

namespace settings::network {

// A longer label wraps rather than squeezing the fields.
static constexpr double kMaximumLabelWidth = 140;

namespace {

// Text in the accent colour that acts when clicked.
class LinkLabel : public View
{
public:
    explicit LinkLabel(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
    }

    void setText(std::string text) { m_label->setText(std::move(text)); }

    SizeF sizeHint() const override { return m_label->sizeHint(); }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().highlight);
        m_label->setGeometry(localRect());
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    Label* m_label;
};

} // namespace

static Label* addFieldLabel(RowLayout* row, const std::string& text, std::vector<Label*>& labels)
{
    Label* label = row->addChild<Label>(text);
    label->setWrap(true);
    labels.push_back(label);
    return label;
}

static RowLayout* addFormRow(ColumnLayout* parent)
{
    RowLayout* row = parent->addChild<RowLayout>();
    row->setSpacing(metrics::dialogSpacing);
    row->setLayoutHints({.fillWidth = true});
    return row;
}

// The fields of a plugin VPN: the basic ones, then the advanced ones by
// section behind a toggle.
class PluginForm : public ColumnLayout
{
public:
    std::function<void()> onAccepted;
    std::function<void()> onChanged;

    PluginForm()
    {
        setSpacing(metrics::formRowSpacing);
        setLayoutHints({.fillWidth = true});
    }

    void load(const std::vector<schema::FormField>& fields, bool shared)
    {
        m_fields = fields;
        m_shared = shared;
        m_values.clear();
        m_storage.clear();
        m_secrets.clear();
        for (const schema::FormField& field : fields) {
            m_values[field.key] = field.type == schema::Type::Bool ? (field.checked ? "true" : "false") : field.value;
            if (field.type == schema::Type::Password)
                m_storage[field.key] = field.storage;
        }
        m_advancedShown = false;
        m_advancedMeasured = false;
        build();
    }

    void setShared(bool shared)
    {
        if (shared == m_shared)
            return;
        m_shared = shared;
        for (FieldRow& row : m_rows) {
            if (row.storage)
                loadStorage(row);
        }
        update();
    }

    void setSecrets(const std::map<std::string, std::string>& secrets)
    {
        m_secrets.clear();
        for (const schema::FormField& field : m_fields) {
            const auto found = secrets.find(field.key);
            if (field.type == schema::Type::Password && found != secrets.end())
                m_secrets[field.key] = found->second;
        }
        reload();
    }

    const schema::Values& values() const { return m_values; }
    const std::map<std::string, uint32_t>& storage() const { return m_storage; }
    const std::map<std::string, std::string>& secrets() const { return m_secrets; }

    bool complete() const
    {
        for (const schema::FormField& field : m_fields) {
            const auto value = m_values.find(field.key);
            const std::string text = value == m_values.end() ? std::string() : value->second;
            if (field.required && shown(field) && text.find_first_not_of(" \t\n") == std::string::npos)
                return false;
        }
        return true;
    }

    // The labels that share the editor's label column, as far as they have appeared.
    std::vector<Label*> labels() const
    {
        std::vector<Label*> labels;
        for (const FieldRow& row : m_rows) {
            if (row.label && (!row.field->advanced || m_advancedMeasured))
                labels.push_back(row.label);
        }
        return labels;
    }

    // Field labels other than switches' get the column's width.
    void setLabelWidth(double width)
    {
        for (FieldRow& row : m_rows) {
            if (!row.label || row.field->type == schema::Type::Bool)
                continue;
            LayoutHints hints = row.label->layoutHints();
            if (hints.preferredWidth != width) {
                hints.preferredWidth = width;
                row.label->setLayoutHints(hints);
            }
        }
    }

private:
    struct FieldRow
    {
        const schema::FormField* field = nullptr;
        RowLayout* row = nullptr;
        Label* label = nullptr;
        TextField* text = nullptr;
        ComboBox* box = nullptr;
        ComboBox* storage = nullptr;
        Switch* toggle = nullptr;
        std::vector<uint32_t> storageValues;
    };

    bool shown(const schema::Field& field) const
    {
        std::vector<schema::Field> plain(m_fields.begin(), m_fields.end());
        return schema::shown(field, plain, m_values);
    }

    void build()
    {
        while (!children().empty())
            takeChild(children().front().get());
        m_rows.clear();
        m_sectionTitles.clear();
        m_rows.reserve(m_fields.size());

        bool hasAdvanced = false;
        for (const schema::FormField& field : m_fields) {
            if (!field.advanced)
                addRow(this, field);
            hasAdvanced |= field.advanced;
        }

        m_link = addChild<LinkLabel>(tr("Show advanced options"));
        m_link->setVisible(hasAdvanced);
        m_link->onClicked = [this] {
            m_advancedShown = !m_advancedShown;
            m_advancedMeasured = true;
            update();
        };

        m_advanced = addChild<ColumnLayout>();
        m_advanced->setSpacing(metrics::formRowSpacing);
        m_advanced->setLayoutHints({.fillWidth = true});
        std::string section;
        ColumnLayout* sectionColumn = nullptr;
        for (const schema::FormField& field : m_fields) {
            if (!field.advanced)
                continue;
            if (!sectionColumn || field.section != section) {
                section = field.section;
                sectionColumn = m_advanced->addChild<ColumnLayout>();
                sectionColumn->setSpacing(metrics::formRowSpacing);
                sectionColumn->setLayoutHints({.fillWidth = true});
                Label* title = sectionColumn->addChild<Label>(section);
                title->setWeight(600);
                title->setLayoutHints({.fillWidth = true, .margins = {0, metrics::s, 0, 0}});
                m_sectionTitles.push_back(title);
            }
            addRow(sectionColumn, field);
        }
        reload();
    }

    void addRow(ColumnLayout* parent, const schema::FormField& field)
    {
        FieldRow row;
        row.field = &field;
        row.row = addFormRow(parent);
        std::vector<Label*> unused;
        row.label = addFieldLabel(row.row, field.label, unused);
        switch (field.type) {
        case schema::Type::Password: {
            row.text = row.row->addChild<TextField>();
            row.text->setPlaceholder(tr("Asked on connect"));
            row.text->setEchoMode(TextField::EchoMode::Password);
            row.text->setLayoutHints({.fillWidth = true});
            const std::string key = field.key;
            row.text->onTextEdited = [this, key, text = row.text] {
                m_secrets[key] = text->text();
                changed();
            };
            row.text->onAccepted = [this] { accepted(); };
            row.storage = row.row->addChild<ComboBox>();
            row.storage->onActivated = [this, index = m_rows.size()](int choice) {
                FieldRow& current = m_rows[index];
                if (choice >= 0 && size_t(choice) < current.storageValues.size())
                    m_storage[current.field->key] = current.storageValues[size_t(choice)];
                update();
            };
            break;
        }
        case schema::Type::File: {
            row.text = row.row->addChild<TextField>();
            row.text->setPlaceholder(field.required ? tr("Required") : tr("Optional"));
            row.text->setLayoutHints({.fillWidth = true});
            const std::string key = field.key;
            row.text->onTextEdited = [this, key, text = row.text] { setValue(key, text->text()); };
            row.text->onAccepted = [this] { accepted(); };
            Button* choose = row.row->addChild<Button>(tr("Choose…"));
            choose->setLayoutHints({.fillHeight = true});
            const std::string title = field.label;
            choose->onClicked = [this, key, title] {
                m_fileChooser = std::make_unique<FileChooser>(window());
                m_fileChooser->setTitle(title);
                m_fileChooser->onAccepted = [this, key](const std::vector<std::string>& paths, const std::string&) {
                    if (paths.empty())
                        return;
                    setValue(key, paths.front());
                    reload();
                };
                m_fileChooser->open();
            };
            break;
        }
        case schema::Type::Choice: {
            std::vector<std::string> labels;
            for (const schema::Choice& choice : field.choices)
                labels.push_back(choice.label);
            row.box = row.row->addChild<ComboBox>(labels);
            row.box->setLayoutHints({.fillWidth = true});
            const std::string key = field.key;
            row.box->onActivated = [this, key, choices = field.choices](int index) {
                if (index >= 0 && size_t(index) < choices.size())
                    setValue(key, choices[size_t(index)].value);
            };
            break;
        }
        case schema::Type::Bool: {
            row.label->setLayoutHints({.fillWidth = true});
            row.toggle = row.row->addChild<Switch>();
            setTrailingSwitchPadding(row.toggle);
            const std::string key = field.key;
            row.toggle->onToggled = [this, key](bool checked) { setValue(key, checked ? "true" : "false"); };
            break;
        }
        default: {
            const bool number = field.type == schema::Type::Number;
            row.text = row.row->addChild<TextField>();
            row.text->setPlaceholder(!field.placeholder.empty() ? field.placeholder
                                     : field.required           ? tr("Required")
                                                                : tr("Optional"));
            row.text->setLayoutHints({.fillWidth = true});
            if (number) {
                const long long maximum = field.maximum > 0 ? field.maximum : INT_MAX;
                row.text->setValidator([maximum](std::string_view text) { return isNumberText(text, maximum); });
            }
            const std::string key = field.key;
            row.text->onTextEdited = [this, key, text = row.text] { setValue(key, text->text()); };
            row.text->onAccepted = [this] { accepted(); };
            break;
        }
        }
        m_rows.push_back(std::move(row));
    }

    void loadStorage(FieldRow& row)
    {
        const uint32_t saveFlags = m_shared ? nm::SecretSystemOwned : nm::SecretAgentOwned;
        std::vector<std::string> labels{m_shared ? tr("Saved for all") : tr("Saved"), tr("Ask each time")};
        row.storageValues = {saveFlags, nm::SecretNotSaved};
        if (row.field->allowNotRequired) {
            labels.push_back(tr("Not needed"));
            row.storageValues.push_back(nm::SecretNotRequired);
        }
        row.storage->setItems(labels);
        const auto found = m_storage.find(row.field->key);
        const uint32_t stored = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
        const uint32_t wanted = stored == 0 || stored == 1 ? saveFlags : stored;
        const auto index = std::find(row.storageValues.begin(), row.storageValues.end(), wanted);
        row.storage->setCurrentIndex(index == row.storageValues.end() ? 0 : int(index - row.storageValues.begin()));
    }

    // The fields read their values again.
    void reload()
    {
        for (FieldRow& row : m_rows) {
            const std::string& key = row.field->key;
            const auto value = m_values.find(key);
            const std::string text = value == m_values.end() ? std::string() : value->second;
            if (row.field->type == schema::Type::Password) {
                const auto secret = m_secrets.find(key);
                row.text->setText(secret == m_secrets.end() ? std::string() : secret->second);
                loadStorage(row);
            } else if (row.text) {
                row.text->setText(text);
            } else if (row.box) {
                int index = 0;
                for (size_t i = 0; i < row.field->choices.size(); ++i) {
                    if (row.field->choices[i].value == text)
                        index = int(i);
                }
                row.box->setCurrentIndex(index);
            } else if (row.toggle) {
                row.toggle->setChecked(text == "true");
            }
        }
        update();
    }

    void setValue(const std::string& key, const std::string& value)
    {
        m_values[key] = value;
        update();
    }

    void accepted()
    {
        if (onAccepted)
            onAccepted();
    }

    void changed()
    {
        if (onChanged)
            onChanged();
    }

    void update()
    {
        for (FieldRow& row : m_rows) {
            row.row->setVisible(shown(*row.field));
            if (row.storage) {
                const auto found = m_storage.find(row.field->key);
                const uint32_t storage = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
                const bool saving = storage != nm::SecretNotSaved && storage != nm::SecretNotRequired;
                row.text->setVisible(saving);
                row.storage->setLayoutHints({.fillWidth = !saving, .preferredWidth = 120});
            }
        }
        m_link->setText(m_advancedShown ? tr("Hide advanced options") : tr("Show advanced options"));
        m_advanced->setVisible(m_advancedShown);
        requestLayout();
        changed();
    }

    std::vector<schema::FormField> m_fields;
    schema::Values m_values;
    std::map<std::string, uint32_t> m_storage;
    std::map<std::string, std::string> m_secrets;
    bool m_shared = false;
    bool m_advancedShown = false;
    bool m_advancedMeasured = false;
    std::vector<FieldRow> m_rows;
    std::vector<Label*> m_sectionTitles;
    LinkLabel* m_link = nullptr;
    ColumnLayout* m_advanced = nullptr;
    std::unique_ptr<FileChooser> m_fileChooser;
};

// A WireGuard tunnel as wg-quick lays it out: the interface, then a card per peer.
class WireGuardEditor : public ColumnLayout
{
public:
    std::function<void()> onAccepted;
    std::function<void()> onChanged;

    WireGuardEditor()
    {
        setSpacing(metrics::formRowSpacing);
        setLayoutHints({.fillWidth = true});

        m_privateKey = addField(this, tr("Private key"), tr("Required"));
        m_privateKey->setEchoMode(TextField::EchoMode::Password);
        m_address = addField(this, tr("Address"), "10.0.0.2/24");
        m_dns = addField(this, tr("DNS"), tr("Optional"));

        m_link = addChild<LinkLabel>(tr("Show advanced options"));
        m_link->onClicked = [this] {
            m_advancedShown = !m_advancedShown;
            update();
        };

        m_advanced = addChild<ColumnLayout>();
        m_advanced->setSpacing(metrics::formRowSpacing);
        m_advanced->setLayoutHints({.fillWidth = true});
        m_interface = addField(m_advanced, tr("Interface name"), tr("From the name"));
        m_interface->setMaximumLength(15);
        m_listenPort = addField(m_advanced, tr("Listen port"), tr("Automatic"));
        m_listenPort->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        m_fwmark = addField(m_advanced, tr("Firewall mark"), tr("Optional"));
        m_mtu = addField(m_advanced, tr("MTU"), tr("Automatic"));
        m_mtu->setValidator([](std::string_view text) { return isNumberText(text, INT_MAX); });
        RowLayout* routes = addFormRow(m_advanced);
        Label* routesLabel = routes->addChild<Label>(tr("Add routes for the peers' allowed IPs"));
        routesLabel->setWrap(true);
        routesLabel->setLayoutHints({.fillWidth = true});
        m_peerRoutes = routes->addChild<Switch>();
        setTrailingSwitchPadding(m_peerRoutes);
        m_peerRoutes->setChecked(true);

        m_peers = addChild<ColumnLayout>();
        m_peers->setSpacing(metrics::formRowSpacing);
        m_peers->setLayoutHints({.fillWidth = true});
        m_addPeer = addChild<LinkLabel>(tr("Add peer"));
        m_addPeer->onClicked = [this] {
            addPeer({});
            update();
        };
    }

    void load(const nm::vpn::WireGuardForm& form)
    {
        m_interface->setText(form.interfaceName);
        m_privateKey->setText({});
        m_hasPrivateKey = false;
        m_address->setText(form.addresses);
        m_dns->setText(form.dns);
        m_listenPort->setText(form.listenPort);
        m_fwmark->setText(form.fwmark);
        m_mtu->setText(form.mtu);
        m_peerRoutes->setChecked(form.peerRoutes);
        m_advancedShown = false;
        while (!m_peers->children().empty())
            m_peers->takeChild(m_peers->children().front().get());
        m_peerCards.clear();
        for (const nm::vpn::WireGuardPeer& peer : form.peers)
            addPeer(peer);
        if (m_peerCards.empty())
            addPeer({});
        update();
    }

    void setSecrets(const nm::VpnSecrets& secrets)
    {
        if (!secrets.privateKey.empty()) {
            m_privateKey->setText(secrets.privateKey);
            m_hasPrivateKey = true;
        }
        for (PeerCard& card : m_peerCards) {
            const auto key = secrets.presharedKeys.find(card.publicKey->text());
            if (key != secrets.presharedKeys.end())
                card.presharedKey->setText(key->second);
        }
        update();
    }

    nm::vpn::WireGuardForm collect() const
    {
        nm::vpn::WireGuardForm form;
        form.interfaceName = m_interface->text();
        form.privateKey = m_privateKey->text();
        form.addresses = m_address->text();
        form.dns = m_dns->text();
        form.listenPort = m_listenPort->text();
        form.fwmark = m_fwmark->text();
        form.mtu = m_mtu->text();
        form.peerRoutes = m_peerRoutes->isChecked();
        for (const PeerCard& card : m_peerCards) {
            form.peers.push_back({card.publicKey->text(), card.presharedKey->text(), card.hasPresharedKey,
                                  card.endpoint->text(), card.allowedIps->text(), card.keepalive->text()});
        }
        return form;
    }

    bool complete() const
    {
        const auto blank = [](const std::string& text) { return text.find_first_not_of(" \t\n") == std::string::npos; };
        if (blank(m_address->text()))
            return false;
        if (!m_hasPrivateKey && blank(m_privateKey->text()))
            return false;
        if (m_peerCards.empty())
            return false;
        return std::none_of(m_peerCards.begin(), m_peerCards.end(),
                            [&blank](const PeerCard& card) { return blank(card.publicKey->text()); });
    }

    std::vector<Label*> labels() const { return m_labels; }

    void setLabelWidth(double width)
    {
        for (Label* label : m_labels) {
            LayoutHints hints = label->layoutHints();
            if (hints.preferredWidth != width) {
                hints.preferredWidth = width;
                label->setLayoutHints(hints);
            }
        }
    }

private:
    struct PeerCard
    {
        Card* card = nullptr;
        Label* title = nullptr;
        Button* remove = nullptr;
        TextField* publicKey = nullptr;
        TextField* presharedKey = nullptr;
        bool hasPresharedKey = false;
        TextField* endpoint = nullptr;
        TextField* allowedIps = nullptr;
        TextField* keepalive = nullptr;
        // The five field labels, which leave the label column with the card.
        Label* labels[5] = {};
    };

    TextField* addField(ColumnLayout* parent, const char* label, const char* placeholder)
    {
        RowLayout* row = addFormRow(parent);
        addFieldLabel(row, label, m_labels);
        TextField* field = row->addChild<TextField>();
        field->setPlaceholder(placeholder);
        field->setLayoutHints({.fillWidth = true});
        field->onTextEdited = [this] { changed(); };
        field->onAccepted = [this] {
            if (onAccepted)
                onAccepted();
        };
        return field;
    }

    void addPeer(const nm::vpn::WireGuardPeer& peer)
    {
        PeerCard card;
        card.card = m_peers->addChild<Card>();
        card.card->setPadding(Margins::all(metrics::cardHorizontalPadding));
        card.card->setSpacing(metrics::formRowSpacing);
        RowLayout* header = addFormRow(card.card);
        card.title = header->addChild<Label>(tr("Peer"));
        card.title->setWeight(600);
        card.title->setLayoutHints({.fillWidth = true});
        card.remove = header->addChild<Button>(tr("Remove"));
        Card* owner = card.card;
        card.remove->onClicked = [this, owner] {
            const auto found = std::find_if(m_peerCards.begin(), m_peerCards.end(),
                                            [owner](const PeerCard& card) { return card.card == owner; });
            if (found == m_peerCards.end())
                return;
            for (Label* label : {found->labels[0], found->labels[1], found->labels[2], found->labels[3], found->labels[4]})
                std::erase(m_labels, label);
            m_peerCards.erase(found);
            // The card holds the button being clicked.
            std::shared_ptr<View> gone = m_peers->takeChild(owner);
            update();
            Lingmo::base::EventLoop::current()->post([gone] {});
        };
        card.publicKey = addField(card.card, tr("Public key"), tr("Required"));
        card.publicKey->setText(peer.publicKey);
        card.presharedKey = addField(card.card, tr("Preshared key"),
                                     // TRANSLATORS: a placeholder: the stored key stays unless a new one is typed.
                                     peer.hasPresharedKey ? tr("Kept") : tr("Optional"));
        card.presharedKey->setEchoMode(TextField::EchoMode::Password);
        card.presharedKey->setText(peer.presharedKey);
        card.hasPresharedKey = peer.hasPresharedKey;
        card.endpoint = addField(card.card, tr("Endpoint"), "vpn.example.com:51820");
        card.endpoint->setText(peer.endpoint);
        card.allowedIps = addField(card.card, tr("Allowed IPs"), tr("All traffic"));
        card.allowedIps->setText(peer.allowedIps);
        card.keepalive = addField(card.card, tr("Keepalive"), tr("Off"));
        card.keepalive->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        card.keepalive->setText(peer.keepalive);
        std::copy(m_labels.end() - 5, m_labels.end(), card.labels);
        TextField* preshared = card.presharedKey;
        card.presharedKey->onTextEdited = [this, owner, preshared] {
            // Clearing the field drops the stored key.
            for (PeerCard& card : m_peerCards) {
                if (card.card == owner)
                    card.hasPresharedKey = !preshared->text().empty();
            }
            changed();
        };
        m_peerCards.push_back(card);
    }

    void changed()
    {
        if (onChanged)
            onChanged();
    }

    void update()
    {
        m_link->setText(m_advancedShown ? tr("Hide advanced options") : tr("Show advanced options"));
        m_advanced->setVisible(m_advancedShown);
        const bool several = m_peerCards.size() > 1;
        for (size_t i = 0; i < m_peerCards.size(); ++i) {
            m_peerCards[i].title->setText(several ? format(tr("Peer {number}"), {{"number", std::to_string(i + 1)}})
                                                  : std::string(tr("Peer")));
            m_peerCards[i].remove->setVisible(several);
            // The card's own air above it.
            m_peerCards[i].card->setLayoutHints({.fillWidth = true, .margins = {0, metrics::s, 0, 0}});
        }
        // Without peers the column would still take a spacing.
        m_peers->setVisible(!m_peerCards.empty());
        requestLayout();
        changed();
    }

    std::vector<Label*> m_labels;
    TextField* m_privateKey;
    TextField* m_address;
    TextField* m_dns;
    LinkLabel* m_link;
    ColumnLayout* m_advanced;
    TextField* m_interface;
    TextField* m_listenPort;
    TextField* m_fwmark;
    TextField* m_mtu;
    Switch* m_peerRoutes;
    ColumnLayout* m_peers;
    LinkLabel* m_addPeer;
    std::vector<PeerCard> m_peerCards;
    bool m_hasPrivateKey = false;
    bool m_advancedShown = false;
};

VpnEditor::VpnEditor()
{
    setSpacing(metrics::formRowSpacing);
    setLayoutHints({.fillWidth = true});

    m_notInstalled = addChild<Label>();
    m_notInstalled->setWrap(true);
    m_notInstalled->setLayoutHints({.fillWidth = true});

    m_nameRow = addFormRow(this);
    std::vector<Label*> unused;
    m_nameLabel = addFieldLabel(m_nameRow, tr("Name"), unused);
    m_name = m_nameRow->addChild<TextField>();
    m_name->setPlaceholder(tr("Optional"));
    m_name->setLayoutHints({.fillWidth = true});
    m_name->onAccepted = [this] {
        if (onAccepted)
            onAccepted();
    };

    m_switches = addChild<ColumnLayout>();
    m_switches->setSpacing(metrics::formRowSpacing);
    m_switches->setLayoutHints({.fillWidth = true, .margins = {0, metrics::s, 0, 0}});
    const auto addSwitch = [this](const char* text) {
        RowLayout* row = addFormRow(m_switches);
        Label* label = row->addChild<Label>(text);
        label->setWrap(true);
        label->setLayoutHints({.fillWidth = true});
        Switch* toggle = row->addChild<Switch>();
        setTrailingSwitchPadding(toggle);
        return toggle;
    };
    m_shared = addSwitch(tr("Available to all users"));
    m_shared->onToggled = [this](bool checked) {
        if (m_plugin)
            m_plugin->setShared(checked);
    };
    m_ownNetworkOnly = addSwitch(tr("Use only for resources on its network"));
}

VpnEditor::~VpnEditor() = default;

void VpnEditor::load(const nm::VpnForm& form)
{
    m_form = form;
    m_extraLabelWidth = 0;
    m_name->setText(form.name);
    m_shared->setChecked(form.shared);
    m_ownNetworkOnly->setChecked(form.ownNetworkOnly);

    if (m_plugin)
        takeChild(m_plugin);
    if (m_wireGuard)
        takeChild(m_wireGuard);
    m_plugin = nullptr;
    m_wireGuard = nullptr;
    if (form.installed) {
        // Between the name and the switches.
        std::unique_ptr<View> switches = takeChild(m_switches);
        if (form.kind == "wireguard") {
            m_wireGuard = addChild<WireGuardEditor>();
            m_wireGuard->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_wireGuard->onChanged = [this] { changed(); };
            m_wireGuard->load(form.wireguard);
        } else {
            m_plugin = addChild<PluginForm>();
            m_plugin->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_plugin->onChanged = [this] { changed(); };
            m_plugin->load(form.fields, form.shared);
        }
        addChild(std::move(switches));
    }

    m_notInstalled->setText(format(tr("Install {package} to set up this kind of VPN."), {{"package", form.package}}));
    m_notInstalled->setVisible(!form.installed && !form.kind.empty());
    m_nameRow->setVisible(form.installed);
    m_switches->setVisible(form.installed);
    changed();
}

void VpnEditor::setSecrets(const nm::VpnSecrets& secrets)
{
    if (m_plugin)
        m_plugin->setSecrets(secrets.values);
    if (m_wireGuard)
        m_wireGuard->setSecrets(secrets);
}

nm::VpnValues VpnEditor::collect() const
{
    nm::VpnValues values;
    values.kind = m_form.kind;
    values.name = m_name->text();
    values.shared = m_shared->isChecked();
    values.ownNetworkOnly = m_ownNetworkOnly->isChecked();
    if (m_plugin) {
        values.values = m_plugin->values();
        values.storage = m_plugin->storage();
        values.secrets = m_plugin->secrets();
    }
    if (m_wireGuard)
        values.wireguard = m_wireGuard->collect();
    return values;
}

bool VpnEditor::complete() const
{
    if (!m_form.installed)
        return false;
    if (m_plugin)
        return m_plugin->complete();
    if (m_wireGuard)
        return m_wireGuard->complete();
    return false;
}

void VpnEditor::focusFirst()
{
    m_name->setFocus();
}

void VpnEditor::measure(double width)
{
    m_extraLabelWidth = std::max(m_extraLabelWidth, std::min(width, kMaximumLabelWidth));
    changed();
}

void VpnEditor::changed()
{
    requestLayout();
    if (onChanged)
        onChanged();
}

double VpnEditor::labelWidth() const
{
    std::vector<Label*> labels{m_nameLabel};
    if (m_plugin) {
        for (Label* label : m_plugin->labels())
            labels.push_back(label);
    }
    if (m_wireGuard) {
        for (Label* label : m_wireGuard->labels())
            labels.push_back(label);
    }
    double width = m_extraLabelWidth;
    for (const Label* label : labels)
        width = std::max(width, std::min(label->sizeHint().width, kMaximumLabelWidth));
    return width;
}

void VpnEditor::layout()
{
    m_notInstalled->setColor(Theme::current().disabledText);
    const double width = labelWidth();
    LayoutHints hints = m_nameLabel->layoutHints();
    if (hints.preferredWidth != width) {
        hints.preferredWidth = width;
        m_nameLabel->setLayoutHints(hints);
    }
    if (m_plugin)
        m_plugin->setLabelWidth(width);
    if (m_wireGuard)
        m_wireGuard->setLabelWidth(width);
    ColumnLayout::layout();
}

} // namespace settings::network
