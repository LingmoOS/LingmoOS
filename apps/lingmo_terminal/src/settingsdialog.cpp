// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsdialog.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/slider.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "ui/window.h"

#include "TerminalFont.h"
#include "TerminalView.h"

#include "terminalsettings.h"
#include "terminalwindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

// Bundled schemes, by file name, and their labels.
static const std::vector<std::string> kThemeNames = {
    "Catppuccin-Latte", "Catppuccin-Mocha", "Dracula", "Gruvbox-Dark", "Gruvbox-Light", "Nord", "One-Dark",
    "One-Light", "Solarized", "SolarizedLight", "TokyoNight", "TokyoNight-Day", "Tomorrow-Night"};
static const std::vector<std::string> kThemeLabels = {
    "Catppuccin Latte", "Catppuccin Mocha", "Dracula", "Gruvbox Dark", "Gruvbox Light", "Nord", "One Dark",
    "One Light", "Solarized Dark", "Solarized Light", "Tokyo Night", "Tokyo Night Day", "Tomorrow Night"};
static const std::vector<int> kScrollbackLines = {1000, 10000, 100000, -1};

static constexpr double kDialogWidth = 440;
static constexpr double kRowHeight = 44;
static constexpr double kRowInset = 14;

static Color separatorColor()
{
    return Theme::current().text.withAlpha(0.08);
}

// A label on the left, its controls on the right, a hairline below unless last.
class SettingRow : public View
{
public:
    SettingRow(std::string text, bool last)
        : m_label(addChild<Label>(std::move(text)))
        , m_last(last)
    {
    }

    template<typename T, typename... Args>
    T* addControl(Args&&... args)
    {
        T* control = addChild<T>(std::forward<Args>(args)...);
        m_controls.push_back(control);
        return control;
    }
    void setControlWidth(View* control, double width) { m_widths[control] = width; }

    SizeF sizeHint() const override { return {0, kRowHeight}; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const SizeF label = m_label->sizeHint();
        m_label->setGeometry({kRowInset, (kRowHeight - label.height) / 2, label.width, label.height});

        // A row at the right edge, large spacing between, children centred vertically.
        double x = width - kRowInset;
        for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it) {
            View* control = *it;
            const SizeF size = control->sizeHint();
            const auto fixed = m_widths.find(control);
            const double controlWidth = fixed != m_widths.end() ? fixed->second : size.width;
            x -= controlWidth;
            control->setGeometry({x, (kRowHeight - size.height) / 2, controlWidth, size.height});
            x -= Theme::current().largeSpacing;
        }
    }

    void paint(Painter& painter) override
    {
        if (!m_last)
            painter.fillRect({kRowInset, kRowHeight - 1, geometry().width - 2 * kRowInset, 1}, separatorColor());
    }

private:
    Label* m_label;
    bool m_last;
    std::vector<View*> m_controls;
    std::map<View*, double> m_widths;
};

// A value shown beside its slider, right-aligned in a fixed width.
class ValueLabel : public View
{
public:
    ValueLabel()
        : m_label(addChild<Label>())
    {
        m_label->setColor(ColorId::DisabledText);
        m_label->setFontFeatures({{"tnum", 1}});
    }

    void setText(const std::string& text)
    {
        m_label->setText(text);
        requestLayout();
    }

    SizeF sizeHint() const override { return {40, m_label->sizeHint().height}; }

protected:
    void layout() override
    {
        const SizeF size = m_label->sizeHint();
        m_label->setGeometry({geometry().width - size.width, 0, size.width, size.height});
    }

private:
    Label* m_label;
};

// A rounded card in the second background with rows in a column.
class Card : public View
{
public:
    SettingRow* addRow(std::string text, bool last = false)
    {
        SettingRow* row = addChild<SettingRow>(std::move(text), last);
        m_rows.push_back(row);
        return row;
    }

