// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include "base/dbus.h"
#include "base/i18n.h"
#include "base/process.h"
#include "base/signal.h"
#include "lingmo/update/update_status.h"
#include "ui/button.h"
#include "ui/color_provider.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/progress_bar.h"
#include "ui/text_markup.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
using Lingmo::i18n::format;

namespace settings {

namespace {

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

// Format a Unix timestamp as "今天，HH:MM", "昨天，HH:MM", or "YYYY-MM-DD".
static std::string formatCheckTime(int64_t unixTime)
{
    if (unixTime == 0)
        return tr("Never");

    const std::time_t t = static_cast<std::time_t>(unixTime);
    std::tm checkTm{};
    localtime_r(&t, &checkTm);

    const std::time_t now = std::time(nullptr);
    std::tm nowTm{};
    localtime_r(&now, &nowTm);

    char timeBuf[6];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", &checkTm);

    if (checkTm.tm_year == nowTm.tm_year && checkTm.tm_yday == nowTm.tm_yday)
        return format(tr("Today, {time}"), {{"time", timeBuf}});

    // Yesterday
    std::tm yesterday = nowTm;
    yesterday.tm_mday -= 1;
    std::mktime(&yesterday);
    if (checkTm.tm_year == yesterday.tm_year && checkTm.tm_yday == yesterday.tm_yday)
        return format(tr("Yesterday, {time}"), {{"time", timeBuf}});

    char dateBuf[11];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &checkTm);
    return dateBuf;
}

// The update information card shown in SystemUpgrade/SoftwareUpdate/
// RestartRequired/Error states.
class UpdateInfoCard : public Card
{
public:
    UpdateInfoCard()
        : Card(Card::Style::Padded)
    {
        // Header row: icon + title + size label
        RowLayout* header = addChild<RowLayout>();
        header->setSpacing(metrics::s);
        header->setLayoutHints({.fillWidth = true});

        m_icon = header->addChild<RowIcon>("update.svg",
                                           Color::rgb(0x0067FF),
                                           Theme::current().highlight);

        ColumnLayout* titleCol = header->addChild<ColumnLayout>();
        titleCol->setSpacing(0);
        titleCol->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        m_titleLabel = titleCol->addChild<Label>();
        m_titleLabel->setLayoutHints({.fillWidth = true});
        m_titleLabel->setElide(Label::Elide::Right);

        m_sizeLabel = header->addChild<SecondaryLabel>();

        // Separator
        m_separator = addChild<View>();
        m_separator->setLayoutHints({.fillWidth = true, .preferredHeight = 1});

        // Changelog / error log text
        m_bodyLabel = addChild<Label>();
        m_bodyLabel->setWrap(true);
        m_bodyLabel->setLayoutHints({.fillWidth = true});
    }

    void setTitle(const std::string& title) { m_titleLabel->setText(title); }
    void setSizeText(const std::string& text) { m_sizeLabel->setText(text); }
    void setBody(const std::string& text, bool markup = false)
    {
        if (text.empty()) {
            m_bodyLabel->setText({});
            return;
        }
        if (!markup) {
            m_bodyLabel->setText(text);
            return;
        }
        std::vector<Label::Span> spans = styledTextSpans(text, Theme::current().highlight);
        if (spans.empty()) {
            m_bodyLabel->setText({});
            return;
        }
        if (spans.size() == 1 && !spans.front().weight && !spans.front().color
            && !spans.front().italic && !spans.front().underline) {
            m_bodyLabel->setText(spans.front().text);
        } else {
            m_bodyLabel->setSpans(std::move(spans));
        }
    }
    void setIconColor(Color tint) { m_icon->set("update.svg", tint); }

protected:
    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left || !m_bodyLabel->isVisible())
            return;
        // A link in the release notes opens, as in a notification body.
        const RectF body = m_bodyLabel->geometry();
        const std::string link = m_bodyLabel->linkAt(
                {event.position.x - body.x, event.position.y - body.y});
        if (!link.empty())
            base::Process::startDetached("xdg-open", {link});
    }

    void paint(Painter& painter) override
    {
        Card::paint(painter);
        // Draw the separator using the toolkit's Separator colour.
        const RectF sep = m_separator->geometry();
        painter.fillRect({sep.x, sep.y, sep.width, 1},
                         color(ColorId::Separator));
    }

