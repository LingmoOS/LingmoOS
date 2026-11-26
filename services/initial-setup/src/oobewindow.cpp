// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oobewindow.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/paths.h"
#include "base/i18n.h"
#include "lingmo/appearance/appearance.h"
#include "lingmo/timedate/time_date.h"
#include "ui/application.h"
#include "ui/animation.h"
#include "ui/cell_layout.h"
#include "ui/check_box.h"
#include "ui/dialog_window.h"
#include "ui/fast_blur.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/text_area.h"
#include "ui/view.h"

#include "oobestate.h"

using namespace Lingmo::ui;
using Lingmo::I18n::tr;

namespace {

class PageStack final : public View
{
public:
    PageStack()
    {
        setClipsChildren(true);
    }

    View* addPage(std::unique_ptr<View> page)
    {
        View* result = page.get();
        m_pages.push_back(result);
        addChild(std::move(page));
        result->setVisible(m_pages.size() == 1);
        return result;
    }

    void setCurrent(size_t index)
    {
        if (index >= m_pages.size())
            return;
        if (index == m_current && m_hasCurrent)
            return;

        stopTransition();
        if (!m_hasCurrent) {
            m_current = index;
            m_hasCurrent = true;
            for (size_t i = 0; i < m_pages.size(); ++i) {
                m_pages[i]->setVisible(i == m_current);
                m_pages[i]->setOpacity(i == m_current ? 1 : 0);
                m_pages[i]->setTransform({});
            }
            return;
        }

        View* previous = m_pages[m_current];
        View* next = m_pages[index];
        const double direction = index > m_current ? 1 : -1;
        m_current = index;
        next->setVisible(true);
        next->setOpacity(0);
        next->setTransform({{direction * 36, 0}, 1});
        raiseChild(next);
        requestLayout();
        requestPaint();

        if (!window()) {
            finishTransition(previous, next);
            return;
        }

        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(260));
        m_transition->setEasing(Animation::Easing::OutCubic);
        m_transition->onUpdate = [this, previous, next, direction](double value) {
            previous->setOpacity(1 - value);
            previous->setTransform({{-direction * 36 * value, 0}, 1});
            next->setOpacity(value);
            next->setTransform({{direction * 36 * (1 - value), 0}, 1});
        };
        m_transition->onFinished = [this, previous, next] { finishTransition(previous, next); };
        m_transition->start();
    }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        const double inset = std::min(12.0, bounds.width / 12.0);
        for (View* page : m_pages) {
            const double width = std::max(0.0, bounds.width - inset * 2);
            const double height = std::min(bounds.height, page->heightForWidth(width));
            page->setGeometry({inset, std::max(0.0, (bounds.height - height) / 2), width,
                               std::max(0.0, height)});
        }
    }

private:
    void stopTransition()
    {
        if (!m_transition)
            return;
        m_transition->stop();
        m_transition.reset();
        for (View* page : m_pages) {
            page->setVisible(false);
            page->setOpacity(1);
            page->setTransform({});
        }
        if (m_hasCurrent)
            m_pages[m_current]->setVisible(true);
    }

    void finishTransition(View* previous, View* next)
    {
        previous->setVisible(false);
        previous->setOpacity(1);
        previous->setTransform({});
        next->setVisible(true);
        next->setOpacity(1);
        next->setTransform({});
        requestPaint();
    }

    std::vector<View*> m_pages;
    size_t m_current = 0;
    bool m_hasCurrent = false;
    std::unique_ptr<Animation> m_transition;
};

class PageVisual final : public View
{
public:
    explicit PageVisual(std::string wallpaper)
        : m_wallpaper(addChild<ImageView>(std::move(wallpaper)))
        , m_icon(addChild<ThemeIcon>("applications-education-language", 92))
    {
        m_wallpaper->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        m_wallpaper->setCornerRadius(118, themeCorners());
        m_icon->setVisible(false);
    }

    void setWallpaper(std::string wallpaper)
    {
        m_wallpaper->setSource(std::move(wallpaper));
        m_wallpaper->requestPaint();
    }

