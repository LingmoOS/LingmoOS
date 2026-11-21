// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/locale.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/display/night_light.h"
#include "lingmo/display/screen.h"
#include "pages/display/brightness.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "ui/slider.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::Display;
using namespace Lingmo::ui;
using namespace std::chrono_literals;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trc;
using Lingmo::i18n::trn;

namespace settings {

// A time of day in the user's short time format, such as "6:00 PM".
static std::string timeOfDay(int minutes)
{
    return base::Locale::system().toString(base::ClockTime{minutes / 60, minutes % 60},
                                           base::Locale::FormatType::Short);
}

static std::string percent(double scale)
{
    // TRANSLATORS: a percentage, such as "92%".
    return format(tr("{percent}%"), {{"percent", std::to_string(int(std::lround(scale * 100)))}});
}

// A menu item marked as the best choice for the screen.
static std::string recommended(const std::string& item)
{
    return format(tr("{item} (Recommended)"), {{"item", item}});
}

namespace {

// A label in the theme's secondary text colour.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

// A row whose control spans half of it, as the Qt version's RowSlider and
// position fields did; the label takes the rest.
class HalfRow : public ListRow
{
public:
    using ListRow::ListRow;

    void setControl(View* control) { m_control = control; }

protected:
    void layout() override
    {
        const double width = std::round((geometry().width - 2 * metrics::cardHorizontalPadding) / 2);
        if (m_control && m_control->layoutHints().preferredWidth != width)
            m_control->setLayoutHints({.preferredWidth = width});
        ListRow::layout();
    }

private:
    View* m_control = nullptr;
};

// An integer field that applies on Return or when focus leaves it, keeping
// only what an integer can hold.
class NumberField : public TextField
{
public:
    NumberField()
    {
        onTextChanged = [this] {
            std::string kept;
            for (char c : text()) {
                if ((c >= '0' && c <= '9') || (c == '-' && kept.empty()))
                    kept += c;
            }
            if (kept != text())
                setText(kept);
        };
        onAccepted = [this] { finish(); };
    }

    std::function<void(int)> onEditingFinished;

protected:
    void focusOut() override
    {
        TextField::focusOut();
        finish();
    }

private:
    void finish()
    {
        try {
            const int value = std::stoi(text());
            if (value >= -32768 && value <= 32768 && onEditingFinished)
                onEditingFinished(value);
        } catch (...) {
        }
    }
};

// The half-hour steps of a day; a time set some other way shows as the nearest.
class TimeComboBox : public RowComboBox
{
public:
    TimeComboBox()
    {
        std::vector<std::string> times;
        for (int step = 0; step < 48; ++step)
            times.push_back(timeOfDay(step * 30));
        setItems(std::move(times));
        onActivated = [this](int index) {
            if (onPicked)
                onPicked(index * 30);
        };
    }

    void setMinutes(int minutes) { setCurrentIndex(int(std::lround(minutes / 30.0)) % 48); }

    std::function<void(int minutes)> onPicked;
};

class DisplayPage : public Page
{
public:
    explicit DisplayPage(SettingsWindow& window)
        : Page(window, tr("Display"))
    {
        m_screens = content()->addChild<ColumnLayout>();
        m_screens->setSpacing(metrics::sectionLabelSpacing);
        m_screens->setLayoutHints({.fillWidth = true});
        // A single screen is named; several are picked between here.
        m_screenName = m_screens->addChild<SectionLabel>();
        m_screenName->setName("display:name");
        m_screenTabs = m_screens->addChild<SegmentedControl>();
        m_screenTabs->setName("display:tabs");
        m_screenTabs->setLayoutHints({.fillWidth = true});
        m_screenTabs->onCurrentChanged = [this](int) { updateScreens(); };
        // The ListView's delegate: a layout as wide as the page, as tall as it needs.
        m_screenPage = m_screens->addChild<ColumnLayout>();
        m_screenPage->setSpacing(metrics::cardSpacing);
        m_screenPage->setLayoutHints({.fillWidth = true});
        buildModes();
        buildArrangement();

        m_errorCard = content()->addChild<Card>();
        m_errorCard->setName("display:error");
        m_error = m_errorCard->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});

        buildNightLight();

