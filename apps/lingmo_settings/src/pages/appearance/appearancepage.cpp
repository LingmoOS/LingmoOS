// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <functional>

#include "base/i18n.h"
#include "base/settings.h"
#include "base/paths.h"
#include "widgets/choicerow.h"
#include "lingmo/appearance/appearance.h"
#include "ui/icon.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::appearance::Appearance;
using Lingmo::i18n::tr;

namespace settings {

namespace {

constexpr std::array<Color, 7> kAccentColors = {accent::blue, accent::red, accent::green, accent::purple,
                                                accent::pink, accent::orange, accent::grey};

// One accent colour: a dot, ringed in its own colour while chosen or pressed.
// The dot is a parallel curve of the ring, so both snap to the same circle
// and the gap is even all round.
class AccentSwatch : public View
{
public:
    explicit AccentSwatch(Color color)
        : m_color(color)
    {
        m_check = addChild<Icon>("check");
    }

    void setChecked(bool checked)
    {
        if (checked == m_checked)
            return;
        m_checked = checked;
        requestLayout();
        requestPaint();
    }

    // The ring sits smallSpacing inside the cell, the dot as much inside the ring.
    SizeF sizeHint() const override
    {
        const double side = 40;
        return {side, side};
    }
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const RectF dot = localRect().shrunk(Margins::all(8));
        const double size = dot.height * 0.6;
        m_check->setSize(size);
        m_check->setColor(theme.highlightedText);
        m_check->setVisible(m_checked);
        m_check->setGeometry({dot.x + (dot.width - size) / 2, dot.y + (dot.height - size) / 2, size, size});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const RectF ring = localRect().shrunk(Margins::all(4));
        const RoundedShape circle{ring.width / 2, 0, Corners::Circular};
        if (m_checked || m_pressed)
            painter.drawRoundedRect(ring, circle, Color{0, 0, 0, 0}, 3, m_color.withAlpha(m_pressed ? 0.6 : 0.4));
        painter.drawRoundedRect(ring.shrunk(Margins::all(4)),
                                {circle.radius, 4, Corners::Circular}, m_color);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    Color m_color;
    Icon* m_check;
    bool m_checked = false;
    bool m_pressed = false;
};

class AppearancePage : public Page
{
public:
    explicit AppearancePage(SettingsWindow& window)
        : Page(window, tr("Appearance"))
    {
        Card* modeCard = content()->addChild<Card>(Card::Style::List);
        auto* modes=modeCard->addChild<ChoiceRow>(tr("Appearance"));
        const SizeF preview{92,64};
        m_auto=modes->options()->addChild<ImageSelectionItem>(dataPath("images/auto_mode.svg"),tr("Auto"),preview);
        m_light=modes->options()->addChild<ImageSelectionItem>(dataPath("images/light_mode.svg"),tr("Light"),preview);
        m_dark=modes->options()->addChild<ImageSelectionItem>(dataPath("images/dark_mode.svg"),tr("Dark"),preview);
        m_auto->onClicked=[this]{m_appearance.setDarkModeSchedule(Appearance::SunsetToSunrise);};
        m_light->onClicked=[this]{chooseDarkMode(false);};m_dark->onClicked=[this]{chooseDarkMode(true);};
        Section* theme=content()->addChild<Section>(tr("Theme"));
        Card* accentCard=theme->addChild<Card>(Card::Style::List);
        auto* accents=accentCard->addChild<ChoiceRow>(tr("Accent color"));accents->options()->setSpacing(0);
        for(size_t i=0;i<kAccentColors.size();++i){auto* swatch=accents->options()->addChild<AccentSwatch>(kAccentColors[i]);swatch->onClicked=[this,i]{m_appearance.setAccentColor(int(i));};m_swatches.push_back(swatch);}
        Card* styleCard=theme->addChild<Card>(Card::Style::List);
        auto* styles=styleCard->addChild<ChoiceRow>(tr("Widget color style"));
        const char* names[]={tr("Automatic"),tr("Monochrome"),tr("Full Color")};
        for(int i=0;i<3;++i){
            auto* choice=styles->options()->addChild<ImageSelectionItem>(std::make_unique<ShellPreview>(i==0?ShellPreview::Style::WidgetAutomatic:i==1?ShellPreview::Style::WidgetMonochrome:ShellPreview::Style::WidgetColor),names[i],SizeF{92,64},0,ImageSelectionItem::PreviewFit::Cover);
            choice->setName("appearance:widget-style-"+std::to_string(i));
            choice->onClicked=[this,i]{Lingmo::base::Settings config(Lingmo::base::paths::lingmoConfig("filemanager"));config.setValue("DesktopWidgets/colorMode",i);config.sync();Lingmo::base::dbus::Proxy(Lingmo::base::dbus::Connection::session(),"org.lingmo.FileManager","/FileManager","org.lingmo.FileManager").send("RefreshWidgets");update();};
            m_widgetStyles.push_back(choice);
        }
        theme->addChild<Footnote>(tr("Automatic keeps widgets colorful on the active desktop and uses monochrome in the background."));
        auto* surfaces=content()->addChild<Section>(tr("Desktop surfaces"));
        auto* surfaceCard=surfaces->addChild<Card>(Card::Style::List);
        auto* menu=surfaceCard->addChild<NavigationItem>(tr("Menu Bar"));menu->onClicked=[&window]{window.switchToPage("statusbar");};
        auto* dock=surfaceCard->addChild<NavigationItem>(tr("Desktop & Dock"));dock->onClicked=[&window]{window.switchToPage("dock");};
        Section* details=content()->addChild<Section>(tr("Text and icons"));
        Card* links = details->addChild<Card>(Card::Style::List);
        const std::array<std::tuple<const char*, const char*, Color, const char*>, 3> pages = {
                std::tuple{tr("Fonts"), "fonts.svg", Color::rgb(0xFE8433), "fonts"},
                {tr("Icon Theme"), "icontheme.svg", Color::rgb(0x2EC347), "icontheme"},
                {tr("Cursor Style"), "cursor.svg", Color::rgb(0x3385FF), "cursortheme"}};
        for (const auto& [title, icon, tint, id] : pages) {
            NavigationItem* item = links->addChild<NavigationItem>(title);
            item->setLeading<RowIcon>(icon, tint);
            const std::string page = id;
            item->onClicked = [&window, page] { window.pushPage(page); };
        }

        m_subscription = m_appearance.changed.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        // The theme follows the service too; its colours decide the checks.
        update();
        Page::layout();
    }

private:
    bool autoDarkMode() const { return m_appearance.darkModeSchedule() == Appearance::SunsetToSunrise; }