    void setPage(size_t page)
    {
        if (page == m_page)
            return;
        static constexpr const char* icons[] = {
            "", "text-x-generic", "input-keyboard", "preferences-system-time",
            "network-wireless", "zorin-appearance", "emblem-checked"};
        View* oldView = m_page == 0 ? static_cast<View*>(m_wallpaper) : static_cast<View*>(m_icon);
        View* newView = page == 0 ? static_cast<View*>(m_wallpaper) : static_cast<View*>(m_icon);
        m_page = page;
        if (m_transition)
            m_transition->stop();
        m_transition.reset();
        oldView->setVisible(true);
        oldView->setOpacity(1);
        if (page != 0)
            m_icon->setIconName(icons[std::min(page, size_t(6))]);
        newView->setVisible(true);
        newView->setOpacity(0);
        if (!window()) {
            finishTransition(oldView, newView);
            return;
        }
        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(220));
        m_transition->setEasing(Animation::Easing::InOutCubic);
        m_transition->onUpdate = [oldView, newView](double value) {
            oldView->setOpacity(1 - value);
            newView->setOpacity(value);
        };
        m_transition->onFinished = [this, oldView, newView] { finishTransition(oldView, newView); };
        m_transition->start();
    }

protected:
    void layout() override
    {
        m_wallpaper->setGeometry(localRect());
        m_icon->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        if (!m_icon->isVisible())
        {
            painter.drawShadow(localRect(), localRect().width / 2, 16,
                               Color::rgb(0x000000).withAlpha(theme.darkMode ? 0.30 : 0.18), {0, 8});
            return;
        }
        const RectF circle = localRect().adjusted(18, 18, -18, -18);
        painter.drawShadow(circle, circle.width / 2, 14,
                           Color::rgb(0x000000).withAlpha(theme.darkMode ? 0.28 : 0.16), {0, 6});
        painter.fillEllipse(circle, theme.secondBackground.withAlpha(theme.darkMode ? 0.84 : 0.88));
        painter.fillEllipse(circle.adjusted(3, 3, -3, -3),
                           theme.highlight.withAlpha(theme.darkMode ? 0.16 : 0.10));
    }

private:
    void finishTransition(View* oldView, View* newView)
    {
        if (oldView != newView)
            oldView->setVisible(false);
        oldView->setOpacity(1);
        newView->setVisible(true);
        newView->setOpacity(1);
        requestPaint();
    }

    ImageView* m_wallpaper;
    ThemeIcon* m_icon;
    size_t m_page = 0;
    std::unique_ptr<Animation> m_transition;
};

class ChoiceCard final : public View
{
public:
    ChoiceCard(std::string icon, std::string title, std::string description)
        : m_row(addChild<RowLayout>())
        , m_icon(m_row->addChild<ThemeIcon>(std::move(icon), 30))
        , m_text(m_row->addChild<ColumnLayout>())
        , m_title(m_text->addChild<Label>(std::move(title)))
        , m_description(m_text->addChild<Label>(std::move(description)))
        , m_check(m_row->addChild<Icon>("check", 18))
    {
        m_row->setPadding({14, 4, 30, 4});
        m_row->setSpacing(10);
        m_row->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_text->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_title->setWeight(500);
        m_title->setElide(Label::Elide::Right);
        m_description->setColor(ColorId::DisabledText);
        m_description->setFontSize(11);
        m_description->setElide(Label::Elide::Right);
        m_check->setColor(ColorId::Highlight);
        m_check->setOpacity(0);
        setTracksHover(true);
    }

    void setSelected(bool selected)
    {
        if (selected == m_selected)
            return;
        m_selected = selected;
        m_check->setOpacity(selected ? 1 : 0);
        requestPaint();
    }

    bool isSelected() const { return m_selected; }
    std::function<void()> onClicked;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override { m_row->setGeometry(localRect()); }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        Color fill = theme.secondBackground.withAlpha(theme.darkMode ? 0.68 : 0.76);
        Color border = theme.text.withAlpha(theme.darkMode ? 0.10 : 0.12);
        if (m_selected) {
            fill = theme.highlight.withAlpha(theme.darkMode ? 0.24 : 0.14);
            border = theme.highlight.withAlpha(0.72);
        } else if (isHovered()) {
            fill = theme.alternateBackground.withAlpha(theme.darkMode ? 0.82 : 0.92);
        }
        const RectF bounds = localRect().adjusted(0, 0, -12, 0);
        painter.drawRoundedRect(bounds, {theme.mediumRadius, 0, themeCorners()}, fill,
                                1 / painter.scale(), border);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left)
            m_pressed = true;
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onClicked)
            onClicked();
    }

    void hoverChanged() override { requestPaint(); }

