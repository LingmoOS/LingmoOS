// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/todowidget.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "base/i18n.h"
#include "base/event_loop.h"
#include "base/json.h"
#include "base/paths.h"
#include "base/settings.h"
#include "core/config.h"
#include "ui/button.h"
#include "ui/color_id.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/text_field.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }
using Lingmo::base::json::Value;

namespace fm {


// ── ItemRow ───────────────────────────────────────────────────────────────────

class TodoWidget::ItemRow : public ui::View
{
public:
    ItemRow(TodoWidget* owner, size_t index, const TodoWidget::Item& item)
        : m_owner(owner)
        , m_index(index)
        , m_done(item.done)
        , m_text(item.text)
    {
        setCursorShape(ui::CursorShape::Pointer);
        m_label = addChild<ui::Label>();
        m_label->setText(m_text);
        m_label->setElide(ui::Label::Elide::Right);
        m_label->setFontSize(13);
    }

    bool acceptsPointer() const override { return true; }

    ui::SizeF sizeHint() const override { return {0, 36}; }

    // Update the done state in place instead of a full rebuild, so the
    // strikethrough and checkmark reflect the toggle without recreating rows.
    void setDone(bool done)
    {
        if (m_done == done)
            return;
        m_done = done;
        requestLayout(); requestPaint();
    }

    void paint(ui::Painter& painter) override
    {
        const ui::RectF cell = localRect();
        if (cell.width <= 0)
            return;

        const ui::Theme& theme = widgetTheme(this);

        // Hover highlight
        if (m_hovered)
            painter.fillRoundedRect(cell, 6, theme.text.withAlpha(0.07));

        // Circle indicator
        const double circleSize = 14;
        const double circleX = cell.x + 4;
        const double circleY = cell.y + (cell.height - circleSize) / 2.0;
        const ui::RectF circleRect{circleX, circleY, circleSize, circleSize};

        if (m_done) {
            painter.fillEllipse(circleRect, theme.highlight);
            // Checkmark inside circle
            const double cx = circleRect.x + circleRect.width / 2.0;
            const double cy = circleRect.y + circleRect.height / 2.0;
            painter.drawLine({cx - 3.5, cy}, {cx - 1.0, cy + 2.5},
                             ui::Color{255, 255, 255, 230}, 1.5);
            painter.drawLine({cx - 1.0, cy + 2.5}, {cx + 3.5, cy - 3.0},
                             ui::Color{255, 255, 255, 230}, 1.5);
        } else {
            // Quiet outlined circle
            painter.fillRoundedRect(circleRect, circleSize / 2,
                                     theme.text.withAlpha(0.06));
            painter.drawRoundedRect(circleRect,
                                    {circleSize / 2, 0, ui::Corners::Circular},
                                    ui::Color {0, 0, 0, 0}, 1.5,
                                    theme.text.withAlpha(0.30));
        }

        // Strikethrough line if done
        if (m_done) {
            const double labelX = cell.x + 4 + circleSize + 8;
            const double labelW = std::max(0.0, cell.width - labelX - 4);
            const double midY   = std::round(cell.y + cell.height / 2.0);
            if (labelW > 0)
                painter.drawHorizontalLine(labelX, midY, labelW,
                                           theme.text.withAlpha(0.35), 1.0);
        }
    }

    void layout() override
    {
        const ui::RectF cell = localRect();
        const double circleSize = 14;
        const double labelX = cell.x + 4 + circleSize + 8;
        const double labelW = std::max(0.0, cell.width - labelX - 4);
        const double labelH = std::max(0.0, m_label->sizeHint().height);
        m_label->setGeometry({labelX, cell.y + (cell.height - labelH) / 2.0, labelW, labelH});
        m_label->setColor(ui::ColorRecipe(
                widgetTheme(this).text.withAlpha(m_done ? 0.40 : 0.92)));
    }

protected:
    void pointerEnter() override { m_hovered = true; requestPaint(); }
    void pointerLeave() override { m_hovered = false; requestPaint(); }
    void hoverChanged() override { requestPaint(); }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        if (event.button==ui::MouseButton::Left && localRect().contains(event.position)) {
            auto* owner=m_owner;const auto index=m_index;
            base::EventLoop::current()->post([owner,index,alive=owner->m_alive]{if(*alive)owner->toggleItem(index);});
        }
    }

private:
    TodoWidget* m_owner;
    size_t      m_index;
    bool        m_done;
    std::string m_text;
    bool        m_hovered = false;
    ui::Label*  m_label   = nullptr;
};


// ── TodoWidget ────────────────────────────────────────────────────────────────

TodoWidget::TodoWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Todo, size)
{
    setName("todo-widget");

    // Count label (used in Small mode; also visible in Medium/Large when empty)
    m_countLabel = addChild<ui::Label>();
    m_countLabel->setTextAlignment(ui::Label::TextAlignment::Center);
    m_countLabel->setFontSize(28);
    m_countLabel->setWeight(300);

    // Scroll + list (Medium/Large)
    m_scroll = addChild<ui::ScrollView>();
    m_list   = m_scroll->setContent<ui::Column>();

    // Input row (Large only)
    m_addField  = addChild<ui::TextField>();
    m_addButton = addChild<ui::Button>(tr("Add"));

    m_addField->setName("todo-input");m_addButton->setName("todo-add");
    m_addField->setPlaceholder(tr("New task…"));
    m_addField->onAccepted = [this] {
        const std::string t = m_addField->text();
        if (!t.empty()) {
            addItem(t);
            m_addField->setText({});
        }
    };
    m_addButton->onClicked = [this] {
        const std::string t = m_addField->text();
        if (!t.empty()) {
            addItem(t);
            m_addField->setText({});
        }
    };

    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(std::chrono::milliseconds(400));
    m_saveTimer.onTimeout = [this] { save(); };
}