    double heightForWidth(double) const override { return kRowHeight * double(m_rows.size()); }
    SizeF sizeHint() const override { return {0, heightForWidth(0)}; }

protected:
    void layout() override
    {
        double y = 0;
        for (SettingRow* row : m_rows) {
            row->setGeometry({0, y, geometry().width, kRowHeight});
            y += kRowHeight;
        }
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, 0, themeCorners()}, theme.secondBackground);
    }

private:
    std::vector<SettingRow*> m_rows;
};

// The chosen font on the terminal's own colours.
class Preview : public View
{
public:
    explicit Preview(TerminalWindow* window)
        : m_window(window)
        , m_sampleClip(addChild<View>())
        , m_sample(m_sampleClip->addChild<Label>())
    {
        // Large sizes stop at the inner margin rather than at the border.
        m_sampleClip->setClipsChildren(true);
        m_sample->setName("sample");
        m_sample->setElide(Label::Elide::None);
        // Unwrapped labels keep one line; the sample has two.
        m_sample->setWrap(true);
    }

    void update()
    {
        const TerminalSettings& settings = m_window->settings();
        const Konsole::ColorSchemeInfo info = Konsole::TerminalView::colorSchemeInfo(settings.colorScheme());
        m_background = m_window->terminalBackground();
        const Color text = info.valid ? info.foreground : Theme::current().text;
        const Color green = info.valid ? info.colors[2] : Theme::current().text;
        const Color blue = info.valid ? info.colors[4] : Theme::current().text;
        m_swatches.clear();
        if (info.valid)
            m_swatches.assign(info.colors.begin(), info.colors.begin() + 8);

        m_sample->setFontFamily(settings.fontName());
        // A point size on a 96 dpi screen.
        m_sample->setFontSize(settings.fontPointSize() * 4.0 / 3.0);
        m_sample->setColor(text);
        m_sample->setSpans({{"user@lingmo", 0, green},
                            {" ", 0, {}},
                            {"~", 0, blue},
                            {" $ ls\n", 0, {}},
                            {"Documents\xc2\xa0\xc2\xa0Pictures", 0, blue},
                            {"\xc2\xa0\xc2\xa0notes.txt", 0, {}}});
        requestLayout();
        requestPaint();
    }