private:
    RowLayout* m_row;
    ThemeIcon* m_icon;
    ColumnLayout* m_text;
    Label* m_title;
    Label* m_description;
    Icon* m_check;
    bool m_selected = false;
    bool m_pressed = false;
};

class StepIndicator final : public View
{
public:
    void setCount(size_t count)
    {
        m_count = std::max<size_t>(1, count);
        requestPaint();
    }

    void setCurrent(size_t page)
    {
        if (page == m_current)
            return;
        m_current = page;
        requestPaint();
    }

protected:
    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const double gap = 6;
        const double height = 5;
        const double width = (geometry().width - gap * (m_count - 1)) / m_count;
        for (size_t i = 0; i < m_count; ++i) {
            const RectF segment {(width + gap) * i, 0, width, height};
            const Color color = i <= m_current ? theme.highlight : theme.alternateBackground;
            painter.fillRoundedRect(segment, height / 2, color);
        }
    }

private:
    size_t m_current = 0;
    size_t m_count = 4;
};

class SetupCard final : public View
{
public:
    SetupCard(Lingmo::appearance::Appearance& appearance, std::function<void()> onComplete)
        : m_appearance(appearance)
        , m_onComplete(std::move(onComplete))
    {
        setClipsChildren(true);
        setChildClipRadius(Theme::current().windowRadius);

        m_row = addChild<RowLayout>();
        m_row->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_row->setPadding({64, 52, 64, 52});
        m_row->setSpacing(56);

        std::string wallpaper = m_appearance.wallpaper();
        std::error_code error;
        if (wallpaper.empty() || !std::filesystem::is_regular_file(wallpaper, error))
            wallpaper = "/usr/share/backgrounds/lingmoos/default.jpg";
        m_visual = m_row->addChild<PageVisual>(std::move(wallpaper));
        m_visual->setLayoutHints({.preferredWidth = 264, .preferredHeight = 264,
                                  .verticalAlignment = Alignment::Center});

        m_panel = m_row->addChild<ColumnLayout>();
        m_panel->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_panel->setSpacing(16);

        m_title = m_panel->addChild<Label>();
        m_title->setFontSize(26);
        m_title->setWeight(600);
        m_title->setWrap(true);
        m_title->setMaximumLineCount(2);
        m_subtitle = m_panel->addChild<Label>();
        m_subtitle->setFontSize(14);
        m_subtitle->setColor(ColorId::DisabledText);
        m_subtitle->setWrap(true);
        m_subtitle->setMaximumLineCount(2);

        m_steps = m_panel->addChild<StepIndicator>();
        m_steps->setLayoutHints({.fillWidth = true, .preferredHeight = 5});

        m_pages = m_panel->addChild<PageStack>();
        m_pages->setLayoutHints({.fillWidth = true, .fillHeight = true, .stretch = 1});
        addPages();

        m_actions = m_panel->addChild<RowLayout>();
        m_actions->setLayoutHints({.fillWidth = true});
        m_actions->setSpacing(Theme::current().mediumSpacing);

        m_back = m_actions->addChild<DialogButton>(tr("Back"));
        m_back->setLayoutHints({.preferredWidth = 112, .preferredHeight = 44});
        m_back->onClicked = [this] {
            if (m_page > 0)
                showPage(m_page - 1);
        };

        auto* spacer = m_actions->addChild<View>();
        spacer->setLayoutHints({.fillWidth = true, .stretch = 1});

        m_next = m_actions->addChild<DialogButton>(tr("Continue"), true);
        m_next->setName("oobe-next");
        m_next->setLayoutHints({.preferredWidth = 144, .preferredHeight = 44});
        m_next->onClicked = [this] {
            if (m_page + 1 < kPageCount)
                showPage(m_page + 1);
            else if (m_onComplete)
                m_onComplete();
        };

        showPage(0);
        m_appearanceChanged = m_appearance.changed.connect([this] {
            updateWallpaper();
            updateAppearanceChoices();
        });
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double inset = std::clamp(width * 0.065, 32.0, 64.0);
        const double spacing = std::clamp(width * 0.055, 32.0, 56.0);
        const double visualSize = std::clamp(width * 0.30, 176.0, 264.0);
        m_row->setPadding({inset, 40, inset, 40});
        m_row->setSpacing(spacing);
        m_visual->setLayoutHints({.preferredWidth = visualSize, .preferredHeight = visualSize,
                                  .verticalAlignment = Alignment::Center});
        m_row->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const Color shadow = Color::rgb(0x000000).withAlpha(theme.darkMode ? 0.42 : 0.25);
        const Color surface = theme.secondBackground.withAlpha(theme.darkMode ? 0.88 : 0.84);
        painter.drawShadow(localRect(), theme.windowRadius, 24, shadow, {0, 12});
        painter.fillRoundedRect(localRect(), theme.windowRadius, surface, themeCorners());
        painter.drawSmoothRectangle(localRect().adjusted(0.5, 0.5, -0.5, -0.5),
                                    theme.windowRadius, Color::rgb(0x000000).withAlpha(0), 1,
                                    theme.text.withAlpha(theme.darkMode ? 0.08 : 0.12));
    }

private:
    static constexpr size_t kPageCount = 7;