TodoWidget::~TodoWidget()
{
    *m_alive = false;
    m_saveTimer.stop();
    save();
}

// ── persistence ───────────────────────────────────────────────────────────────

std::string TodoWidget::dataPath() const
{
    const std::string hash = std::to_string(std::hash<std::string>{}(instanceId()));
    return base::paths::dataHome() + "/lingmo/filemanager/widget-todo/" + hash + ".json";
}

void TodoWidget::load()
{
    m_saveTimer.stop();
    m_loaded = false;
    m_items.clear();

    if (!instanceId().empty()) {
        std::ifstream file(dataPath(), std::ios::binary);
        if (file) {
            std::ostringstream buf;
            buf << file.rdbuf();
            const auto parsed = Value::parse(buf.str());
            if (parsed && parsed->isArray()) {
                for (const Value& v : parsed->toArray()) {
                    Item item;
                    item.text = v["text"].toString();
                    item.done = v["done"].toBool(false);
                    if (!item.text.empty())
                        m_items.push_back(std::move(item));
                }
            }
        }
    }

    m_loaded = true;
    rebuildRows();
}

void TodoWidget::save()
{
    if (!m_loaded || instanceId().empty())
        return;
    const std::string path = dataPath();
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
    if (ec)
        return;

    Value::Array array;
    array.reserve(m_items.size());
    for (const Item& item : m_items) {
        Value obj = Value::object();
        obj.insert("text", Value(item.text));
        obj.insert("done", Value(item.done));
        array.push_back(std::move(obj));
    }
    const std::string json = Value(std::move(array)).serialize();
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << json;
}

// ── data ops ──────────────────────────────────────────────────────────────────

void TodoWidget::addItem(const std::string& text)
{
    const auto first=text.find_first_not_of(" \t\r\n");
    if(first==std::string::npos)return;
    m_items.push_back({text.substr(first,text.find_last_not_of(" \t\r\n")-first+1), false});
    m_saveTimer.start();
    rebuildRows();
}

void TodoWidget::toggleItem(size_t index)
{
    if (index >= m_items.size())
        return;
    m_items[index].done = !m_items[index].done;
    m_saveTimer.start();
    rebuildRows();
}

void TodoWidget::rebuildRows()
{
    // Remove old rows from the list column
    for (ItemRow* row : m_rows)
        m_list->takeChild(row);
    m_rows.clear();

    for (size_t i = 0; i < m_items.size(); ++i) {
        ItemRow* row = m_list->addChild<ItemRow>(this, i, m_items[i]);
        row->setLayoutHints({.fillWidth=true});
        row->setName("todo-row-"+std::to_string(i));
        m_rows.push_back(row);
    }

    // Update count label
    const int remaining = static_cast<int>(
            std::count_if(m_items.begin(), m_items.end(),
                          [](const Item& it) { return !it.done; }));
    if (m_items.empty())
        m_countLabel->setText(tr("No tasks"));
    else if (remaining == 0)
        m_countLabel->setText(tr("All done!"));
    else
        m_countLabel->setText(std::to_string(remaining) + " " + tr("left"));

    requestLayout();
    requestPaint();
}

// ── layout ────────────────────────────────────────────────────────────────────

void TodoWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;

    const bool isSmall  = (size() == WidgetSize::Small);
    const bool isLarge  = (size() == WidgetSize::Large);
    const bool isMedium = (size() == WidgetSize::Medium);

    if (isSmall) {
        // Small: just the centred count label
        m_countLabel->setFontSize(28);
        m_countLabel->setWeight(300);
        m_countLabel->setVisible(true);
        m_scroll->setVisible(false);
        m_addField->setVisible(false);
        m_addButton->setVisible(false);

        const double labelH = std::max(36.0, m_countLabel->sizeHint().height);
        m_countLabel->setGeometry({content.x,
                                   content.y + (content.height - labelH) / 2.0,
                                   content.width, labelH});
        return;
    }

    // Medium / Large
    m_countLabel->setFontSize(13);m_countLabel->setWeight(500);
    m_countLabel->setVisible(m_items.empty());
    if(m_items.empty())m_countLabel->setGeometry({content.x,content.y+8,content.width,24});
    m_scroll->setVisible(true);

    if (isLarge || isMedium) {
        // Input row at the bottom
        m_addField->setVisible(true);
        m_addButton->setVisible(true);

        const double inputH   = 32;
        const double inputGap = 8;
        const double inputY   = content.y + content.height - inputH;
        const double fieldW   = content.width * 0.70;
        const double buttonW  = std::max(0.0, content.width - fieldW - inputGap);

        m_addField->setGeometry({content.x, inputY, fieldW, inputH});
        m_addButton->setGeometry({content.x + fieldW + inputGap, inputY, buttonW, inputH});

        const double scrollH = std::max(0.0, content.height - inputH - inputGap);
        m_scroll->setGeometry({content.x, content.y, content.width, scrollH});
    } else {
        // Medium: no input row
        m_addField->setVisible(false);
        m_addButton->setVisible(false);
        m_scroll->setGeometry(content);
    }
}

void TodoWidget::paintContent(ui::Painter& /*painter*/)
{
    // Background card is drawn by DesktopWidget::paint(); nothing extra here.
}

// ── retranslate ───────────────────────────────────────────────────────────────

void TodoWidget::retranslateContent()
{
    m_addField->setPlaceholder(tr("New task…"));
    m_addButton->setText(tr("Add"));
    rebuildRows();
}

// ── instanceIdChanged ─────────────────────────────────────────────────────────

void TodoWidget::instanceIdChanged()
{
    if (m_loaded) {
        m_items.clear();
        m_loaded = false;
    }
    load();
}

} // namespace fm
