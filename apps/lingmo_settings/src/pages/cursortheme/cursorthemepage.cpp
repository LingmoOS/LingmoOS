// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "lingmo/appearance/appearance.h"
#include "pages/cursortheme/cursorthemes.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/slider.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "widgets/tilegrid.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::appearance::Appearance;
using Lingmo::i18n::tr;

namespace settings {

namespace {

constexpr double kIconSize = 24;

// The theme's example cursor, centred and scaled to the tile's icon size.
class CursorPreview : public ImageView
{
public:
    explicit CursorPreview(const Image& image)
    {
        setImage(image);
        setFillMode(FillMode::PreserveAspectFit);
    }

    SizeF sizeHint() const override { return {kIconSize, kIconSize}; }
};

class CursorThemePage : public Page
{
public:
    explicit CursorThemePage(SettingsWindow& window)
        : Page(window, tr("Cursor Style"))
        , m_themes(CursorTheme::installed())
    {
        Section* themes = content()->addChild<Section>(tr("Cursor theme"));
        Card* card = themes->addChild<Card>();
        TileGrid* grid = card->addChild<TileGrid>(SizeF{104 + metrics::tileSpacing, 104}, metrics::m);
        grid->setVisible(!m_themes.themes.empty());
        for (const CursorTheme& theme : m_themes.themes) {
            auto* item = grid->addChild<ImageSelectionItem>(std::make_unique<CursorPreview>(theme.preview()),
                                                            theme.name, SizeF{104, 64}, 0);
            const std::string id = theme.id;
            item->onClicked = [this, id] {
                if (themeIndex(m_appearance.cursorTheme()) != themeIndex(id))
                    m_appearance.setCursorTheme(id);
            };
            m_items.push_back(item);
        }

        Section* size = content()->addChild<Section>(tr("Cursor size"));
        Card* sizeCard = size->addChild<Card>();
        RowLayout* row = sizeCard->addChild<RowLayout>();
        row->setSpacing(metrics::rowSpacing);
        m_slider = row->addChild<RowSlider>();
        m_slider->slider()->setRange(16, 64);
        m_slider->slider()->setStepSize(8);
        m_slider->slider()->setStepped(true);
        m_slider->slider()->onMoved = [this](double value) {
            m_appearance.setCursorSize(std::clamp(int(std::round(value)), 16, 64));
        };
        m_size = row->addChild<Label>();

        m_subscription = m_appearance.changed.connect([this] { update(); });
        update();
    }

private:
    int themeIndex(const std::string& theme) const
    {
        const std::string id = theme == "default" ? m_themes.defaultTheme : theme;
        for (size_t i = 0; i < m_themes.themes.size(); ++i) {
            if (m_themes.themes[i].id == id)
                return int(i);
        }
        return -1;
    }

    void update()
    {
        const int current = themeIndex(m_appearance.cursorTheme().empty() ? "default" : m_appearance.cursorTheme());
        for (size_t i = 0; i < m_items.size(); ++i)
            m_items[i]->setChecked(int(i) == current);
        const int size = std::clamp(m_appearance.cursorSize(), 16, 64);
        m_slider->slider()->setValue(size);
        m_size->setText(std::to_string(size));
    }

    Appearance m_appearance;
    Lingmo::base::Subscription m_subscription;
    CursorTheme::List m_themes;
    std::vector<ImageSelectionItem*> m_items;
    RowSlider* m_slider;
    Label* m_size;
};

} // namespace

static const PageRegistration registration("cursortheme", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<CursorThemePage>(window);
});

} // namespace settings
