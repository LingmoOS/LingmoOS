// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>

#include "base/i18n.h"
#include "pages/input/availability.h"
#include "pages/touchpad/touchpad.h"
#include "ui/slider.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

// libinput's speed runs from -1 to 1; eleven steps put its default in the
// middle.
static constexpr int kPointerSteps = 10;
// Ten steps doubling every third, from a quarter to twice libinput's speed;
// the default half is step 4.
static constexpr int kScrollSteps = 9;
static constexpr int kDefaultScrollStep = 3;
static constexpr double kDefaultScrollSpeed = 0.5;

namespace {

class TouchpadPage : public Page
{
public:
    explicit TouchpadPage(SettingsWindow& window)
        : Page(window, tr("Touchpad"))
    {
        m_deviceCard = content()->addChild<Card>(Card::Style::List);
        m_deviceCard->setName("touchpad:device");
        m_enabled = m_deviceCard->addChild<SwitchRow>(tr("Enable"));
        m_enabled->onToggled = [this](bool checked) { m_touchpad.setEnabled(checked); };
        m_disableWithMouse = m_deviceCard->addChild<SwitchRow>(tr("Disable when a mouse is connected"));
        m_disableWithMouse->onToggled = [this](bool checked) { m_touchpad.setDisableWithMouse(checked); };
        m_disableWhileTyping = m_deviceCard->addChild<SwitchRow>(tr("Disable while typing"));
        m_disableWhileTyping->onToggled = [this](bool checked) { m_touchpad.setDisableWhileTyping(checked); };

        m_gestureCard = content()->addChild<Card>(Card::Style::List);
        m_gestureCard->setName("touchpad:gestures");
        m_tapToClick = m_gestureCard->addChild<SwitchRow>(tr("Tap to click"));
        m_tapToClick->onToggled = [this](bool checked) { m_touchpad.setTapToClick(checked); };
        m_tapDragging = m_gestureCard->addChild<SwitchRow>(tr("Tap and drag"));
        m_tapDragging->setName("touchpad:tapDragging");
        m_tapDragging->onToggled = [this](bool checked) { m_touchpad.setTapDragging(checked); };
        m_secondaryClick = m_gestureCard->addChild<ListRow>(tr("Secondary click"));
        m_secondaryClick->setName("touchpad:secondaryClick");
        m_secondaryClick->setInteractive(false);
        m_secondaryClickMethod = m_secondaryClick->addContent<RowComboBox>(
                std::vector<std::string>{tr("Two-finger press"), tr("Bottom-right corner press")});
        m_secondaryClickMethod->onActivated = [this](int index) { m_touchpad.setTwoFingerSecondaryClick(index == 0); };
        m_naturalScroll = m_gestureCard->addChild<SwitchRow>(tr("Natural scrolling"));
        m_naturalScroll->onToggled = [this](bool checked) { m_touchpad.setNaturalScroll(checked); };
        m_pointerSpeed = m_gestureCard->addChild<SliderRow>(tr("Pointer speed"));
        m_pointerSpeed->setRange(0, kPointerSteps);
        m_pointerSpeed->setStepSize(1);
        m_pointerSpeed->slider()->setSnapMode(Slider::SnapMode::SnapAlways);
        m_pointerSpeed->onMoved = [this](double value) {
            m_touchpad.setPointerAcceleration(std::round(value) * 2 / kPointerSteps - 1);
        };
        m_scrollSpeed = m_gestureCard->addChild<SliderRow>(tr("Scroll speed"));
        m_scrollSpeed->setName("touchpad:scrollSpeed");
        m_scrollSpeed->setRange(0, kScrollSteps);
        m_scrollSpeed->setStepSize(1);
        m_scrollSpeed->slider()->setSnapMode(Slider::SnapMode::SnapAlways);
        m_scrollSpeed->onMoved = [this](double value) {
            m_touchpad.setScrollSpeed(kDefaultScrollSpeed * std::pow(2, (std::round(value) - kDefaultScrollStep) / 3));
        };

        m_changed = m_touchpad.changed().connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        const bool available = m_touchpad.isAvailable();
        const bool enabled = m_touchpad.enabled();
        m_deviceCard->setEnabled(available);
        m_enabled->setChecked(enabled);
        m_disableWithMouse->setEnabled(enabled);
        m_disableWithMouse->setChecked(m_touchpad.disableWithMouse());
        m_disableWhileTyping->setEnabled(enabled);
        m_disableWhileTyping->setChecked(m_touchpad.disableWhileTyping());

        m_gestureCard->setEnabled(available && enabled);
        m_tapToClick->setChecked(m_touchpad.tapToClick());
        // Tap and drag starts with a tap, so it needs tapping on.
        m_tapDragging->setEnabled(m_touchpad.tapToClick());
        m_tapDragging->setChecked(m_touchpad.tapDragging());
        m_secondaryClick->setVisible(m_touchpad.secondaryClickConfigurable());
        m_secondaryClickMethod->setCurrentIndex(m_touchpad.twoFingerSecondaryClick() ? 0 : 1);
        m_naturalScroll->setChecked(m_touchpad.naturalScroll());
        m_pointerSpeed->setValue(std::round((m_touchpad.pointerAcceleration() + 1) * kPointerSteps / 2));
        m_scrollSpeed->setValue(
                std::round(kDefaultScrollStep + 3 * std::log2(m_touchpad.scrollSpeed() / kDefaultScrollSpeed)));
    }

    Touchpad m_touchpad;
    base::Subscription m_changed;
    Card* m_deviceCard;
    SwitchRow* m_enabled;
    SwitchRow* m_disableWithMouse;
    SwitchRow* m_disableWhileTyping;
    Card* m_gestureCard;
    SwitchRow* m_tapToClick;
    SwitchRow* m_tapDragging;
    ListRow* m_secondaryClick;
    RowComboBox* m_secondaryClickMethod;
    SwitchRow* m_naturalScroll;
    SliderRow* m_pointerSpeed;
    SliderRow* m_scrollSpeed;
};

} // namespace

static const PageRegistration registration(
        "touchpad",
        [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<TouchpadPage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) {
            return answerOnce(Touchpad().isAvailable(), std::move(callback));
        });

} // namespace settings