        m_brightnessTimer.setSingleShot(true);
        m_brightnessTimer.setInterval(100ms);
        m_brightnessTimer.onTimeout = [this] {
            m_brightness.setValue(int(std::lround(m_brightnessSlider->slider()->value())));
        };
        m_temperatureTimer.setSingleShot(true);
        m_temperatureTimer.setInterval(100ms);
        m_temperatureTimer.onTimeout = [this] { m_nightLight.setTemperature(m_pendingTemperature); };

        m_subscriptions.push_back(m_screen.stateChanged.connect([this] { updateScreens(); }));
        m_subscriptions.push_back(m_screen.outputModel().rowsChanged.connect([this] { updateScreens(); }));
        m_subscriptions.push_back(m_screen.outputModel().changed.connect([this] { updateScreens(); }));
        m_subscriptions.push_back(m_brightness.changed.connect([this] { updateBrightness(); }));
        m_subscriptions.push_back(m_nightLight.changed.connect([this] { updateNightLight(); }));
        updateScreens();
        updateNightLight();
    }

    ~DisplayPage() override
    {
        m_subscriptions.clear();
    }

private:
    OutputModel& outputs() { return m_screen.outputModel(); }
    int currentRow() const { return std::max(0, m_screenTabs->currentIndex()); }

    void buildModes()
    {
        Card* card = m_screenPage->addChild<Card>(Card::Style::List);
        m_brightnessRow = card->addChild<HalfRow>(tr("Brightness"));
        m_brightnessRow->setName("display:brightness");
        m_brightnessRow->setInteractive(false);
        m_brightnessRow->setMinimumRowHeight(metrics::controlRowHeight);
        m_brightnessSlider = m_brightnessRow->addContent<RowSlider>();
        m_brightnessRow->setControl(m_brightnessSlider);
        Slider* slider = m_brightnessSlider->slider();
        slider->setRange(1, 100);
        slider->setStepSize(1);
        slider->onMoved = [this](double) { m_brightnessTimer.start(); };
        slider->setValueToolTip([](double value) { return std::to_string(std::lround(value)); });

        m_resolutionRow = card->addChild<ListRow>(tr("Resolution"));
        m_resolutionRow->setInteractive(false);
        m_resolution = m_resolutionRow->addContent<RowComboBox>();
        m_resolution->setName("display:resolution");
        m_resolution->onActivated = [this](int index) { outputs().setResolutionIndex(currentRow(), index); };

        ListRow* refreshRow = card->addChild<ListRow>(tr("Refresh rate"));
        refreshRow->setInteractive(false);
        m_refreshRate = refreshRow->addContent<RowComboBox>();
        m_refreshRate->setName("display:refresh");
        m_refreshRate->onActivated = [this](int index) { outputs().setRefreshRateIndex(currentRow(), index); };

        ListRow* scaleRow = card->addChild<ListRow>(tr("Scale"));
        scaleRow->setInteractive(false);
        m_scale = scaleRow->addContent<RowComboBox>();
        m_scale->setName("display:scale");
        m_scale->onActivated = [this](int index) {
            const std::vector<double> scales = outputs().supportedScales();
            if (index >= 0 && index < int(scales.size()))
                outputs().setScale(currentRow(), scales[index]);
        };

        ListRow* rotationRow = card->addChild<ListRow>(tr("Rotation"));
        rotationRow->setInteractive(false);
        // Paired with the labels as the old rotation pictures were.
        m_rotation = rotationRow->addContent<RowComboBox>(
            std::vector<std::string>{trc("rotation", "Normal"), "90°", "180°", "270°"});
        m_rotation->setName("display:rotation");
        m_rotation->onActivated = [this](int index) {
            static const int rotations[] = {OutputModel::None, OutputModel::Left, OutputModel::Inverted,
                                            OutputModel::Right};
            if (index >= 0 && index < 4)
                outputs().setRotation(currentRow(), rotations[index]);
        };
    }

