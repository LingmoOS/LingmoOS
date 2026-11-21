// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/sliderrow.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/i18n.h"
#include "base/timer.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/slider.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::trc;

namespace settings {

// FishUI's slider handle, and how far it overhangs the visible track's ends.
static constexpr double kHandle = 22;
static constexpr double kTrack = 4;
static constexpr double kTrackInset = (kHandle - kTrack) / 2;
static constexpr int kMeterSegments = 15;
static constexpr double kMeterDot = 8;
static constexpr double kTickHeight = 6;
static constexpr double kCaptionPixelSize = 11;

// Captions under a slider, flush with the two ends of its track, each under
// a tick. A caption longer than "Left" or "Right" keeps its tick where
// theirs sits.
class SliderCaptions : public View
{
public:
    SliderCaptions()
    {
        m_left = addChild<Label>();
        m_right = addChild<Label>();
        m_leftReference = addChild<Label>(trc("balance", "Left"));
        m_rightReference = addChild<Label>(trc("balance", "Right"));
        for (Label* label : {m_left, m_right, m_leftReference, m_rightReference}) {
            label->setFontSize(kCaptionPixelSize);
            // Sized to their text, which rounding must not cut.
            label->setElide(Label::Elide::None);
        }
        m_leftReference->setVisible(false);
        m_rightReference->setVisible(false);
    }

    void setTexts(std::string left, std::string right)
    {
        m_left->setText(std::move(left));
        m_right->setText(std::move(right));
        setVisible(!m_left->text().empty() || !m_right->text().empty());
        requestLayout();
    }

    SizeF sizeHint() const override
    {
        return {0, kTickHeight + metrics::xxs + m_left->sizeHint().height};
    }

protected:
    void layout() override
    {
        const Color color = Theme::current().disabledText;
        m_left->setColor(color);
        m_right->setColor(color);
        const SizeF left = m_left->sizeHint();
        const SizeF right = m_right->sizeHint();
        const double top = kTickHeight + metrics::xxs;
        m_left->setGeometry({0, top, left.width, left.height});
        m_right->setGeometry({std::round(geometry().width - right.width), top, right.width, right.height});
    }

    void paint(Painter& painter) override
    {
        const Color tick = color(ColorId::SliderTickMark);
        if (!m_left->text().empty()) {
            const double span = std::min(m_left->sizeHint().width, m_leftReference->sizeHint().width);
            painter.drawRoundedRect({std::round((span - 1) / 2), 0, 1, kTickHeight}, {}, tick);
        }
        if (!m_right->text().empty()) {
            const double span = std::min(m_right->sizeHint().width, m_rightReference->sizeHint().width);
            const double x = std::round(std::round(geometry().width - span) + (span - 1) / 2);
            painter.drawRoundedRect({x, 0, 1, kTickHeight}, {}, tick);
        }
    }

private:
    Label* m_left;
    Label* m_right;
    Label* m_leftReference;
    Label* m_rightReference;
};

// Fifteen dots on the track's span, lit up to the level.
class LevelMeter : public View
{
public:
    void setLevel(double level)
    {
        m_level = level;
        requestPaint();
    }

    SizeF sizeHint() const override { return {0, kMeterDot}; }

protected:
    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const int lit = int(std::round(std::max(0.0, m_level) * kMeterSegments));
        const double spacing = (geometry().width - kMeterSegments * kMeterDot) / (kMeterSegments - 1);
        const double y = (geometry().height - kMeterDot) / 2;
        for (int i = 0; i < kMeterSegments; ++i) {
            const RectF dot{i * (kMeterDot + spacing), y, kMeterDot, kMeterDot};
            if (i < lit)
                painter.drawSmoothRectangle(dot, kMeterDot / 2, theme.highlight);
            else
                painter.drawSmoothRectangle(dot, kMeterDot / 2, theme.disabledText.withAlpha(0.3));
        }
    }

private:
    double m_level = -1;
};

// The controls span half the row, the column other pages give their
// controls: the icons' outer edges, a bare slider's track, the meter's dots
// and the captions all meet its ends.
class SliderControls : public View
{
public:
    SliderControls()
    {
        m_leftIcon = addChild<Icon>();
        m_rightIcon = addChild<Icon>();
        m_leftIcon->setVisible(false);
        m_rightIcon->setVisible(false);
        m_slider = addChild<Slider>();
        m_meter = addChild<LevelMeter>();
        m_meter->setVisible(false);
        m_captions = addChild<SliderCaptions>();
        m_captions->setVisible(false);
    }

    Slider* slider() const { return m_slider; }
    SliderCaptions* captions() const { return m_captions; }

    void setIcons(std::string left, std::string right)
    {
        m_leftIcon->setVisible(!left.empty());
        m_rightIcon->setVisible(!right.empty());
        m_leftIcon->setIconName(std::move(left));
        m_rightIcon->setIconName(std::move(right));
        requestLayout();
    }

