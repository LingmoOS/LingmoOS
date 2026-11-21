// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/bluetooth/bluetoothdialogs.h"

#include <map>
#include <string>

#include "base/i18n.h"
#include "ui/dialog_window.h"
#include "ui/cell_layout.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace settings {

static constexpr double kDialogWidth = 400;
// Bluetooth names are at most 248 bytes.
static constexpr size_t kNameLength = 248;

// The kind of device behind each icon name BlueZ reports.
static std::string deviceType(const std::string& icon)
{
    static const std::map<std::string, const char*> types = {
        {"audio-card", trNoop("Audio device")},
        {"audio-headphones", trNoop("Headphones")},
        {"audio-headset", trNoop("Headset")},
        {"camera-photo", trNoop("Camera")},
        {"camera-video", trNoop("Video camera")},
        {"computer", trNoop("Computer")},
        {"input-gaming", trNoop("Game controller")},
        {"input-keyboard", trNoop("Keyboard")},
        {"input-mouse", trNoop("Mouse")},
        {"input-tablet", trNoop("Drawing tablet")},
        {"modem", trNoop("Modem")},
        {"multimedia-player", trNoop("Media player")},
        {"network-wireless", trNoop("Network device")},
        {"phone", trNoop("Phone")},
        {"printer", trNoop("Printer")},
        {"scanner", trNoop("Scanner")},
        {"video-display", trNoop("Display")},
    };
    const auto found = types.find(icon);
    return found == types.end() ? std::string() : tr(found->second);
}

// The dialog's content: the page's padding on top, as the window keeps it below.
static ColumnLayout* makeContent(DialogWindow& dialog)
{
    ColumnLayout* content = dialog.setContent<ColumnLayout>();
    content->setPadding({0, metrics::dialogPadding, 0, 0});
    content->setSpacing(metrics::dialogSpacing);
    return content;
}

static RowLayout* makeButtons(ColumnLayout* content)
{
    RowLayout* buttons = content->addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    return buttons;
}

AdapterDetailsDialog::AdapterDetailsDialog(Window* parent, std::shared_ptr<bluez::BluetoothObject> adapter)
    : m_adapter(std::move(adapter))
    , m_dialog(std::make_unique<DialogWindow>(parent, m_adapter->name(), kDialogWidth))
{
    ColumnLayout* content = makeContent(*m_dialog);

    Card* identity = content->addChild<Card>(Card::Style::List);
    ListRow* nameRow = identity->addChild<ListRow>(tr("Name"));
    nameRow->setInteractive(false);
    nameRow->setMinimumRowHeight(metrics::controlRowHeight);
    m_name = nameRow->addContent<TextField>();
    m_name->setLayoutHints({.preferredWidth = 200, .horizontalAlignment = Alignment::End});
    m_name->setText(m_adapter->name());
    m_name->onTextChanged = [this] {
        if (m_name->text().size() > kNameLength) {
            std::string text = m_name->text();
            // Not inside a character.
            size_t end = kNameLength;
            while (end > 0 && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80)
                --end;
            text.resize(end);
            m_name->setText(text);
        }
    };
    m_name->onAccepted = [this] { rename(); };
    m_address = identity->addChild<InfoRow>(tr("Address"));

    Card* switches = content->addChild<Card>(Card::Style::List);
    m_powered = switches->addChild<SwitchRow>(tr("Powered"));
    m_powered->onToggled = [this](bool checked) { m_adapter->setPowered(checked); };
    m_discoverable = switches->addChild<SwitchRow>(tr("Visible to other devices"));
    m_discoverable->onToggled = [this](bool checked) { m_adapter->setDiscoverable(checked); };
    m_pairable = switches->addChild<SwitchRow>(tr("Allow incoming pairing"));
    m_pairable->onToggled = [this](bool checked) { m_adapter->setPairable(checked); };

    DialogButton* close = makeButtons(content)->addChild<DialogButton>(tr("Close"));
    close->onClicked = [this] {
        rename();
        m_dialog->close();
    };

    m_changed = m_adapter->changed.connect([this] { update(); });
    update();
}