    // How a screen sits among the others.
    void buildArrangement()
    {
        m_arrangement = m_screenPage->addChild<Card>(Card::Style::List);
        m_arrangement->setName("display:arrangement");
        m_enabled = m_arrangement->addChild<SwitchRow>(tr("Enabled"));
        m_enabled->onToggled = [this](bool checked) {
            if (!outputs().setEnabled(currentRow(), checked))
                updateScreens();
        };
        // A screen stops being primary only when another one takes over, so
        // the switch turns on and never off.
        m_primary = m_arrangement->addChild<SwitchRow>(tr("Primary"));
        m_primary->onToggled = [this](bool) {
            if (!outputs().setPrimary(currentRow()))
                updateScreens();
        };
        m_modeRow = m_arrangement->addChild<ListRow>(tr("Mode"));
        m_modeRow->setInteractive(false);
        m_mode = m_modeRow->addContent<RowComboBox>();
        m_mode->onActivated = [this](int index) { outputs().setReplicationSourceIndex(currentRow(), index); };

        HalfRow* positionRow = m_arrangement->addChild<HalfRow>(tr("Position"));
        positionRow->setInteractive(false);
        positionRow->setMinimumRowHeight(metrics::controlRowHeight);
        RowLayout* fields = positionRow->addContent<RowLayout>();
        positionRow->setControl(fields);
        fields->setSpacing(metrics::controlSpacing);
        m_positionX = fields->addChild<NumberField>();
        m_positionY = fields->addChild<NumberField>();
        // Equal halves, as Layout.preferredWidth 1 on both gives.
        m_positionX->setLayoutHints({.fillWidth = true, .preferredWidth = 1});
        m_positionY->setLayoutHints({.fillWidth = true, .preferredWidth = 1});
        m_positionX->onEditingFinished = [this](int x) {
            outputs().setPosition(currentRow(), {x, outputs().position(currentRow()).y});
        };
        m_positionY->onEditingFinished = [this](int y) {
            outputs().setPosition(currentRow(), {outputs().position(currentRow()).x, y});
        };
    }

    void buildNightLight()
    {
        m_nightCard = content()->addChild<Card>(Card::Style::List);
        m_nightCard->setName("display:nightlight");
        m_nightSwitch = m_nightCard->addChild<SwitchRow>(tr("Night Light"));
        m_nightSwitch->onToggled = [this](bool checked) { m_nightLight.setEnabled(checked); };

        m_temperature = m_nightCard->addChild<SliderRow>(tr("Color temperature"));
        m_temperature->setCaptions(tr("Less warm"), tr("More warm"));
        m_temperature->setRange(0, 1);
        m_temperature->onMoved = [this](double value) {
            m_pendingTemperature = value;
            m_temperatureTimer.start();
        };
        m_temperature->onPressedChanged = [this] {
            const bool pressed = m_temperature->isPressed();
            // The last move goes out before the preview ends, or it would only be saved.
            if (!pressed && m_temperatureTimer.isActive()) {
                m_temperatureTimer.stop();
                m_nightLight.setTemperature(m_pendingTemperature);
            }
            m_nightLight.setPreview(pressed);
        };

        ListRow* scheduleRow = m_nightCard->addChild<ListRow>(tr("Schedule"));
        scheduleRow->setInteractive(false);
        m_schedule = scheduleRow->addContent<RowComboBox>(
            std::vector<std::string>{tr("Never"), tr("Sunset to sunrise"), tr("Custom")});
        m_schedule->setName("display:schedule");
        m_schedule->onActivated = [this](int index) { m_nightLight.setSchedule(index); };

        m_sunsetRow = m_nightCard->addChild<ListRow>(tr("Sunset"));
        m_sunsetRow->setName("display:sunset");
        m_sunsetRow->setInteractive(false);
        m_sunset = m_sunsetRow->addContent<SecondaryLabel>();
        m_sunriseRow = m_nightCard->addChild<ListRow>(tr("Sunrise"));
        m_sunriseRow->setInteractive(false);
        m_sunrise = m_sunriseRow->addContent<SecondaryLabel>();

        m_fromRow = m_nightCard->addChild<ListRow>(tr("From"));
        m_fromRow->setName("display:from");
        m_fromRow->setInteractive(false);
        m_from = m_fromRow->addContent<TimeComboBox>();
        m_from->onPicked = [this](int minutes) { m_nightLight.setCustomStart(minutes); };
        m_toRow = m_nightCard->addChild<ListRow>(tr("To"));
        m_toRow->setInteractive(false);
        m_to = m_toRow->addContent<TimeComboBox>();
        m_to->onPicked = [this](int minutes) { m_nightLight.setCustomEnd(minutes); };
    }

