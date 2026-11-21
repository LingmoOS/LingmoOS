// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/hotspot.h"
#include "pages/network/common/textchecks.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

static constexpr double kFieldWidth = 220;
static constexpr double kGenerateSize = 18;

namespace {

// The password field, with the button that makes a new password inside it,
// its ink inset from the edge as far as the text is.
class PasswordField : public View
{
public:
    PasswordField()
    {
        field = addChild<TextField>();
        const Theme& theme = Theme::current();
        field->setTextMargins(theme.largeSpacing, theme.largeSpacing + kGenerateSize + theme.smallSpacing);
        generate = addChild<IconButton>("refresh", kGenerateSize);
    }

    SizeF sizeHint() const override { return field->sizeHint(); }

    TextField* field;
    IconButton* generate;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        field->setGeometry(localRect());
        const double right = theme.largeSpacing + metrics::trailingGlyphMargin("refresh", kGenerateSize);
        generate->setGeometry({geometry().width - right - kGenerateSize,
                               rounding::centeredStart(geometry().height, kGenerateSize), kGenerateSize, kGenerateSize});
        generate->setColor(theme.highlight);
    }
};

// Shares this computer's internet connection over a Wi-Fi network of its own.
// The name and password are set while the hotspot is off and take effect when
// it starts.
class HotspotPage : public Page
{
public:
    explicit HotspotPage(SettingsWindow& window)
        : Page(window, tr("Hotspot"))
        , m_configuration(nm::HotspotConfiguration::instance())
    {
        content()->setSpacing(metrics::sectionLabelSpacing);

        Card* switchCard = content()->addChild<Card>(Card::Style::List);
        m_switch = switchCard->addChild<SwitchRow>(tr("Wi-Fi hotspot"));
        m_switch->setName("hotspot:switch");
        m_switch->onToggled = [this](bool checked) {
            if (checked)
                requestStart();
            else
                m_handler.stopHotspot();
            // The switch shows what NetworkManager reports, not what was asked for.
            m_switch->setChecked(m_handler.hotspotActive());
        };

        m_status = addNote();
        m_status->setName("hotspot:status");

        // The cards stand a section apart, the status line between them.
        Card* fields = content()->addChild<Card>(Card::Style::List);
        fields->setLayoutHints(
            {.fillWidth = true, .margins = {0, metrics::sectionSpacing - metrics::sectionLabelSpacing, 0, 0}});
        ListRow* nameRow = fields->addChild<ListRow>(tr("Network name"));
        nameRow->setInteractive(false);
        nameRow->setMinimumRowHeight(metrics::controlRowHeight);
        m_name = nameRow->addContent<TextField>(m_configuration->name());
        m_name->setName("hotspot:name");
        m_name->setLayoutHints({.preferredWidth = kFieldWidth});
        m_name->onTextEdited = [this] { update(); };
        m_name->onEditingFinished = [this] {
            if (nm::HotspotConfiguration::nameValid(m_name->text()))
                m_configuration->setName(m_name->text());
        };

        ListRow* passwordRow = fields->addChild<ListRow>(tr("Password"));
        passwordRow->setInteractive(false);
        passwordRow->setMinimumRowHeight(metrics::controlRowHeight);
        m_password = passwordRow->addContent<PasswordField>();
        m_password->setName("hotspot:password");
        m_password->setLayoutHints({.preferredWidth = kFieldWidth});
        m_password->field->setText(m_configuration->password());
        m_password->field->setMaximumLength(64);
        m_password->field->onTextEdited = [this] { update(); };
        m_password->field->onEditingFinished = [this] {
            if (nm::HotspotConfiguration::passwordValid(m_password->field->text()))
                m_configuration->setPassword(m_password->field->text());
        };
        m_password->generate->setToolTip(tr("Generate password"));
        m_password->generate->onClicked = [this] {
            m_password->field->setText(nm::HotspotConfiguration::generatePassword());
            m_configuration->setPassword(m_password->field->text());
            update();
        };

        m_validation = addNote();
        m_validation->setName("hotspot:validation");

        m_subscriptions.push_back(m_handler.hotspotFailed.connect([this](const std::string& message) {
            m_errorMessage = message;
            update();
        }));
        m_subscriptions.push_back(m_handler.hotspotActiveChanged.connect([this] {
            if (m_handler.hotspotActive())
                m_errorMessage.clear();
            update();
        }));
        m_subscriptions.push_back(m_handler.hotspotSupportedChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_enabled.changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_configuration->changed.connect([this] {
            // The fields follow the configuration while they aren't being edited.
            if (!m_name->hasFocus())
                m_name->setText(m_configuration->name());
            if (!m_password->field->hasFocus())
                m_password->field->setText(m_configuration->password());
            update();
        }));
        update();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_status->setColor(m_errorMessage.empty() ? theme.disabledText : accent::red);
        m_validation->setColor(accent::red);
        Page::layout();
    }

private:
    // Wrapping text lined up with the cards' text.
    Label* addNote()
    {
        Label* label = content()->addChild<Label>();
        label->setWrap(true);
        label->setLayoutHints(
            {.fillWidth = true, .margins = {metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0}});
        return label;
    }