    static std::unique_ptr<ColumnLayout> page(std::string text)
    {
        auto result = std::make_unique<ColumnLayout>();
        result->setSpacing(10);
        auto* body = result->addChild<Label>(std::move(text));
        body->setWrap(true);
        body->setMaximumLineCount(3);
        body->setFontSize(14);
        return result;
    }

    void addPages()
    {
        m_pages->addPage(page(tr("A calm, capable desktop for your everyday work.\n\nThis setup takes a few moments and can be changed later in Settings.")));

        auto agreement = page(tr("Please review the Lingmo Aurora service agreement before continuing."));
        auto* agreementTitle = agreement->addChild<Label>(tr("Lingmo Aurora Service Agreement"));
        agreementTitle->setWeight(600);
        auto* effective = agreement->addChild<Label>(tr("Effective date: 2026-11-01"));
        effective->setColor(ColorId::DisabledText);
        auto* text = agreement->addChild<TextArea>(tr(
                "Lingmo Aurora Service Agreement\n\n"
                "Before using Lingmo Aurora or any service provided with it, please read this agreement carefully. "
                "By installing or using the system, you agree to the terms and conditions described here.\n\n"
                "The system may include third-party services and software. Their respective terms and notices remain "
                "available in Settings. You can review or change your preferences after setup."));
        text->setReadOnly(true);
        text->setWrapMode(TextArea::WrapMode::WordWrap);
        text->setFrameVisible(true);
        text->setPadding({12, 10, 12, 10});
        text->setLayoutHints({.fillWidth = true, .preferredHeight = 180, .maximumHeight = 220});
        m_agreementAccepted = agreement->addChild<CheckBox>(tr("I agree to the Lingmo Aurora service agreement"));
        m_agreementAccepted->onToggled = [this](bool accepted) {
            m_next->setEnabled(accepted);
        };
        m_pages->addPage(std::move(agreement));

        auto keyboard = page(tr("Select the keyboard layout you use most often. You can add more layouts later in Settings."));
        m_usKeyboard = keyboard->addChild<ChoiceCard>("input-keyboard", tr("English (US)"),
                                                       tr("QWERTY keyboard layout"));
        m_pinyinKeyboard = keyboard->addChild<ChoiceCard>("input-keyboard", tr("中文拼音"), tr("Chinese Pinyin input"));
        m_usKeyboard->setLayoutHints({.fillWidth = true, .preferredHeight = 48, .maximumHeight = 48,
                                      .verticalAlignment = Alignment::Start, .margins = {0, 0, 12, 0}});
        m_pinyinKeyboard->setLayoutHints({.fillWidth = true, .preferredHeight = 48, .maximumHeight = 48,
                                          .verticalAlignment = Alignment::Start, .margins = {0, 0, 12, 0}});
        m_usKeyboard->setSelected(true);
        m_usKeyboard->onClicked = [this] { selectKeyboard(m_usKeyboard); };
        m_pinyinKeyboard->onClicked = [this] { selectKeyboard(m_pinyinKeyboard); };
        m_pages->addPage(std::move(keyboard));

        std::string timeText = tr("Use network time to keep the system clock accurate.");
        if (!m_timeDate.timezone().empty())
            timeText += std::string(tr("\n\nCurrent time zone: ")) + m_timeDate.timezone();
        auto time = page(std::move(timeText));
        auto* ntp = time->addChild<CheckBox>(tr("Set time automatically"));
        ntp->setChecked(m_timeDate.ntp());
        ntp->setEnabled(m_timeDate.canNtp());
        ntp->onToggled = [this](bool checked) {
            if (m_timeDate.canNtp())
                m_timeDate.setNtp(checked);
        };
        m_pages->addPage(std::move(time));

        m_pages->addPage(page(tr("Network access can be configured from the status bar after setup. Your existing connections will remain available.")));

        auto appearance = page(tr("Choose how Lingmo Aurora should look. You can change this later in Settings."));
        m_lightAppearance = appearance->addChild<ChoiceCard>("preferences-desktop-appearance", tr("Light appearance"),
                                                              tr("Bright surfaces with clear contrast"));
        m_darkAppearance = appearance->addChild<ChoiceCard>("zorin-appearance", tr("Dark appearance"),
                                                             tr("A darker palette for dim environments"));
        m_lightAppearance->setLayoutHints({.fillWidth = true, .preferredHeight = 48, .maximumHeight = 48,
                                           .verticalAlignment = Alignment::Start, .margins = {0, 0, 12, 0}});
        m_darkAppearance->setLayoutHints({.fillWidth = true, .preferredHeight = 48, .maximumHeight = 48,
                                          .verticalAlignment = Alignment::Start, .margins = {0, 0, 12, 0}});
        m_lightAppearance->onClicked = [this] { chooseAppearance(false); };
        m_darkAppearance->onClicked = [this] { chooseAppearance(true); };
        updateAppearanceChoices();
        m_pages->addPage(std::move(appearance));

        m_pages->addPage(page(tr("Your basic preferences are ready. Select Finish to enter the Lingmo Aurora desktop.")));
    }

