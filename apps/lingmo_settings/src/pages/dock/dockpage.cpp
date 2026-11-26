// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>
#include <vector>

#include "base/i18n.h"
#include "base/settings.h"
#include "base/paths.h"
#include "widgets/choicerow.h"
#include "widgets/rowcombobox.h"
#include "widgets/rowicon.h"
#include "pages/dock/docksettings.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class DockPage : public Page
{
public:
    explicit DockPage(SettingsWindow& window)
        : Page(window, tr("Desktop & Dock"))
    {
        auto *modeSection = content()->addChild<Section>(tr("Desktop mode"));
        auto *modeCard = modeSection->addChild<Card>(Card::Style::List);
        auto *modeRow = modeCard->addChild<ListRow>(tr("Mode"));
        modeRow->setInteractive(false);
        m_mode = modeRow->addContent<RowComboBox>(std::vector<std::string>{Lingmo::i18n::trc("desktop mode", "Normal"), tr("Efficient")});
        m_mode->setName("dock:desktop-mode");
        m_mode->onActivated = [this](int index) { m_settings.setEfficientMode(index == 1); updateMode(); };
        modeSection->addChild<Footnote>(tr("Efficient mode uses a bottom taskbar with the system tray, time and notifications. The top bar and global menus are disabled."));
        m_modeError = modeSection->addChild<Footnote>(std::string());
        Card* efficientSettings = modeSection->addChild<Card>(Card::Style::List);
        m_taskbarSettings = efficientSettings->addChild<NavigationItem>(tr("Taskbar settings"));
        m_taskbarSettings->onClicked = [&window] { window.switchToPage("statusbar"); };
        m_modeSubscription = m_settings.modeChanged.connect([this] { updateMode(); });
        Card* taskbar = modeSection->addChild<Card>(Card::Style::List);
        auto* alignment = taskbar->addChild<ListRow>(tr("Taskbar alignment"));
        alignment->setInteractive(false);
        auto* alignmentBox = alignment->addContent<RowComboBox>(std::vector<std::string>{tr("Left"), tr("Center")});
        alignmentBox->setCurrentIndex(m_settings.taskbarAlignment());
        alignmentBox->onActivated = [this, alignmentBox](int index) { m_settings.setTaskbarAlignment(index); alignmentBox->setCurrentIndex(m_settings.taskbarAlignment()); };
        auto* search = taskbar->addChild<ListRow>(tr("Search box"));
        search->setInteractive(false);
        auto* searchBox = search->addContent<RowComboBox>(std::vector<std::string>{tr("Hidden"), tr("Shown")});
        searchBox->setCurrentIndex(m_settings.taskbarSearch());
        searchBox->onActivated = [this, searchBox](int index) { m_settings.setTaskbarSearch(index); searchBox->setCurrentIndex(m_settings.taskbarSearch()); };
        m_taskbarOptions = taskbar;
        Section* position = content()->addChild<Section>(tr("Dock"));
        m_normalDock = position;
        updateMode();
        Card* positions = position->addChild<Card>(Card::Style::List);
        auto* choices=positions->addChild<ChoiceRow>(tr("Position on screen"));
        auto* tiles=choices->options();
        const char* const names[] = {tr("Left"), tr("Bottom"), tr("Right")};
        const char* const pictures[] = {"images/dock_left.svg", "images/dock_bottom.svg", "images/dock_right.svg"};
        for (int i = 0; i < 3; ++i) {
            ImageSelectionItem* tile = tiles->addChild<ImageSelectionItem>(dataPath(pictures[i]), names[i],
                    SizeF{92, 64}, 2, ImageSelectionItem::PreviewFit::Cover);
            tile->onClicked = [this, i] {
                m_settings.setDirection(i);
                updateDirection();
            };
            m_directions.push_back(tile);
        }
        updateDirection();

        Card* sliders = position->addChild<Card>(Card::Style::List);
        SliderRow* size = sliders->addChild<SliderRow>(tr("Size"));
        size->setName("dock:size");
        size->setCaptions(tr("Small"), tr("Huge"));
        // The value is the icon size itself; the dock derives its paddings
        // from it (54 is the design baseline).
        size->setRange(32, 128);
        size->setStepSize(1);
        size->setValue(m_settings.iconSize());
        // The dock reports no changes back, so the row learns the new value
        // here; otherwise it returns to the old one once the move settles.
        size->onMoved = [this, size](double value) {
            m_settings.setIconSize(int(std::round(value)));
            size->setValue(m_settings.iconSize());
        };
        // The magnification: its own switch, and how large the icons become
        // under the pointer. A scale, 1 for none.
        SwitchRow* magnification = sliders->addChild<SwitchRow>(tr("Magnification"));
        magnification->setName("dock:magnification");
        magnification->setChecked(m_settings.magnification());
        SliderRow* magnificationSize = sliders->addChild<SliderRow>(tr("Maximum magnification"));
        magnificationSize->setName("dock:magnification-factor");
        magnificationSize->setCaptions(tr("Subtle"), tr("Large"));
        magnificationSize->setRange(1, 2);
        magnificationSize->setStepSize(0.05);
        magnificationSize->setValue(m_settings.magnificationFactor());
        magnificationSize->setSliderEnabled(m_settings.magnification());
        // The dock reports no changes back, so the row learns the new value
        // here; otherwise it returns to the old one once the move settles.
        magnificationSize->onMoved = [this, magnificationSize](double value) {
            m_settings.setMagnificationFactor(value);
            magnificationSize->setValue(m_settings.magnificationFactor());
        };
        // Off, there is no size to choose: the slider is disabled with it.
        magnification->onToggled = [this, magnificationSize](bool checked) {
            m_settings.setMagnification(checked);
            magnificationSize->setSliderEnabled(checked);
        };

        SliderRow* glass = sliders->addChild<SliderRow>(tr("Glass effect"));
        glass->setName("dock:glass");
        glass->setCaptions(tr("More clear"), tr("More tinted"));
        glass->setRange(0, 1);
        glass->setValue(m_settings.glassTint());
        glass->onMoved = [this, glass](double value) {
            m_settings.setGlassTint(value);
            glass->setValue(m_settings.glassTint());
        };

        Section* launch = content()->addChild<Section>(tr("Dock behavior"));
        Card* behaviour = launch->addChild<Card>(Card::Style::List);
        m_normalBehaviour = behaviour;
        SwitchRow* bounce = behaviour->addChild<SwitchRow>(tr("Bounce on launch"));
        bounce->setName("dock:bounce");
        bounce->setChecked(m_settings.bounceOnLaunch());
        bounce->onToggled = [this](bool checked) { m_settings.setBounceOnLaunch(checked); };

        auto* visibilityRow=behaviour->addChild<ListRow>(tr("Automatically hide the Dock"));visibilityRow->setInteractive(false);
        auto* visibility=visibilityRow->addContent<RowComboBox>(std::vector<std::string>{tr("Always show"),tr("Always hide"),tr("Smart hide")});
        visibility->setName("dock:visibility");visibility->setCurrentIndex(m_settings.visibility());
        visibility->onActivated=[this](int index){m_settings.setVisibility(index);};
        Card* corners = launch->addChild<Card>(Card::Style::List);
        NavigationItem* hotCorners = corners->addChild<NavigationItem>(tr("Hot Corners"));
        hotCorners->onClicked = [&window] { window.pushPage("hotcorners"); };

        Card* minimized = launch->addChild<Card>(Card::Style::List);
        m_normalMinimized = minimized;
        SwitchRow* area = minimized->addChild<SwitchRow>(tr("Minimized windows area"));
        area->setChecked(m_settings.minimizedArea());
        area->onToggled = [this](bool checked) { m_settings.setMinimizedArea(checked); };
        auto* animRow = minimized->addChild<ListRow>(tr("Minimize animation"));
        animRow->setInteractive(false);
        auto* animBox = animRow->addContent<RowComboBox>(
                std::vector<std::string>{tr("Scale"), tr("Genie Effect")});
        animBox->setName("dock:minimize-animation");
        animBox->setCurrentIndex(m_settings.minimizeStyle());
        animBox->onActivated = [this](int index) { m_settings.setMinimizeStyle(index); };
        Section* desktop=content()->addChild<Section>(tr("Desktop and widgets"));
        auto* desktopCard=desktop->addChild<Card>(Card::Style::List);
        auto* stage = desktopCard->addChild<SwitchRow>(tr("Stage Manager"));
        stage->setName("stage:enabled");
        stage->setChecked(Lingmo::base::Settings(
                                  Lingmo::base::paths::lingmoConfig("shell"))
                                  .boolValue("StageManager/Enabled", false));
        stage->onToggled=[](bool enabled) {
            Lingmo::base::Settings config(Lingmo::base::paths::lingmoConfig("shell"));
            config.setValue("StageManager/Enabled", enabled);
            config.sync();
        };
        auto* show=desktopCard->addChild<SwitchRow>(tr("Show widgets on desktop"));
        show->setChecked(Lingmo::base::Settings(Lingmo::base::paths::lingmoConfig("filemanager")).boolValue("DesktopWidgets/visible",true));
        show->onToggled=[](bool enabled){Lingmo::base::Settings config(Lingmo::base::paths::lingmoConfig("filemanager"));config.setValue("DesktopWidgets/visible",enabled);config.sync();Lingmo::base::dbus::Proxy(Lingmo::base::dbus::Connection::session(),"org.lingmo.FileManager","/FileManager","org.lingmo.FileManager").send("RefreshWidgets");};
        auto* widgets=desktopCard->addChild<NavigationItem>(tr("Widget settings"));widgets->onClicked=[&window]{window.switchToPage("desktopwidgets");};
        auto* wallpaper=desktopCard->addChild<NavigationItem>(tr("Wallpaper"));wallpaper->onClicked=[&window]{window.switchToPage("background");};
        desktop->addChild<Footnote>(tr("Arrange desktop widgets, choose their appearance, and manage the separate Notification Center layout."));
        updateMode();

    }

private:
    void updateMode()
    {
        m_mode->setCurrentIndex(m_settings.efficientMode() ? 1 : 0);
        m_mode->setEnabled(m_settings.modeAvailable() && !m_settings.modePending());
        m_modeError->setText(m_settings.modeError());
        m_modeError->setVisible(!m_settings.modeError().empty());
        if (m_taskbarSettings)
            m_taskbarSettings->setVisible(m_settings.efficientMode());
        if (m_taskbarOptions)
            m_taskbarOptions->setVisible(m_settings.efficientMode());
        if (m_normalDock) m_normalDock->setEnabled(!m_settings.efficientMode());
        if (m_normalBehaviour) m_normalBehaviour->setEnabled(!m_settings.efficientMode());
        if (m_normalMinimized) m_normalMinimized->setEnabled(!m_settings.efficientMode());
    }
    RowComboBox *m_mode = nullptr;
    Footnote *m_modeError = nullptr;
    NavigationItem *m_taskbarSettings = nullptr;
    Card *m_taskbarOptions = nullptr;
    Section *m_normalDock = nullptr;
    Card *m_normalBehaviour = nullptr;
    Card *m_normalMinimized = nullptr;
    Lingmo::base::Subscription m_modeSubscription;

    void updateDirection()
    {
        for (size_t i = 0; i < m_directions.size(); ++i)
            m_directions[i]->setChecked(m_settings.direction() == int(i));
    }

    DockSettings m_settings;
    std::vector<ImageSelectionItem*> m_directions;
};

} // namespace

static const PageRegistration registration("dock", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DockPage>(window);
});

} // namespace settings
