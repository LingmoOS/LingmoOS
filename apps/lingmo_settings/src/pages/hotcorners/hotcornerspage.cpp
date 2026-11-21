// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>

#include "base/i18n.h"
#include "pages/hotcorners/hotcorners.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace settings {

namespace {

// The compositor's action names, "" for none, and what the menu calls them.
const std::array<const char*, 5> kActions = {"", "toggle-overview", "toggle-show-desktop", "toggle-app-list",
                                             "lock-screen"};
const std::array<const char*, 5> kActionNames = {trNoop("None"), trNoop("Show All Windows"), trNoop("Show Desktop"),
                                                 trNoop("Launcher"), trNoop("Lock Screen")};

class HotCornersPage : public Page
{
public:
    explicit HotCornersPage(SettingsWindow& window)
        : Page(window, tr("Hot Corners"))
    {
        m_unavailable = content()->addChild<Card>();
        m_unavailableLabel = m_unavailable->addChild<Label>(
                tr("Hot corners are managed by the Lingmo compositor, which is not running."));
        m_unavailableLabel->setWrap(true);
        m_unavailableLabel->setLayoutHints({.fillWidth = true});

        m_list = content()->addChild<Card>(Card::Style::List);
        const std::array<std::pair<const char*, const char*>, 4> corners = {
                std::pair{tr("Top left"), "top-left"}, {tr("Top right"), "top-right"},
                {tr("Bottom left"), "bottom-left"}, {tr("Bottom right"), "bottom-right"}};
        std::vector<std::string> actionNames;
        for (const char* name : kActionNames)
            actionNames.push_back(tr(name));
        for (const auto& [text, corner] : corners) {
            ListRow* row = m_list->addChild<ListRow>(text);
            row->setInteractive(false);
            RowComboBox* box = row->addContent<RowComboBox>(actionNames);
            const std::string name = corner;
            // Keys held while choosing are the ones the corner will ask for.
            box->onActivated = [this, name](int index) {
                m_corners.setCorner(name, kActions[size_t(index)], index > 0 ? m_corners.heldModifiers() : 0);
            };
            m_boxes.push_back({name, box});
        }
        m_note = content()->addChild<Footnote>(
                tr("Move the pointer into a corner of the screen to run its action. Hold modifier keys while "
                   "choosing an action to make the corner work only while they are held."));
        m_subscription = m_corners.changed.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        m_unavailableLabel->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    void update()
    {
        const bool available = m_corners.isAvailable();
        m_unavailable->setVisible(!available);
        m_list->setVisible(available);
        m_note->setVisible(available);
        for (const auto& [corner, box] : m_boxes) {
            const std::string action = m_corners.action(corner);
            const auto it = std::find(kActions.begin(), kActions.end(), action);
            const int index = it == kActions.end() ? 0 : int(it - kActions.begin());
            box->setCurrentIndex(index);
            const int modifiers = m_corners.modifiers(corner);
            if (modifiers != 0 && index > 0)
                // TRANSLATORS: the keys a hot corner needs held and its action, such as "Ctrl + Alt – Lock Screen".
                box->setDisplayText(format(tr("{keys} – {action}"), {{"keys", m_corners.modifierText(modifiers)},
                                                                    {"action", tr(kActionNames[size_t(index)])}}));
            else
                box->setDisplayText(std::nullopt);
        }
    }

    HotCorners m_corners;
    Lingmo::base::Subscription m_subscription;
    Card* m_unavailable;
    Label* m_unavailableLabel;
    Card* m_list;
    Footnote* m_note;
    std::vector<std::pair<std::string, RowComboBox*>> m_boxes;
};

} // namespace

static const PageRegistration registration("hotcorners", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<HotCornersPage>(window);
});

} // namespace settings
