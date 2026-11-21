// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "pages/notifications/notifications.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class NotificationsPage : public Page
{
public:
    explicit NotificationsPage(SettingsWindow& window)
        : Page(window, tr("Notifications"))
    {
        Card* card = content()->addChild<Card>(Card::Style::List);
        m_doNotDisturb = card->addChild<SwitchRow>(tr("Do Not Disturb"));
        m_doNotDisturb->setChecked(m_notifications.doNotDisturb());
        m_doNotDisturb->onToggled = [this](bool checked) { m_notifications.setDoNotDisturb(checked); };
        m_subscriptions.push_back(m_notifications.doNotDisturbChanged.connect(
                [this] { m_doNotDisturb->setChecked(m_notifications.doNotDisturb()); }));

        Section* section = content()->addChild<Section>(tr("Applications"));
        m_empty = section->addChild<Card>();
        m_emptyLabel = m_empty->addChild<Label>(tr("Applications appear here after they send a notification."));
        m_emptyLabel->setWrap(true);
        m_emptyLabel->setLayoutHints({.fillWidth = true});
        m_list = section->addChild<Card>(Card::Style::List);
        m_footnote = section->addChild<Footnote>(
                tr("Notifications that skip the banner still go to the Notification Center. Turned off, "
                   "they are dropped."));
        m_subscriptions.push_back(m_notifications.applicationsChanged.connect([this] { updateApplications(); }));
        updateApplications();
    }

protected:
    void layout() override
    {
        m_emptyLabel->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    void updateApplications()
    {
        const auto& applications = m_notifications.applications();
        m_empty->setVisible(applications.empty());
        m_list->setVisible(!applications.empty());
        m_footnote->setVisible(!applications.empty());
        while (!m_list->children().empty())
            m_list->takeChild(m_list->children().front().get());
        for (const Notifications::Application& application : applications) {
            ListRow* row = m_list->addChild<ListRow>(application.name);
            row->setInteractive(false);
            row->setLeading<ThemeIcon>(application.icon.empty() ? "application-x-executable" : application.icon, 24);
            // Indexes follow Notifications::ApplicationMode.
            RowComboBox* mode = row->addContent<RowComboBox>(
                    std::vector<std::string>{tr("Banners"), tr("Notification Center only"), tr("Off")});
            mode->setCurrentIndex(application.mode);
            const std::string key = application.key;
            mode->onActivated = [this, key](int index) { m_notifications.setApplicationMode(key, index); };
        }
    }

    Notifications m_notifications;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    SwitchRow* m_doNotDisturb;
    Card* m_empty;
    Label* m_emptyLabel;
    Card* m_list;
    Footnote* m_footnote;
};

} // namespace

static const PageRegistration registration("notifications", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<NotificationsPage>(window);
});

} // namespace settings