    void chooseDarkMode(bool dark)
    {
        m_appearance.setDarkModeSchedule(Appearance::Never);
        m_appearance.setDarkMode(dark);
    }

    void update()
    {
        const Theme& theme = Theme::current();
        const bool automatic = autoDarkMode();
        const int widgetStyle=int(Lingmo::base::Settings(Lingmo::base::paths::lingmoConfig("filemanager")).intValue("DesktopWidgets/colorMode",0));
        for(size_t i=0;i<m_widgetStyles.size();++i)m_widgetStyles[i]->setChecked(int(i)==widgetStyle);
        m_light->setChecked(!automatic && !theme.darkMode);
        m_dark->setChecked(!automatic && theme.darkMode);
        m_auto->setChecked(automatic);
        for (size_t i = 0; i < m_swatches.size(); ++i)
            m_swatches[i]->setChecked(theme.highlight == kAccentColors[i]);
    }

    Appearance m_appearance;
    Lingmo::base::Subscription m_subscription;
    ImageSelectionItem* m_light;
    ImageSelectionItem* m_dark;
    ImageSelectionItem* m_auto;
    std::vector<AccentSwatch*> m_swatches;
    std::vector<ImageSelectionItem*> m_widgetStyles;
};

} // namespace

static const PageRegistration registration("appearance", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AppearancePage>(window);
});

} // namespace settings