    void setMeterLevel(double level)
    {
        m_meter->setLevel(level);
        m_meter->setVisible(level >= 0);
        m_slider->setVisible(level < 0);
    }

    // The captions make the row too tall for the plain row padding to look even.
    double verticalMargin() const { return m_captions->isVisible() ? metrics::m : 0; }
    double sliderHeight() const { return m_slider->sizeHint().height; }

    SizeF sizeHint() const override
    {
        double height = verticalMargin() * 2 + sliderHeight();
        if (m_captions->isVisible())
            height += m_captions->sizeHint().height;
        return {0, height};
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double top = verticalMargin();
        const double height = sliderHeight();
        const double iconSize = 16;
        const double iconY = top + (height - iconSize) / 2;
        // The speaker glyphs start 1px inside their box but end at its edge.
        m_leftIcon->setGeometry({-1, iconY, iconSize, iconSize});
        m_rightIcon->setGeometry({width - iconSize, iconY, iconSize, iconSize});
        const double left = m_leftIcon->isVisible() ? -1 + iconSize + metrics::s : 0;
        const double right = m_rightIcon->isVisible() ? width - iconSize - metrics::s : width;
        // The handle overhangs the track's ends.
        m_slider->setGeometry({left - kTrackInset, top, right - left + 2 * kTrackInset, height});
        m_meter->setGeometry({0, top, width, height});
        if (m_captions->isVisible())
            m_captions->setGeometry({0, top + height, width, m_captions->sizeHint().height});
    }

private:
    Icon* m_leftIcon;
    Icon* m_rightIcon;
    Slider* m_slider;
    LevelMeter* m_meter;
    SliderCaptions* m_captions;
};

RowSlider::RowSlider()
{
    m_slider = addChild<Slider>();
    setLayoutHints({.fillWidth = true});
}

SizeF RowSlider::sizeHint() const
{
    const SizeF slider = m_slider->sizeHint();
    return {slider.width - 2 * kTrackInset, slider.height};
}

void RowSlider::layout()
{
    m_slider->setGeometry({-kTrackInset, 0, geometry().width + 2 * kTrackInset, geometry().height});
}

SliderRow::SliderRow(std::string text)
    : ListRow(std::move(text))
    , m_settle(std::make_unique<Lingmo::base::Timer>())
{
    setInteractive(false);
    setMinimumRowHeight(metrics::controlRowHeight);
    m_controls = addContent<SliderControls>();
    m_settle->setSingleShot(true);
    m_settle->onTimeout = [this] {
        m_settling = false;
        if (!isPressed())
            m_controls->slider()->setValue(m_value);
    };
    m_controls->slider()->onMoved = [this](double value) {
        m_settling = true;
        m_settle->start(std::chrono::milliseconds(300));
        if (onMoved)
            onMoved(value);
    };
    m_controls->slider()->onPressedChanged = [this] {
        // Released, the handle takes the setting's value unless a change is going out.
        if (!isPressed() && !m_settling)
            m_controls->slider()->setValue(m_value);
        if (onPressedChanged)
            onPressedChanged();
    };
}

SliderRow::~SliderRow() = default;

bool SliderRow::isPressed() const
{
    return m_controls->slider()->isPressed();
}

void SliderRow::setRange(double from, double to)
{
    m_controls->slider()->setRange(from, to);
    m_controls->slider()->setValue(m_value);
}

void SliderRow::setStepSize(double step)
{
    m_controls->slider()->setStepSize(step);
    requestLayout();
}

void SliderRow::setStepped(bool stepped)
{
    m_controls->slider()->setStepped(stepped);
    requestLayout();
}

void SliderRow::setValue(double value)
{
    m_value = value;
    if (!m_settling && !isPressed())
        m_controls->slider()->setValue(value);
}

double SliderRow::value() const
{
    return m_controls->slider()->value();
}

void SliderRow::setSliderEnabled(bool enabled)
{
    m_controls->slider()->setEnabled(enabled);
}

void SliderRow::setCaptions(std::string left, std::string right)
{
    m_controls->captions()->setTexts(std::move(left), std::move(right));
    // Centres the label on the slider rather than on the slider and its captions.
    setLabelBottomMargin(m_controls->captions()->isVisible() ? m_controls->captions()->sizeHint().height : 0);
}

void SliderRow::setIcons(std::string left, std::string right)
{
    m_controls->setIcons(std::move(left), std::move(right));
}

void SliderRow::setMeterLevel(double level)
{
    m_controls->setMeterLevel(level);
}

Slider* SliderRow::slider() const
{
    return m_controls->slider();
}

void SliderRow::layout()
{
    // Half the row's content width.
    const double width = std::round((geometry().width - 2 * metrics::cardHorizontalPadding) / 2);
    if (m_controls->layoutHints().preferredWidth != width)
        m_controls->setLayoutHints({.preferredWidth = width});
    ListRow::layout();
}

} // namespace settings