    void updateScreens()
    {
        OutputModel& model = outputs();
        const int count = model.count();
        std::vector<std::string> names;
        for (int row = 0; row < count; ++row)
            names.push_back(model.name(row).empty() ? tr("Screen") : model.name(row));
        if (names != m_tabNames) {
            m_tabNames = names;
            const int current = m_screenTabs->currentIndex();
            m_screenTabs->setItems(names);
            m_screenTabs->setCurrentIndex(std::min(std::max(current, 0), std::max(count - 1, 0)));
        }
        m_screens->setVisible(count > 0);
        m_screenName->setVisible(count == 1);
        m_screenTabs->setVisible(count > 1);
        const int row = currentRow();
        m_screenName->setText(count > 0 && !model.name(row).empty() ? model.name(row) : tr("Screen"));
        m_screenPage->setEnabled(m_screen.available() && !m_screen.busy() && !m_screen.pendingConfirmation());

        if (count > 0) {
            const std::vector<std::string> resolutions = model.resolutions(row);
            std::vector<std::string> resolutionItems;
            for (int i = 0; i < int(resolutions.size()); ++i) {
                // "Recommended" is only a hint while choosing.
                resolutionItems.push_back(i == model.recommendedResolutionIndex(row) ? recommended(resolutions[i])
                                                                                     : resolutions[i]);
            }
            setItems(m_resolution, resolutionItems);
            m_resolution->setCurrentIndex(model.resolutionIndex(row));
            const int resolution = m_resolution->currentIndex();
            m_resolution->setDisplayText(resolution >= 0 && resolution < int(resolutions.size())
                                             ? resolutions[resolution]
                                             : std::string());

            setItems(m_refreshRate, model.refreshRates(row));
            m_refreshRate->setCurrentIndex(std::max(0, model.refreshRateIndex(row)));

            const std::vector<double> scales = model.supportedScales();
            std::vector<std::string> scaleItems;
            int nearest = 0;
            for (int i = 0; i < int(scales.size()); ++i) {
                const bool recommendedScale = std::abs(scales[i] - model.recommendedScale(row)) < 0.001;
                scaleItems.push_back(recommendedScale ? recommended(percent(scales[i])) : percent(scales[i]));
                // The nearest step, for a scale set some other way.
                if (std::abs(scales[i] - model.scale(row)) < std::abs(scales[nearest] - model.scale(row)))
                    nearest = i;
            }
            setItems(m_scale, scaleItems);
            m_scale->setCurrentIndex(nearest);
            m_scale->setDisplayText(percent(scales[nearest]));

            static const int rotations[] = {OutputModel::None, OutputModel::Left, OutputModel::Inverted,
                                            OutputModel::Right};
            const auto rotation = std::find(std::begin(rotations), std::end(rotations), model.rotation(row));
            m_rotation->setCurrentIndex(rotation == std::end(rotations) ? 0 : int(rotation - std::begin(rotations)));

            m_arrangement->setVisible(count > 1);
            m_enabled->setChecked(model.isEnabled(row));
            m_primary->setChecked(model.isPrimary(row));
            m_primary->control()->setEnabled(model.isEnabled(row) && !model.isPrimary(row));
            m_modeRow->setVisible((model.capabilities(row) & 4) != 0);
            setItems(m_mode, model.replicationSources(row));
            m_mode->setCurrentIndex(model.replicationSourceIndex(row));
            // Typing keeps its text until it is applied; a position changed elsewhere still shows.
            if (!m_positionX->hasFocus())
                m_positionX->setText(std::to_string(model.position(row).x));
            if (!m_positionY->hasFocus())
                m_positionY->setText(std::to_string(model.position(row).y));

            m_brightness.setDisplay(model.drmConnector(row));
        }
        updateBrightness();

        m_errorCard->setVisible(m_screen.loaded() && (!m_screen.available() || !m_screen.error().empty()));
        m_error->setText(m_screen.available() ? m_screen.error()
                                              : tr("Display configuration is unavailable in this session."));
        updateConfirmation();
    }