    void start()
    {
        m_configuration->setName(m_name->text());
        m_configuration->setPassword(m_password->field->text());
        m_errorMessage.clear();
        update();
        m_handler.createHotspot();
    }

    void requestStart()
    {
        if (m_handler.hotspotReplaces().empty()) {
            start();
            return;
        }
        m_confirm = std::make_unique<ConfirmDialog>(
            &settingsWindow().window(), tr("Turn On Wi-Fi Hotspot?"),
            format(tr("Turning on the hotspot will disconnect from “{network}”, and you will not have access to the "
                      "internet it provides."),
                   {{"network", m_handler.hotspotReplaces()}}));
        m_confirm->setCancelText(tr("Cancel"));
        m_confirm->setConfirmText(tr("Turn on"));
        m_confirm->onAccepted = [this] { start(); };
        m_confirm->show();
    }

    std::string description() const
    {
        return tr("Share this computer's internet connection by creating a Wi-Fi network other devices can join. "
                  "The connection to share must come from a source other than Wi-Fi.");
    }

    void update()
    {
        const bool active = m_handler.hotspotActive();
        const bool nameValid = nm::HotspotConfiguration::nameValid(m_name->text());
        const bool passwordValid = nm::HotspotConfiguration::passwordValid(m_password->field->text());
        m_switch->control()->setEnabled(active || (m_handler.hotspotSupported() && nameValid && passwordValid));
        m_switch->setChecked(active);

        std::string status;
        if (!m_errorMessage.empty())
            status = m_errorMessage;
        else if (!m_enabled.isReady())
            status = description();
        else if (!m_enabled.isWirelessEnabled())
            status = tr("Turn on Wi-Fi to use the hotspot.");
        else if (!m_handler.hotspotAllowed())
            status = tr("System policy prohibits use as a hotspot.");
        else if (active)
            status = format(tr("Other devices can join “{network}” with the password below. Turn off the hotspot to "
                               "use Wi-Fi on this computer."),
                            {{"network", m_configuration->name()}});
        else
            status = description();
        m_status->setText(status);

        m_name->setEnabled(!active);
        m_password->setEnabled(!active);

        std::string validation;
        if (!nameValid)
            validation = m_name->text().empty() ? tr("Network name cannot be empty.") : tr("Network name is too long.");
        else if (!passwordValid)
            validation = m_password->field->text().size() < 8 ? tr("The password must have at least 8 characters.")
                                                               : tr("The password must have at most 63 characters.");
        m_validation->setText(validation);
        m_validation->setVisible(!validation.empty());
        requestLayout();
    }

    nm::Handler m_handler;
    nm::EnabledConnections m_enabled;
    nm::HotspotConfiguration* m_configuration;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    std::unique_ptr<ConfirmDialog> m_confirm;
    std::string m_errorMessage;
    SwitchRow* m_switch;
    Label* m_status;
    TextField* m_name;
    PasswordField* m_password;
    Label* m_validation;
};

// Search offers the page only where a Wi-Fi adapter can run a hotspot.
class HotspotWatch : public AvailabilityWatch
{
public:
    explicit HotspotWatch(AvailabilityCallback callback)
        : m_callback(std::move(callback))
    {
        m_subscriptions.push_back(m_handler.hotspotSupportedChanged.connect([this] { report(); }));
        m_subscriptions.push_back(m_enabled.changed.connect([this] { report(); }));
        report();
    }

private:
    void report()
    {
        if (m_enabled.isReady())
            m_callback(m_handler.hotspotCapable());
    }

    nm::Handler m_handler;
    nm::EnabledConnections m_enabled;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    AvailabilityCallback m_callback;
};

} // namespace

static const PageRegistration registration(
    "hotspot", [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<HotspotPage>(window); },
    [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
        return std::make_unique<HotspotWatch>(std::move(callback));
    });

} // namespace settings::network