    double heightForWidth(double width) const override
    {
        return crowded(width) ? m_sample->sizeHint().height + 2 * 14 + kSwatchSize + 8 : 76;
    }
    SizeF sizeHint() const override { return {0, heightForWidth(geometry().width)}; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        const bool isCrowded = crowded(width);
        const SizeF sample = m_sample->sizeHint();
        const double areaHeight = height - 14 - (isCrowded ? 14 + kSwatchSize + 8 : 14);
        m_sampleClip->setGeometry({14, 14, std::max(0.0, width - 28), std::max(0.0, areaHeight)});
        m_sample->setGeometry({0, (areaHeight - sample.height) / 2, std::ceil(sample.width), sample.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawSmoothRectangle(localRect(), theme.bigRadius, m_background, 1, separatorColor());

        // The scheme's eight normal colours, bottom right.
        double x = geometry().width - 12 - swatchesWidth();
        const double y = geometry().height - 12 - kSwatchSize;
        for (const Color& color : m_swatches) {
            painter.drawSmoothRectangle({x, y, kSwatchSize, kSwatchSize}, 3, color, 1,
                                       Color{128, 128, 128, 255}.withAlpha(0.25));
            x += kSwatchSize + 4;
        }
    }

private:
    static constexpr double kSwatchSize = 10;

    double swatchesWidth() const
    {
        return m_swatches.empty() ? 0 : m_swatches.size() * kSwatchSize + (m_swatches.size() - 1) * 4.0;
    }

    // Text that would run under the swatches gets a band of its own above them.
    bool crowded(double width) const
    {
        return m_sample->sizeHint().width > width - 2 * 14 - swatchesWidth() - 8;
    }

    TerminalWindow* m_window;
    View* m_sampleClip;
    Label* m_sample;
    Color m_background;
    std::vector<Color> m_swatches;
};

class SettingsContent : public View
{
public:
    explicit SettingsContent(TerminalWindow* window)
        : m_window(window)
        , preview(addChild<Preview>(window))
    {
        setName("settingsContent");
        TerminalSettings& settings = window->settings();

        Card* appearance = addCard();
        theme = appearance->addRow(tr("Theme"))->addControl<ComboBox>(kThemeLabels);
        setWidth(theme, 200);
        theme->onActivated = [&settings](int index) { settings.setColorScheme(kThemeNames[size_t(index)]); };

        SettingRow* fontRow = appearance->addRow(tr("Font"));
        font = fontRow->addControl<ComboBox>();
        setWidth(font, 200);
        font->onActivated = [this, &settings](int index) {
            if (index >= 0 && index < int(m_families.size()))
                settings.setFontName(m_families[size_t(index)]);
        };

        SettingRow* sizeRow = appearance->addRow(tr("Font Size"));
        fontSize = sizeRow->addControl<Slider>();
        sizeRow->setControlWidth(fontSize, 160);
        fontSize->setRange(5, 30);
        fontSize->setStepSize(1);
        fontSize->onMoved = [&settings](double value) { settings.setFontPointSize(int(std::lround(value))); };
        fontSizeValue = addValueLabel(sizeRow);

        blinking = appearance->addRow(tr("Blinking Cursor"), true)->addControl<Switch>();
        blinking->onToggled = [&settings](bool checked) { settings.setBlinkingCursor(checked); };

        Card* window2 = addCard();
        SettingRow* opacityRow = window2->addRow(tr("Opacity"));
        opacity = opacityRow->addControl<Slider>();
        opacityRow->setControlWidth(opacity, 160);
        opacity->setRange(0.1, 1.0);
        opacity->setStepSize(0.05);
        opacity->onMoved = [&settings](double value) { settings.setOpacity(value); };
        opacityValue = addValueLabel(opacityRow);

        blur = window2->addRow(tr("Window Blur"), true)->addControl<Switch>();
        blur->onToggled = [&settings](bool checked) { settings.setBlur(checked); };

        Card* behaviour = addCard();
        // In the order of the cursorShape values.
        cursorShape = behaviour->addRow(tr("Cursor Shape"))->addControl<ComboBox>(std::vector<std::string>{
            // TRANSLATORS: the shapes of the text cursor.
            tr("Block"), tr("Underline"), tr("I-Beam")});
        setWidth(cursorShape, 160);
        cursorShape->onActivated = [&settings](int index) { settings.setCursorShape(index); };

        scrollback = behaviour->addRow(tr("Scrollback"))->addControl<ComboBox>(std::vector<std::string>{
            tr("1,000 lines"), tr("10,000 lines"), tr("100,000 lines"), tr("Unlimited")});
        setWidth(scrollback, 160);
        scrollback->onActivated = [&settings](int index) { settings.setScrollbackLines(kScrollbackLines[size_t(index)]); };

        visualBell = behaviour->addRow(tr("Flash on Bell"))->addControl<Switch>();
        visualBell->onToggled = [&settings](bool checked) { settings.setVisualBell(checked); };
        confirmPaste = behaviour->addRow(tr("Warn Before Pasting Multiple Lines"), true)->addControl<Switch>();
        confirmPaste->onToggled = [&settings](bool checked) { settings.setConfirmMultilinePaste(checked); };
    }

    // The settings as they are now.
    void update()
    {
        const TerminalSettings& settings = m_window->settings();
        preview->update();

        const auto scheme = std::find(kThemeNames.begin(), kThemeNames.end(), m_window->colorScheme());
        theme->setCurrentIndex(std::max(0, int(scheme - kThemeNames.begin()) % int(kThemeNames.size())));

        // Built on first use: checking every family for fixed pitch loads the
        // fonts, and only this dialog needs the list.
        if (m_families.empty()) {
            m_families = Konsole::TerminalFont::fixedPitchFamilies();
            font->setItems(m_families);
        }
        const auto family = std::find(m_families.begin(), m_families.end(), settings.fontName());
        font->setCurrentIndex(family == m_families.end() ? 0 : int(family - m_families.begin()));

        fontSize->setValue(settings.fontPointSize());
        // TRANSLATORS: a font size in points.
        fontSizeValue->setText(format(tr("{size} pt"), {{"size", std::to_string(settings.fontPointSize())}}));
        blinking->setChecked(settings.blinkingCursor());
        opacity->setValue(settings.opacity());
        // TRANSLATORS: a percentage, such as "80%".
        opacityValue->setText(format(tr("{percent}%"),
                                     {{"percent", std::to_string(int(std::lround(settings.opacity() * 100)))}}));
        blur->setChecked(settings.blur());
        cursorShape->setCurrentIndex(settings.cursorShape());
        const auto lines = std::find(kScrollbackLines.begin(), kScrollbackLines.end(), settings.scrollbackLines());
        scrollback->setCurrentIndex(lines == kScrollbackLines.end() ? 0 : int(lines - kScrollbackLines.begin()));
        visualBell->setChecked(settings.visualBell());
        confirmPaste->setChecked(settings.confirmMultilinePaste());
        requestLayout();
    }

    double heightForWidth(double width) const override
    {
        double height = preview->heightForWidth(width);
        for (Card* card : m_cards)
            height += Theme::current().largeSpacing + card->heightForWidth(width);
        return height;
    }

    Preview* preview;
    ComboBox* theme;
    ComboBox* font;
    Slider* fontSize;
    ValueLabel* fontSizeValue;
    Switch* blinking;
    Slider* opacity;
    ValueLabel* opacityValue;
    Switch* blur;
    ComboBox* cursorShape;
    ComboBox* scrollback;
    Switch* visualBell;
    Switch* confirmPaste;

protected:
    // A column, large spacing between, at whole logical pixels.
    void layout() override
    {
        const double width = geometry().width;
        double y = 0;
        const double previewHeight = std::ceil(preview->heightForWidth(width));
        preview->setGeometry({0, y, width, previewHeight});
        y += previewHeight;
        for (Card* card : m_cards) {
            y += Theme::current().largeSpacing;
            const double height = card->heightForWidth(width);
            card->setGeometry({0, y, width, height});
            y += height;
        }
    }

private:
    Card* addCard()
    {
        Card* card = addChild<Card>();
        m_cards.push_back(card);
        return card;
    }

    void setWidth(ComboBox* box, double width)
    {
        static_cast<SettingRow*>(box->parent())->setControlWidth(box, width);
    }

    // A value shown beside its slider, in a fixed width so the slider does not shift.
    ValueLabel* addValueLabel(SettingRow* row)
    {
        return row->addControl<ValueLabel>();
    }

    TerminalWindow* m_window;
    std::vector<Card*> m_cards;
    std::vector<std::string> m_families;
};

SettingsDialog::SettingsDialog(TerminalWindow* window)
    : m_window(window)
{
    m_settingsChanged = window->settings().changed.connect([this] { update(); });
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::update()
{
    if (m_content)
        m_content->update();
}

void SettingsDialog::show()
{
    if (!m_dialog) {
        m_dialog = std::make_unique<DialogWindow>(&m_window->window(), tr("Settings"), kDialogWidth);
        m_content = m_dialog->setContent<SettingsContent>(m_window);
        m_dialog->onClosed = [this] {
            m_content = nullptr;
            Lingmo::base::EventLoop::current()->post([this] { m_dialog.reset(); });
        };
    }
    update();
    m_dialog->show();
    if (m_dialog->window())
        m_dialog->window()->activate();
}

void SettingsDialog::retranslate()
{
    if (!m_dialog || !m_content)
        return;
    m_dialog->setTitle(tr("Settings"));
    m_content = m_dialog->setContent<SettingsContent>(m_window);
    update();
}

void SettingsDialog::close()
{
    if (m_dialog)
        m_dialog->close();
}