    void updateBrightness()
    {
        m_brightnessRow->setVisible(m_brightness.available());
        if (!m_brightnessSlider->slider()->isPressed() && !m_brightnessTimer.isActive())
            m_brightnessSlider->slider()->setValue(m_brightness.value());
    }

    // Only when they change, so an open menu stays as it is.
    void setItems(RowComboBox* box, const std::vector<std::string>& items)
    {
        std::vector<std::string>& shown = m_items[box];
        if (shown == items)
            return;
        shown = items;
        box->setItems(items);
    }

    // The compositor reverts the previewed mode on its own deadline; the
    // dialog only relays it and goes away with the preview.
    void updateConfirmation()
    {
        if (!m_screen.pendingConfirmation()) {
            m_confirmation.reset();
            return;
        }
        const int seconds = m_screen.confirmationSeconds();
        const std::string text = trn("The previous settings come back in {n} second.",
                                     "The previous settings come back in {n} seconds.", seconds);
        if (m_confirmation) {
            m_confirmation->setText(text);
            return;
        }
        m_confirmation = std::make_unique<ConfirmDialog>(&settingsWindow().window(),
                                                         tr("Keep these display settings?"), text);
        m_confirmation->setCancelText(tr("Revert"));
        m_confirmation->setConfirmText(tr("Keep"));
        m_confirmation->onAccepted = [this] { m_screen.keep(); };
        m_confirmation->onRejected = [this] { m_screen.revert(); };
        m_confirmation->show();
    }

    void updateNightLight()
    {
        m_nightCard->setVisible(m_nightLight.available());
        m_nightCard->setEnabled(m_nightLight.loaded());
        m_nightSwitch->setChecked(m_nightLight.enabled());
        m_temperature->setValue(m_nightLight.temperature());
        m_schedule->setCurrentIndex(m_nightLight.schedule());
        const bool sunsetToSunrise = m_nightLight.schedule() == NightLight::SunsetToSunrise;
        const bool custom = m_nightLight.schedule() == NightLight::Custom;
        m_sunsetRow->setVisible(sunsetToSunrise);
        m_sunriseRow->setVisible(sunsetToSunrise);
        m_sunset->setText(timeOfDay(m_nightLight.sunset()));
        m_sunrise->setText(timeOfDay(m_nightLight.sunrise()));
        m_fromRow->setVisible(custom);
        m_toRow->setVisible(custom);
        m_from->setMinutes(m_nightLight.customStart());
        m_to->setMinutes(m_nightLight.customEnd());
    }

    Screen m_screen;
    NightLight m_nightLight;
    Brightness m_brightness;
    base::Timer m_brightnessTimer;
    base::Timer m_temperatureTimer;
    double m_pendingTemperature = 0;
    std::vector<std::string> m_tabNames;
    ColumnLayout* m_screens;
    SectionLabel* m_screenName;
    SegmentedControl* m_screenTabs;
    ColumnLayout* m_screenPage;
    HalfRow* m_brightnessRow;
    RowSlider* m_brightnessSlider;
    ListRow* m_resolutionRow;
    RowComboBox* m_resolution;
    RowComboBox* m_refreshRate;
    RowComboBox* m_scale;
    RowComboBox* m_rotation;
    Card* m_arrangement;
    SwitchRow* m_enabled;
    SwitchRow* m_primary;
    ListRow* m_modeRow;
    RowComboBox* m_mode;
    NumberField* m_positionX;
    NumberField* m_positionY;
    Card* m_errorCard;
    Label* m_error;
    Card* m_nightCard;
    SwitchRow* m_nightSwitch;
    SliderRow* m_temperature;
    RowComboBox* m_schedule;
    ListRow* m_sunsetRow;
    Label* m_sunset;
    ListRow* m_sunriseRow;
    Label* m_sunrise;
    ListRow* m_fromRow;
    TimeComboBox* m_from;
    ListRow* m_toRow;
    TimeComboBox* m_to;
    std::unique_ptr<ConfirmDialog> m_confirmation;
    std::map<RowComboBox*, std::vector<std::string>> m_items;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

static const PageRegistration registration("display", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DisplayPage>(window);
});

} // namespace settings