    void showPage(size_t page)
    {
        m_page = page;
        static const char* const titles[] = {
            tr("Welcome to Lingmo Aurora"),
            tr("Service agreement"),
            tr("Keyboard layout"),
            tr("Time & date"),
            tr("Network access"),
            tr("Personalize your desktop"),
            tr("You're ready to go"),
        };
        static const char* const subtitles[] = {
            tr("Let's get your new desktop ready."),
            tr("Review the terms before continuing."),
            tr("Choose your primary input method."),
            tr("Keep your clock in sync."),
            tr("Connect after you enter the desktop."),
            tr("Choose a comfortable appearance."),
            tr("Finish setup to start using Aurora."),
        };
        m_steps->setCount(kPageCount);
        m_title->setText(titles[page]);
        m_subtitle->setText(subtitles[page]);
        m_steps->setCurrent(page);
        m_pages->setCurrent(page);
        m_visual->setPage(page);
        m_back->setVisible(page > 0);
        m_next->setText(page + 1 == kPageCount ? tr("Finish") : tr("Continue"));
        m_next->setEnabled(page != 1 || (m_agreementAccepted && m_agreementAccepted->isChecked()));
        requestLayout();
        requestPaint();
    }

    void updateWallpaper()
    {
        std::string wallpaper = m_appearance.wallpaper();
        std::error_code error;
        if (wallpaper.empty() || !std::filesystem::is_regular_file(wallpaper, error))
            wallpaper = "/usr/share/backgrounds/lingmoos/default.jpg";
        m_visual->setWallpaper(std::move(wallpaper));
    }

    void selectKeyboard(ChoiceCard* selected)
    {
        m_usKeyboard->setSelected(selected == m_usKeyboard);
        m_pinyinKeyboard->setSelected(selected == m_pinyinKeyboard);
    }

    void chooseAppearance(bool dark)
    {
        m_appearance.setDarkMode(dark);
        updateAppearanceChoices();
    }

