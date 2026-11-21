// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <algorithm>
#include <memory>

#include "base/dbus.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/label.h"
#include "ui/text_field.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using Lingmo::i18n::tr;

namespace settings {

namespace {

// The desktop's widgets live in the file manager's desktop window; this asks
// its D-Bus service to open the gallery there.
void openWidgetGallery()
{
    dbus::Proxy(dbus::Connection::session(), "org.lingmo.FileManager", "/FileManager",
                "org.lingmo.FileManager")
            .send("EditWidgets");
}

void saveWeatherCity(const std::string& city)
{
    base::Settings settings(base::paths::lingmoConfig("filemanager"));
    if (city.empty())
        settings.remove("Weather/City");
    else
        settings.setValue("Weather/City", city);
    settings.sync();
    dbus::Proxy(dbus::Connection::session(), "org.lingmo.FileManager", "/FileManager",
                "org.lingmo.FileManager")
            .send("RefreshWidgets");
}

class DesktopWidgetsPage : public Page
{
public:
    explicit DesktopWidgetsPage(SettingsWindow& window)
        : Page(window, tr("Desktop Widgets"))
    {
        Card* card = content()->addChild<Card>();
        card->setLayoutHints({.fillWidth = true});
        Lingmo::ui::Label* description = card->addChild<Lingmo::ui::Label>(
                tr("Choose from twelve widgets, select a size, and place them on your desktop or in Notification Center."));
        description->setWrap(true);
        description->setLayoutHints({.fillWidth = true});

        Section* appearance = content()->addChild<Section>(tr("Widget appearance"));
        Card* appearanceCard = appearance->addChild<Card>(Card::Style::List);
        base::Settings config(base::paths::lingmoConfig("filemanager"));
        auto* visible = appearanceCard->addChild<SwitchRow>(tr("Show widgets on desktop"));
        visible->setName("desktopwidgets:visible");
        visible->setChecked(config.boolValue("DesktopWidgets/visible", true));
        visible->onToggled = [](bool value) {
            base::Settings config(base::paths::lingmoConfig("filemanager"));
            config.setValue("DesktopWidgets/visible", value); config.sync();
            dbus::Proxy(dbus::Connection::session(), "org.lingmo.FileManager", "/FileManager", "org.lingmo.FileManager").send("RefreshWidgets");
        };
        auto* inCenter = appearanceCard->addChild<SwitchRow>(tr("Show widgets in Notification Center"));
        inCenter->setChecked(config.boolValue("NotificationWidgets/visible", true));
        inCenter->onToggled = [](bool value) {
            base::Settings config(base::paths::lingmoConfig("filemanager"));
            config.setValue("NotificationWidgets/visible", value);config.sync();
        };
        auto* styleRow = appearanceCard->addChild<ListRow>(tr("Widget color style"));
        styleRow->setInteractive(false);
        auto* style = styleRow->addContent<RowComboBox>(std::vector<std::string>{tr("Automatic"), tr("Monochrome"), tr("Full Color")});
        style->setName("desktopwidgets:color-style");
        style->setCurrentIndex(std::clamp(int(config.intValue("DesktopWidgets/colorMode", 0)), 0, 2));
        style->onActivated = [](int value) {
            base::Settings config(base::paths::lingmoConfig("filemanager"));
            config.setValue("DesktopWidgets/colorMode", value); config.sync();
            dbus::Proxy(dbus::Connection::session(), "org.lingmo.FileManager", "/FileManager", "org.lingmo.FileManager").send("RefreshWidgets");
        };
        appearance->addChild<Footnote>(tr("Automatic uses monochrome when another window is active. Editing widgets restores color. Hidden widgets keep their layout and content."));

        Card* weather = content()->addChild<Card>(Card::Style::List);
        ListRow* cityRow = weather->addChild<ListRow>(tr("Weather city"));
        cityRow->setHighlightOnPress(false);
        Lingmo::ui::TextField* city = cityRow->addContent<Lingmo::ui::TextField>();
        city->setName("desktopwidgets:weather-city");
        city->setPlaceholder(tr("City name"));
        city->setText(base::Settings(base::paths::lingmoConfig("filemanager")).stringValue("Weather/City"));
        city->onEditingFinished = [city] { saveWeatherCity(city->text()); };
        city->onAccepted = [city] { saveWeatherCity(city->text()); };

        // Icons section: new icon position
        const int currentCorner = int(
                base::Settings(base::paths::lingmoConfig("filemanager")).intValue("Desktop/iconCorner", 0));
        Section* iconsSection = content()->addChild<Section>(tr("Icons"));
        Card* iconsCard = iconsSection->addChild<Card>(Card::Style::List);
        ListRow* cornerRow = iconsCard->addChild<ListRow>(tr("New icon position"));
        cornerRow->setHighlightOnPress(false);
        cornerRow->setInteractive(false);
        RowComboBox* cornerCombo = cornerRow->addContent<RowComboBox>(
                std::vector<std::string>{tr("Top Right"), tr("Top Left"), tr("Bottom Left"), tr("Bottom Right")});
        cornerCombo->setCurrentIndex(currentCorner);
        cornerCombo->onActivated = [](int index) {
            dbus::Proxy(dbus::Connection::session(), "org.lingmo.FileManager", "/FileManager",
                        "org.lingmo.FileManager")
                    .send("SetIconCorner", int32_t(index));
        };

        StandardButton* edit = content()->addChild<StandardButton>(tr("Edit Widgets…"));
        edit->setName("desktopwidgets:edit");
        edit->onClicked = [] { openWidgetGallery(); };

        auto* editCenter = content()->addChild<StandardButton>(tr("Edit Notification Widgets…"));
        editCenter->onClicked=[] { dbus::Proxy(dbus::Connection::session(),"org.lingmo.FileManager","/FileManager","org.lingmo.FileManager").send("EditNotificationWidgets"); };
        content()->addChild<Footnote>(tr("Notification Center has its own widget layout. Changes appear the next time it opens."));

        content()->addChild<Footnote>(
                tr("The weather card looks the city up with Open-Meteo. A widget's place is saved for the "
                   "display it is on; right-click a card to remove it or change its size."));
    }
};

} // namespace

static const PageRegistration registration("desktopwidgets", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DesktopWidgetsPage>(window);
});

} // namespace settings
