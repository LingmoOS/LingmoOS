// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>

#include "base/i18n.h"
#include "pages/defaultapps/defaultapplications.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class DefaultAppsPage : public Page
{
public:
    explicit DefaultAppsPage(SettingsWindow& window)
        : Page(window, tr("Default Applications"))
    {
        Card* card = content()->addChild<Card>(Card::Style::List);
        const std::array<const char*, DefaultApplications::KindCount> titles = {tr("Web browser"), tr("Email client"),
                                                                               tr("Terminal"), tr("Video player")};
        for (int kind = 0; kind < DefaultApplications::KindCount; ++kind) {
            ListRow* row = card->addChild<ListRow>(titles[size_t(kind)]);
            row->setInteractive(false);
            RowComboBox* box = row->addContent<RowComboBox>();
            box->onActivated = [this, kind](int index) {
                m_applications.setDefault(DefaultApplications::Kind(kind), index);
            };
            m_rows[size_t(kind)] = {row, box};
        }
        m_subscription = m_applications.changed.connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        for (int kind = 0; kind < DefaultApplications::KindCount; ++kind) {
            const auto& list = m_applications.applications(DefaultApplications::Kind(kind));
            auto [row, box] = m_rows[size_t(kind)];
            std::vector<RowComboBox::Item> items;
            for (const auto& application : list)
                items.push_back({application.name, application.icon});
            box->setItems(std::move(items));
            const int index = m_applications.defaultIndex(DefaultApplications::Kind(kind));
            box->setCurrentIndex(index >= 0 ? index : (list.empty() ? -1 : 0));
            box->setDisplayText(list.empty() ? std::optional<std::string>(tr("None")) : std::nullopt);
            row->setEnabled(!list.empty());
        }
    }

    DefaultApplications m_applications;
    Lingmo::base::Subscription m_subscription;
    std::array<std::pair<ListRow*, RowComboBox*>, DefaultApplications::KindCount> m_rows;
};

} // namespace

static const PageRegistration registration("defaultapps", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DefaultAppsPage>(window);
});

} // namespace settings
