// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "ui/scroll_view.h"

#include <functional>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class Label;
}

namespace fm {

class PlacesModel;

// A virtual overview for computer:/// containing standard folders and mounted
// volumes. Entries open their real local URL when activated.
class ComputerView : public ui::ScrollView
{
public:
    struct Entry
    {
        std::string name;
        std::string url;
        std::string icon;
        std::string id;
        bool drive = false;
        bool ejectable = false;
        std::string filesystem;
        uint64_t used = 0;
        uint64_t total = 0;
    };

    explicit ComputerView(std::function<void(const std::string&)> open,
                          std::function<void(const Entry&)> properties = {},
                          std::function<void(const Entry&)> eject = {},
                          std::function<void(const std::string&)> notify = {},
                          std::function<void(const std::string&)> openInNewTab = {});
    ~ComputerView() override;

    void requestEject(const std::string& id);

    void retranslate();

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    class Content;
    void rebuild();
    void layoutContent();
    double contentHeight(double width) const;

    std::function<void(const std::string&)> m_open;
    std::function<void(const Entry&)> m_properties;
    std::function<void(const Entry&)> m_eject;
    std::function<void(const std::string&)> m_notify;
    std::function<void(const std::string&)> m_openInNewTab;
    std::unique_ptr<PlacesModel> m_places;
    std::vector<base::Subscription> m_subscriptions;
    ui::Label* m_foldersTitle = nullptr;
    ui::Label* m_drivesTitle = nullptr;
    Content* m_content = nullptr;
    std::vector<Entry> m_entries;
    std::vector<ui::View*> m_cards;
};

} // namespace fm
