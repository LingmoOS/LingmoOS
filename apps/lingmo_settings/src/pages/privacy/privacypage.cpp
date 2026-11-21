// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>
#include <string>
#include <vector>

#include "base/application_registry.h"
#include "base/desktop_entry.h"
#include "base/i18n.h"
#include "base/signal.h"
#include "pages/privacy/computercontrol.h"
#include "ui/button.h"
#include "ui/cell_layout.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::base::ApplicationRegistry;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// A label in the theme's secondary text colour.
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

// An app allowed to control the computer: its name and icon over the program
// trusted, and Remove.
class AllowedAppRow : public ListRow
{
public:
    AllowedAppRow(const ComputerControl::App& app, std::function<void()> remove)
    {
        setInteractive(false);
        const auto entry = app.appId.empty() ? nullptr : ApplicationRegistry::instance().byId(app.appId);
        std::string title = entry && !entry->name.empty() ? entry->name : app.name;
        if (title.empty())
            title = app.path;
        setLeading<ThemeIcon>(entry && !entry->icon.empty() ? entry->icon : std::string("computer"), 24);
        ColumnLayout* texts = addContent<ColumnLayout>();
        texts->setSpacing(metrics::textSpacing);
        texts->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        Label* name = texts->addChild<Label>(title);
        name->setElide(Label::Elide::Right);
        name->setLayoutHints({.fillWidth = true});
        SecondaryLabel* path = texts->addChild<SecondaryLabel>(app.path);
        path->setElide(Label::Elide::Middle);
        path->setLayoutHints({.fillWidth = true});
        path->setVisible(title != app.path);
        Button* button = addContent<Button>(tr("Remove"));
        button->onClicked = std::move(remove);
    }
};

class PrivacyPage : public Page
{
public:
    explicit PrivacyPage(SettingsWindow& window)
        : Page(window, tr("Privacy"))
    {
        Section* section = content()->addChild<Section>(tr("Computer control"));
        m_unavailable = section->addChild<Card>();
        m_unavailableLabel = m_unavailable->addChild<Label>(
                tr("Computer control is managed by the Lingmo compositor, which is not running."));
        m_unavailableLabel->setWrap(true);
        m_unavailableLabel->setLayoutHints({.fillWidth = true});

        m_empty = section->addChild<Card>();
        m_emptyLabel = m_empty->addChild<Label>(tr("No apps are allowed to control this computer without asking."));
        m_emptyLabel->setWrap(true);
        m_emptyLabel->setLayoutHints({.fillWidth = true});

        m_list = section->addChild<Card>(Card::Style::List);

        m_note = content()->addChild<Footnote>(
                tr("An app you always allow can let its agent see the screen and use the pointer and keyboard "
                   "without asking first. The Stop button in the status bar still shows while it is in control."));
        m_subscription = m_control.changed.connect([this] { update(); });
        // Names and icons come once the registry has read the desktop entries.
        m_registry = ApplicationRegistry::instance().changed.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        m_unavailableLabel->setColor(Theme::current().disabledText);
        m_emptyLabel->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    void update()
    {
        const bool available = m_control.isAvailable();
        const std::vector<ComputerControl::App>& apps = m_control.allowedApps();
        m_unavailable->setVisible(!available);
        m_empty->setVisible(available && apps.empty());
        m_list->setVisible(available && !apps.empty());
        m_note->setVisible(available);

        // The rows go only once the list changed, after a Remove returned.
        for (ListRow* row : m_rows)
            m_list->takeChild(row);
        m_rows.clear();
        for (const ComputerControl::App& app : apps) {
            const std::string path = app.path;
            m_rows.push_back(m_list->addChild<AllowedAppRow>(app, [this, path] { m_control.forget(path); }));
        }
    }

    ComputerControl m_control;
    Lingmo::base::Subscription m_subscription;
    Lingmo::base::Subscription m_registry;
    Card* m_unavailable;
    Label* m_unavailableLabel;
    Card* m_empty;
    Label* m_emptyLabel;
    Card* m_list;
    Footnote* m_note;
    std::vector<ListRow*> m_rows;
};

} // namespace

static const PageRegistration registration("privacy", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<PrivacyPage>(window);
});

} // namespace settings