AdapterDetailsDialog::~AdapterDetailsDialog() = default;

void AdapterDetailsDialog::show()
{
    m_dialog->show();
}

void AdapterDetailsDialog::update()
{
    m_dialog->setTitle(m_adapter->name());
    m_address->setValue(m_adapter->address());
    m_powered->setChecked(m_adapter->powered());
    m_discoverable->setChecked(m_adapter->discoverable());
    m_discoverable->control()->setEnabled(m_adapter->powered());
    m_pairable->setChecked(m_adapter->pairable());
    m_pairable->control()->setEnabled(m_adapter->powered());
}

void AdapterDetailsDialog::rename()
{
    const std::string name = m_name->text();
    if (!name.empty() && name != m_adapter->name())
        m_adapter->setName(name);
}

DeviceDetailsDialog::DeviceDetailsDialog(Window* parent, std::shared_ptr<bluez::BluetoothObject> device)
    : m_device(std::move(device))
    , m_dialog(std::make_unique<DialogWindow>(parent, m_device->name(), kDialogWidth))
    , m_servicesResolved(m_device->servicesResolved())
{
    ColumnLayout* content = makeContent(*m_dialog);

    Card* details = content->addChild<Card>(Card::Style::List);
    m_type = details->addChild<InfoRow>(tr("Device type"));
    m_model = details->addChild<InfoRow>(tr("Model number"));
    m_version = details->addChild<InfoRow>(tr("Version"));
    m_address = details->addChild<InfoRow>(tr("Address"));

    Card* switches = content->addChild<Card>(Card::Style::List);
    m_trusted = switches->addChild<SwitchRow>(tr("Trusted"));
    m_trusted->onToggled = [this](bool checked) { m_device->setTrusted(checked); };
    m_blocked = switches->addChild<SwitchRow>(tr("Blocked"));
    m_blocked->onToggled = [this](bool checked) { m_device->setBlocked(checked); };

    RowLayout* buttons = makeButtons(content);
    DialogButton* close = buttons->addChild<DialogButton>(tr("Close"));
    close->onClicked = [this] { m_dialog->close(); };
    m_forget = buttons->addChild<DialogButton>(tr("Forget this device"), true);
    m_forget->setDestructive(true);
    m_forget->onClicked = [this] {
        // BlueZ removes the object immediately after RemoveDevice. Stop
        // receiving its final Changed signal before the shared object is
        // detached, otherwise the dialog can update destroyed controls.
        m_changed = {};
        m_forget->setEnabled(false);
        m_device->forget();
        m_dialog->close();
    };

    m_changed = m_device->changed.connect([this] {
        const bool resolved = m_device->servicesResolved();
        if (resolved && !m_servicesResolved)
            m_device->readDeviceInformation();
        m_servicesResolved = resolved;
        update();
    });
    m_device->readDeviceInformation();
    update();
}

DeviceDetailsDialog::~DeviceDetailsDialog() = default;

void DeviceDetailsDialog::show()
{
    m_dialog->show();
}

void DeviceDetailsDialog::update()
{
    m_dialog->setTitle(m_device->name());
    m_type->setValue(deviceType(m_device->icon()));
    m_model->setValue(m_device->modelNumber());
    m_version->setValue(m_device->firmwareRevision());
    m_address->setValue(m_device->address());
    m_trusted->setVisible(m_device->paired());
    m_trusted->setChecked(m_device->trusted());
    m_trusted->control()->setEnabled(!m_device->busy());
    m_blocked->setChecked(m_device->blocked());
    m_blocked->control()->setEnabled(!m_device->busy());
    m_forget->setVisible(m_device->paired());
    m_forget->setEnabled(!m_device->busy());
}

} // namespace settings
