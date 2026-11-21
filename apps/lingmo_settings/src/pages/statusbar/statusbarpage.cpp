// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>

#include "base/i18n.h"
#include "base/settings.h"
#include "base/paths.h"
#include "widgets/choicerow.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/rowicon.h"
#include "pages/statusbar/statusbarsettings.h"
#include "ui/cell_layout.h"
#include "ui/layout.h"
#include "widgets/card.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class StatusBarPage : public Page
{
public:
    explicit StatusBarPage(SettingsWindow& window)
        : Page(window, tr("Menu Bar"))
    {
        auto* overview=content()->addChild<Card>(Card::Style::List);
        auto* material=overview->addChild<ChoiceRow>(tr("Menu bar background"));
        m_clear=material->options()->addChild<ImageSelectionItem>(std::make_unique<ShellPreview>(ShellPreview::Style::MenuClear),tr("Clear"),SizeF{112,72},0,ImageSelectionItem::PreviewFit::Cover);
        m_tinted=material->options()->addChild<ImageSelectionItem>(std::make_unique<ShellPreview>(ShellPreview::Style::MenuTinted),tr("Tinted"),SizeF{112,72},0,ImageSelectionItem::PreviewFit::Cover);
        m_clear->onClicked=[this]{m_settings.setOpacity(.25);m_transparency->setValue(75);updatePreviews();};
        m_tinted->onClicked=[this]{m_settings.setOpacity(.8);m_transparency->setValue(20);updatePreviews();};
        auto* controls=content()->addChild<Section>(tr("Menu bar controls"));
        controls->addChild<Footnote>(tr("App menus appear on the left. System controls and Notification Center are on the right."));
        auto* items=controls->addChild<Card>(Card::Style::List);
        auto navigation=[&window,items](const char* text,const char* glyph,const char* page){
            auto* item=items->addChild<NavigationItem>(text);item->setLeading<RowIcon>(glyph,Theme::current().highlight);
            item->onClicked=[&window,id=std::string(page)]{window.switchToPage(id);};
        };
        navigation(tr("Clock options"),"datetime.svg","datetime");
        navigation(tr("Battery options"),"battery.svg","battery");
        navigation(tr("Network"),"network.svg","network");
        navigation(tr("Sound"),"sound.svg","sound");
        navigation(tr("Notification Center"),"notifications.svg","notifications");
        auto* percentage=items->addChild<SwitchRow>(tr("Show battery percentage"));
        percentage->setName("statusbar:battery-percentage");
        percentage->setChecked(Lingmo::base::Settings(Lingmo::base::paths::lingmoConfig("shell")).boolValue("Statusbar/BatteryPercentage",false));
        percentage->onToggled=[](bool enabled){Lingmo::base::dbus::Proxy(Lingmo::base::dbus::Connection::session(),"org.lingmo.Statusbar","/Statusbar","org.lingmo.Statusbar").send("setBatteryPercentage",enabled);};

        Section* appearance = content()->addChild<Section>(tr("Background details"));
        Card* card = appearance->addChild<Card>(Card::Style::List);

        SliderRow* blur = card->addChild<SliderRow>(tr("Blur"));
        blur->setName("statusbar:blur");
        blur->setCaptions(tr("Off"), tr("Strong"));
        // Logical pixels of Gaussian sigma, the unit the compositor takes.
        blur->setRange(0, StatusBarSettings::kMaxBlurSigma);
        blur->setStepSize(1);
        blur->setValue(m_settings.blurSigma());
        // The bar reports no changes back, so the row learns the new value
        // here; otherwise it returns to the old one once the move settles.
        blur->onMoved = [this, blur](double value) {
            m_settings.setBlurSigma(value);
            blur->setValue(m_settings.blurSigma());
        };

        SliderRow* transparency = card->addChild<SliderRow>(tr("Transparency"));
        m_transparency=transparency;
        transparency->setName("statusbar:transparency");
        transparency->setCaptions(tr("More solid"), tr("More clear"));
        transparency->setRange(0, 100);
        transparency->setStepSize(1);
        transparency->setValue((1.0 - m_settings.opacity()) * 100.0);
        transparency->onMoved = [this, transparency](double value) {
            m_settings.setOpacity(1.0 - value / 100.0);
            transparency->setValue((1.0 - m_settings.opacity()) * 100.0);
            updatePreviews();
        };
        updatePreviews();
    }

private:
    void updatePreviews(){m_clear->setChecked(m_settings.opacity()<.5);m_tinted->setChecked(m_settings.opacity()>=.5);}
    ImageSelectionItem* m_clear=nullptr;
    ImageSelectionItem* m_tinted=nullptr;
    SliderRow* m_transparency=nullptr;
    StatusBarSettings m_settings;
};

} // namespace

static const PageRegistration registration("statusbar", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<StatusBarPage>(window);
});

} // namespace settings
