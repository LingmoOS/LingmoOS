// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>

#include "base/i18n.h"
#include "pages/mouse/mouse.h"
#include "ui/slider.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

// Ten steps, one per acceleration curve the compositor has. Speed 0 is the
// fourth and default; below it a step is a third of the range, above it a
// sixth.
static constexpr int kSteps = 9;
static constexpr int kDefaultStep = 3;

static int stepForSpeed(double speed)
{
    return int(std::round(speed < 0 ? kDefaultStep * (1 + speed) : kDefaultStep + speed * (kSteps - kDefaultStep)));
}

static double speedForStep(int step)
{
    return step < kDefaultStep ? double(step) / kDefaultStep - 1 : double(step - kDefaultStep) / (kSteps - kDefaultStep);
}

namespace {

class MousePage : public Page
{
public:
    explicit MousePage(SettingsWindow& window)
        : Page(window, tr("Mouse"))
    {
        m_card = content()->addChild<Card>(Card::Style::List);
        m_card->setName("mouse:card");
        m_leftHanded = m_card->addChild<SwitchRow>(tr("Left hand"));
        m_leftHanded->setName("mouse:leftHanded");
        m_leftHanded->onToggled = [this](bool checked) { m_mouse.setLeftHanded(checked); };
        m_naturalScroll = m_card->addChild<SwitchRow>(tr("Natural scrolling"));
        m_naturalScroll->onToggled = [this](bool checked) { m_mouse.setNaturalScroll(checked); };
        m_acceleration = m_card->addChild<SwitchRow>(tr("Mouse acceleration"));
        m_acceleration->onToggled = [this](bool checked) { m_mouse.setAcceleration(checked); };
        m_speed = m_card->addChild<SliderRow>(tr("Pointer speed"));
        m_speed->setName("mouse:speed");
        m_speed->setRange(0, kSteps);
        m_speed->setStepSize(1);
        m_speed->slider()->setSnapMode(Slider::SnapMode::SnapAlways);
        m_speed->onMoved = [this](double value) { m_mouse.setPointerAcceleration(speedForStep(int(std::round(value)))); };

        auto *locatorCard = content()->addChild<Card>(Card::Style::List);
        m_locator = locatorCard->addChild<SwitchRow>(tr("Shake pointer to locate"));
        m_locator->setName("mouse:shakeToLocate");
        m_locator->onToggled = [this](bool checked) { m_mouse.setShakeToLocate(checked); };
        m_changed = m_mouse.changed().connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        m_locator->setEnabled(m_mouse.locatorAvailable());
        m_locator->setChecked(m_mouse.shakeToLocate());
        m_card->setEnabled(m_mouse.isAvailable());
        m_leftHanded->setChecked(m_mouse.leftHanded());
        m_naturalScroll->setChecked(m_mouse.naturalScroll());
        m_acceleration->setChecked(m_mouse.acceleration());
        m_speed->setValue(stepForSpeed(m_mouse.pointerAcceleration()));
    }

    Mouse m_mouse;
    base::Subscription m_changed;
    Card* m_card;
    SwitchRow* m_leftHanded;
    SwitchRow* m_locator;
    SwitchRow* m_naturalScroll;
    SwitchRow* m_acceleration;
    SliderRow* m_speed;
};

} // namespace

static const PageRegistration registration("mouse", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<MousePage>(window);
});

} // namespace settings