    void updateAppearanceChoices()
    {
        if (m_lightAppearance)
            m_lightAppearance->setSelected(!m_appearance.darkMode());
        if (m_darkAppearance)
            m_darkAppearance->setSelected(m_appearance.darkMode());
    }

    Lingmo::appearance::Appearance& m_appearance;
    Lingmo::timedate::TimeDate m_timeDate;
    base::Subscription m_appearanceChanged;
    std::function<void()> m_onComplete;
    RowLayout* m_row = nullptr;
    PageVisual* m_visual = nullptr;
    ColumnLayout* m_panel = nullptr;
    Label* m_title = nullptr;
    Label* m_subtitle = nullptr;
    StepIndicator* m_steps = nullptr;
    PageStack* m_pages = nullptr;
    RowLayout* m_actions = nullptr;
    DialogButton* m_back = nullptr;
    DialogButton* m_next = nullptr;
    CheckBox* m_agreementAccepted = nullptr;
    ChoiceCard* m_usKeyboard = nullptr;
    ChoiceCard* m_pinyinKeyboard = nullptr;
    ChoiceCard* m_lightAppearance = nullptr;
    ChoiceCard* m_darkAppearance = nullptr;
    size_t m_page = 0;
};

class OobeView final : public View
{
public:
    explicit OobeView(std::function<void()> onComplete)
        : m_appearance()
        , m_wallpaper(addChild<FastBlurredCopy>(22))
        , m_tint(addChild<BackdropTint>())
        , m_card(addChild<SetupCard>(m_appearance, std::move(onComplete)))
    {
        setClipsChildren(false);

        m_wallpaperImage = m_wallpaper->setContent<ImageView>();
        m_wallpaperImage->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        m_wallpaperImage->onStatusChanged = [this](ImageView::Status) {
            m_wallpaper->contentChanged();
        };
        m_appearanceChanged = m_appearance.changed.connect([this] { updateWallpaper(); });
        updateWallpaper();
    }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        const double margin = 64;
        const double cardWidth = std::min(960.0, std::max(0.0, bounds.width - 2 * margin));
        const double cardHeight = std::min(640.0, std::max(0.0, bounds.height - 2 * margin));
        m_wallpaper->setGeometry(bounds);
        m_tint->setGeometry(bounds);
        m_card->setGeometry({(bounds.width - cardWidth) / 2, (bounds.height - cardHeight) / 2,
                             cardWidth, cardHeight});
    }

private:
    void updateWallpaper()
    {
        std::string wallpaper = m_appearance.wallpaper();
        std::error_code error;
        if (wallpaper.empty() || !std::filesystem::is_regular_file(wallpaper, error))
            wallpaper = "/usr/share/backgrounds/lingmoos/default.jpg";
        m_wallpaperImage->setSource(std::move(wallpaper));
        m_wallpaper->contentChanged();
    }

    class BackdropTint final : public View
    {
    protected:
        void paint(Painter& painter) override
        {
            const Theme& theme = Theme::current();
            painter.fillRect(localRect(), theme.background.withAlpha(theme.darkMode ? 0.28 : 0.14));
        }
    };

    Lingmo::appearance::Appearance m_appearance;
    base::Subscription m_appearanceChanged;
    FastBlurredCopy* m_wallpaper;
    BackdropTint* m_tint;
    ImageView* m_wallpaperImage = nullptr;
    SetupCard* m_card;
};

} // namespace

OobeWindow::OobeWindow()
{
    m_window.setTitle(tr("Lingmo Aurora Initial Setup"));
    m_window.setFullScreen(true);
    m_window.setUserResizable(false);
    m_window.setSkipTaskbar(true);
    m_window.setAlwaysOnTop(true);
    m_window.setBackgroundColor(Color::rgb(0x000000).withAlpha(0));
    m_view = std::make_unique<OobeView>([this] { complete(); });
    m_window.setContentView(std::move(m_view));
}

OobeWindow::~OobeWindow() = default;

void OobeWindow::show()
{
    m_window.show();
}

void OobeWindow::complete()
{
    if (!oobe::markComplete(Lingmo::base::paths::home()))
        return;
    oobe::clearAccountState();
    m_window.hide();
    if (auto* application = Application::instance())
        application->quit();
}

void OobeWindow::refreshTheme()
{
    m_window.requestPaint();
}
