// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Button;
class Column;
class Label;
class ScrollView;
class TextField;
}

namespace fm {

class TodoWidget : public DesktopWidget
{
public:
    TodoWidget(WidgetLayer& layer, WidgetSize size);
    ~TodoWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;
    void instanceIdChanged() override;

private:
    struct Item {
        std::string text;
        bool done = false;
    };

    class ItemRow;

    void load();
    void save();
    void addItem(const std::string& text);
    void toggleItem(size_t index);
    void rebuildRows();
    std::string dataPath() const;

    std::vector<Item> m_items;
    std::vector<ItemRow*> m_rows;
    ui::Label* m_countLabel = nullptr;
    ui::ScrollView* m_scroll = nullptr;
    ui::Column* m_list = nullptr;
    ui::TextField* m_addField = nullptr;
    ui::Button* m_addButton = nullptr;
    base::Timer m_saveTimer;
    bool m_loaded = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