private:
    RowIcon* m_icon;
    Label*   m_titleLabel;
    SecondaryLabel* m_sizeLabel;
    View*    m_separator;
    Label*   m_bodyLabel;
};

class UpdatePage : public Page
{
public:
    explicit UpdatePage(SettingsWindow& window)
        : Page(window, tr("Software Update"))
    {
        buildHeader();
        buildUpdateCard();
        buildAdvancedSection();

        m_status = update::UpdateStatus::shared();
        m_subscriptions.push_back(m_status->changed.connect([this] { refresh(); }));
        m_subscriptions.push_back(m_status->upgradeProgressChanged.connect([this] { refreshProgress(); }));

        refresh();
    }

private:
    // ── header card ──────────────────────────────────────────────────────────
    void buildHeader()
    {
        m_headerCard = content()->addChild<Card>();
        m_headerCard->setName("update:header");

        RowLayout* row = m_headerCard->addChild<RowLayout>();
        row->setSpacing(metrics::m);
        row->setLayoutHints({.fillWidth = true});

        // Left: icon + text column
        m_headerIcon = row->addChild<RowIcon>("update.svg",
                                              Color::rgb(0x0067FF),
                                              Theme::current().highlight);

        ColumnLayout* textCol = row->addChild<ColumnLayout>();
        textCol->setSpacing(metrics::textSpacing);
        textCol->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});

        m_statusLabel = textCol->addChild<Label>();
        m_statusLabel->setName("update:status");
        m_statusLabel->setLayoutHints({.fillWidth = true});

        m_progressBar = textCol->addChild<ProgressBar>();
        m_progressBar->setLayoutHints({.fillWidth = true});
        m_progressBar->setVisible(false);

        m_subtitleLabel = textCol->addChild<SecondaryLabel>();
        m_subtitleLabel->setLayoutHints({.fillWidth = true});

        // Right: action button
        m_actionButton = row->addChild<Button>();
        m_actionButton->setName("update:action");
        m_actionButton->setLayoutHints({.verticalAlignment = Alignment::Center});
        m_actionButton->onClicked = [this] { onActionClicked(); };
    }

    // ── update info card ─────────────────────────────────────────────────────
    void buildUpdateCard()
    {
        m_updateCard = content()->addChild<UpdateInfoCard>();
        m_updateCard->setName("update:infocard");
        m_updateCard->setVisible(false);
    }

    // ── advanced settings entry ──────────────────────────────────────────────
    void buildAdvancedSection()
    {
        Section* section = content()->addChild<Section>(tr("More options"));
        section->setName("update:options");
        Card* card = section->addChild<Card>(Card::Style::List);

        NavigationItem* advanced = card->addChild<NavigationItem>(tr("Advanced Settings"));
        advanced->setName("update:advanced");
        advanced->setLeading<RowIcon>("update.svg", Color::rgb(0x0067FF));
        advanced->onClicked = [this] { settingsWindow().pushPage("update-advanced"); };
    }

    // ── state refresh ─────────────────────────────────────────────────────────
    void refresh()
    {
        // Service not yet on the bus — grey out and disable until it appears.
        if (!m_status->isAvailable()) {
            m_closeAfterUpgradeStart = false;
            m_headerIcon->set("update.svg", Color::rgb(0x8E8E93));
            m_statusLabel->setText(tr("Update service unavailable"));
            m_subtitleLabel->setText(tr("The system service is not running."));
            m_subtitleLabel->setVisible(true);
            m_progressBar->setVisible(false);
            m_updateCard->setVisible(false);
            m_actionButton->setText(tr("Check for updates"));
            m_actionButton->setEnabled(false);
            return;
        }

        const update::State state = m_status->state();
        if (m_closeAfterUpgradeStart) {
            if (state == update::State::Upgrading) {
                m_closeAfterUpgradeStart = false;
                // The progress and the restart prompt are notifications now;
                // the update keeps running in lingmo-services.
                if (Lingmo::ui::Window* w = window())
                    w->hide();
            } else if (state == update::State::Error
                       || state == update::State::RestartRequired
                       || !m_status->requestError().empty()) {
                m_closeAfterUpgradeStart = false;
            }
        }
        const int64_t lastCheck   = m_status->lastCheckTime();
        const std::string lastCheckStr = format(
            tr("Last checked: {time}"), {{"time", formatCheckTime(lastCheck)}});

        // Reset defaults
        m_progressBar->setVisible(false);
        m_subtitleLabel->setVisible(true);
        m_updateCard->setVisible(false);
        m_actionButton->setEnabled(!m_status->isRequestPending());

        switch (state) {
        case update::State::Idle:
            m_headerIcon->set("update.svg", Theme::current().highlight);
            m_statusLabel->setText(tr("Check for updates"));
            m_subtitleLabel->setText(lastCheckStr);
            m_actionButton->setText(tr("Check for updates"));
            break;

        case update::State::UpToDate:
            m_headerIcon->set("update.svg", Color::rgb(0x34C759));
            m_statusLabel->setText(tr("You're up to date"));
            m_subtitleLabel->setText(lastCheckStr);
            m_actionButton->setText(tr("Check for updates"));
            break;

        case update::State::Checking:
            m_headerIcon->set("update.svg", Color::rgb(0x0067FF));
            m_statusLabel->setText(tr("Checking for updates"));
            m_subtitleLabel->setVisible(false);
            m_progressBar->setVisible(true);
            m_progressBar->setIndeterminate(true);
            m_actionButton->setText(tr("Check for updates"));
            m_actionButton->setEnabled(false);
            break;

        case update::State::SystemUpgrade:
        case update::State::SoftwareUpdate: {
            m_headerIcon->set("update.svg", Color::rgb(0x0067FF));
            m_statusLabel->setText(tr("Updates available"));
            m_subtitleLabel->setText(lastCheckStr);
            m_actionButton->setText(tr("Update now"));

            // Fill update card
            const bool isSystem = (state == update::State::SystemUpgrade);
            const std::string version = m_status->newVersion();
            const std::string title = isSystem
                ? format(tr("System update | Version: {version}"), {{"version", version}})
                : tr("Software update");
            const int count = m_status->upgradableCount();
            const std::string sizeStr = count > 0
                ? std::to_string(count) + " " + tr("packages")
                : std::string{};
            m_updateCard->setTitle(title);
            m_updateCard->setSizeText(sizeStr);
            m_updateCard->setBody(m_status->changelogText(), true);
            m_updateCard->setIconColor(Color::rgb(0x0067FF));
            m_updateCard->setVisible(true);
            break;
        }

        case update::State::Upgrading:
            m_headerIcon->set("update.svg", Color::rgb(0x0067FF));
            refreshProgress();
            m_actionButton->setEnabled(false);
            m_updateCard->setTitle(tr("Software update"));
            m_updateCard->setSizeText(m_status->newVersion());
            m_updateCard->setBody(m_status->changelogText(), true);
            m_updateCard->setVisible(true);
            break;

        case update::State::RestartRequired:
            m_headerIcon->set("update.svg", Color::rgb(0x34C759));
            m_statusLabel->setText(tr("Restart required"));
            m_subtitleLabel->setText(tr("Updates are installed. Restart now to finish."));
            m_actionButton->setText(tr("Restart now"));
            m_updateCard->setTitle(tr("Restart required"));
            m_updateCard->setSizeText(m_status->newVersion());
            m_updateCard->setBody(tr("The update has been installed. Restart the computer to finish."));
            m_updateCard->setIconColor(Color::rgb(0x34C759));
            m_updateCard->setVisible(true);
            break;

        case update::State::Error:
            m_headerIcon->set("update.svg", Color::rgb(0xFF3B30));
            m_statusLabel->setText(tr("Update failed!"));
            m_subtitleLabel->setText(lastCheckStr);
            m_actionButton->setText(tr("Check again"));
            // Only show the error card when there is a log to display.
            if (!m_status->errorLog().empty()) {
                m_updateCard->setTitle(tr("Failure log"));
                m_updateCard->setSizeText(
                    m_status->newVersion().empty()
                        ? std::string{}
                        : format(tr("Version: {v}"), {{"v", m_status->newVersion()}}));
                m_updateCard->setBody(m_status->errorLog());
                m_updateCard->setIconColor(Color::rgb(0xFF3B30));
                m_updateCard->setVisible(true);
            }
            break;
        }
        if (!m_status->requestError().empty()) {
            m_statusLabel->setText(tr("Update failed!"));
            m_updateCard->setTitle(tr("Failure log"));
            m_updateCard->setSizeText({});
            m_updateCard->setBody(m_status->requestError());
            m_updateCard->setVisible(true);
        }
        if (m_status->isRequestPending()) {
            m_statusLabel->setText(tr("Requesting…"));
            m_updateCard->setVisible(false);
            m_progressBar->setIndeterminate(true);
            m_progressBar->setVisible(true);
            m_actionButton->setEnabled(false);
            m_actionButton->setText(tr("Requesting…"));
        }
    }

    void refreshProgress()
    {
        if (!m_status->isAvailable() || m_status->state() != update::State::Upgrading) return;
        const int phase   = m_status->upgradePhase();
        const int percent = m_status->upgradeProgress();

        if (phase == 1) {
            m_statusLabel->setText(tr("Downloading"));
            m_subtitleLabel->setText(
                percent >= 0 ? std::to_string(percent) + "%" : std::string{});
            m_subtitleLabel->setVisible(true);
            m_progressBar->setIndeterminate(false);
            m_progressBar->setValue(percent >= 0 ? percent / 100.0 : 0.0);
            m_progressBar->setVisible(true);
            m_actionButton->setText(tr("Updating"));
        } else if (phase == 2) {
            m_statusLabel->setText(tr("Installing"));
            m_subtitleLabel->setText(
                percent >= 0 ? std::to_string(percent) + "%" : std::string{});
            m_subtitleLabel->setVisible(true);
            m_progressBar->setIndeterminate(false);
            m_progressBar->setValue(percent >= 0 ? percent / 100.0 : 0.0);
            m_progressBar->setVisible(true);
            m_actionButton->setText(tr("Installing"));
        } else {
            m_statusLabel->setText(tr("Updating"));
            m_subtitleLabel->setVisible(false);
            m_progressBar->setIndeterminate(true);
            m_progressBar->setVisible(true);
            m_actionButton->setText(tr("Updating"));
        }
    }

    // Restart through the session, which quits every application first and
    // carries on if one refuses. The update service only reports the state.
    void restartNow()
    {
        base::dbus::Connection* bus = base::dbus::Connection::session();
        if (!bus)
            return;

        base::dbus::Proxy session(bus, "org.lingmo.Session", "/org/lingmo/Session",
                                  "org.lingmo.Session");
        session.setTimeout(std::chrono::seconds(5));
        m_restartCall = session.callAsync("reboot", [this](const base::dbus::Reply& reply) {
            if (!reply.isError())
                return;
            m_subtitleLabel->setText(tr("Could not restart the session."));
            m_subtitleLabel->setVisible(true);
        });
    }

    void onActionClicked()
    {
        if (!m_status->isAvailable() || m_status->isRequestPending()) return;
        const update::State state = m_status->state();
        if (state == update::State::RestartRequired) {
            restartNow();
        } else if (state == update::State::SystemUpgrade || state == update::State::SoftwareUpdate) {
            // Close once the service reports that the upgrade has started, so
            // a refused request stays visible with its error.
            m_closeAfterUpgradeStart = true;
            m_status->startUpgrade();
        } else {
            m_status->checkForUpdate();
        }
    }

    std::shared_ptr<update::UpdateStatus> m_status;
    base::dbus::PendingCall m_restartCall;
    bool m_closeAfterUpgradeStart = false;
    std::vector<base::Subscription> m_subscriptions;

    Card*           m_headerCard;
    RowIcon*        m_headerIcon;
    Label*          m_statusLabel;
    ProgressBar*    m_progressBar;
    SecondaryLabel* m_subtitleLabel;
    Button*         m_actionButton;
    UpdateInfoCard* m_updateCard;
};

} // namespace

static const PageRegistration registration("update", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<UpdatePage>(window);
});

} // namespace settings
