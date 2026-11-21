// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/capture_toolbar.h"

#include <algorithm>
#include <cmath>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "core/main_loop.h"
#include "lingmo/audio/audio_model.h"
#include "lingmo/audio/audio_object.h"
#include "lingmo/audio/backend.h"
#include "shell/screenshot/capture_overlay.h"
#include "shell/screenshot/capture_settings.h"
#include "shell/screenshot/screen_capture.h"
#include "shell/shell_data.h"
#include "shell/shell_host.h"
#include "shell/shell_window.h"
#include "ui/application.h"
#include "ui/bubble_shape.h"
#include "ui/embedded_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

using Lingmo::i18n::tr;
using Lingmo::i18n::trc;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;

namespace Lingmo::Shell {

// FishUI's bigRadius * 1.5, the toolbar's corners and its buttons' plates.
static constexpr double kRadius = 18;
static constexpr double kButtonWidth = 40;
static constexpr double kButtonHeight = 36;
static constexpr double kSpacing = 2;
static constexpr double kSeparatorHeight = 20;
static constexpr double kChevronSize = 12;
static constexpr double kChevronSpacing = 4;

// What the row gives a separator of a device pixel: whole logical pixels,
// as the row snaps its cells.
static double separatorCell(double scale)
{
    return std::ceil(1 / scale);
}

// A centred item on a whole pixel.
static double centred(double outer, double inner)
{
    return std::round((outer - inner) / 2);
}

static const struct {
    const char *icon;
    const char *tip;
    int type;
    int source;
} kModes[] = {
        {"capture-screen.svg", trNoop("Capture Entire Screen"), ScreenCapture::Image, ScreenCapture::Screen},
        {"capture-window.svg", trNoop("Capture Selected Window"), ScreenCapture::Image, ScreenCapture::Window},
        {"capture-selection.svg", trNoop("Capture Selected Portion"), ScreenCapture::Image, ScreenCapture::Region},
        {"record-screen.svg", trNoop("Record Entire Screen"), ScreenCapture::Video, ScreenCapture::Screen},
        {"record-window.svg", trNoop("Record Selected Window"), ScreenCapture::Video, ScreenCapture::Window},
        {"record-selection.svg", trNoop("Record Selected Portion"), ScreenCapture::Video, ScreenCapture::Region},
};

// --- ToolbarButton ---

ToolbarButton::ToolbarButton(std::string icon, int iconSize, std::string tip)
    : m_icon(std::move(icon))
    , m_iconSize(iconSize)
    , m_tip(std::move(tip))
{
}

ToolbarButton::ToolbarButton(std::string text, bool accent)
    : m_text(std::move(text))
    , m_accent(accent)
{
    m_label = addChild<ui::Label>(m_text);
    m_label->setElide(ui::Label::Elide::None);
}

ToolbarButton::~ToolbarButton() = default;

void ToolbarButton::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    requestPaint();
}

void ToolbarButton::setText(const std::string &text)
{
    if (!m_label || text == m_text)
        return;
    m_text = text;
    m_label->setText(text);
    requestLayout();
}

void ToolbarButton::setTip(std::string tip)
{
    m_tip = std::move(tip);
}

void ToolbarButton::setChevron(bool chevron)
{
    m_chevron = chevron;
    if (chevron) {
        auto *icon = addChild<ui::Icon>("chevron-down", kChevronSize);
        icon->setName("chevron");
    }
    requestLayout();
}

ui::SizeF ToolbarButton::sizeHint() const
{
    const double spacing = ui::Theme::current().largeSpacing;
    if (!m_label)
        return {kButtonWidth, kButtonHeight};
    double width = m_label->sizeHint().width;
    if (m_chevron)
        width += kChevronSpacing + kChevronSize;
    // The layout's cells are whole pixels.
    return {std::ceil(width + spacing * 2), kButtonHeight};
}

ui::Color ToolbarButton::contentColor() const
{
    return m_accent ? ui::Color {255, 255, 255, 255} : ui::Theme::current().text;
}

void ToolbarButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
    if (onStateChanged)
        onStateChanged();
}

void ToolbarButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
    if (onStateChanged)
        onStateChanged();
}

void ToolbarButton::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
    if (onPressed)
        onPressed();
    if (onStateChanged)
        onStateChanged();
}

void ToolbarButton::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (onStateChanged)
        onStateChanged();
    const ui::RectF rect = localRect();
    const bool inside = event.position.x >= 0 && event.position.y >= 0 && event.position.x < rect.width
            && event.position.y < rect.height;
    if (inside && onClicked)
        onClicked(event.modifiers);
}

void ToolbarButton::enabledChanged()
{
    if (!isEnabled()) {
        m_pressed = false;
        m_hovered = false;
    }
    requestPaint();
}

void ToolbarButton::layout()
{
    const ui::RectF rect = localRect();
    if (!m_label)
        return;
    const ui::SizeF label = m_label->sizeHint();
    m_label->setColor(contentColor());
    if (!m_chevron) {
        m_label->setGeometry({centred(rect.width, label.width), centred(rect.height, label.height), label.width,
                              label.height});
        return;
    }
    // A label and a chevron in a Row, centred together; each centred
    // vertically in the row.
    const double row = label.width + kChevronSpacing + kChevronSize;
    const double rowHeight = std::max(label.height, kChevronSize);
    const double x = centred(rect.width, row);
    const double y = centred(rect.height, rowHeight);
    m_label->setGeometry({x, y + centred(rowHeight, label.height), label.width, label.height});
    for (const auto &child : children()) {
        if (child->name() == "chevron") {
            static_cast<ui::Icon *>(child.get())->setColor(ui::Theme::current().text);
            child->setGeometry({x + label.width + kChevronSpacing, y + centred(rowHeight, kChevronSize),
                                kChevronSize, kChevronSize});
        }
    }
}

void ToolbarButton::paint(ui::Painter &painter)
{
    const ui::Theme &theme = ui::Theme::current();
    if (!isEnabled())
        painter.multiplyOpacity(0.35);
    // The highlight is the toolbar's corner drawn its padding inside it, so
    // every highlight is concentric with the toolbar's ends.
    ui::Color plate = contentColor();
    double opacity = m_pressed ? 0.2 : m_checked ? 0.16 : m_hovered ? 0.08 : 0;
    if (m_accent) {
        plate = m_pressed ? theme.highlight.darker(1.2) : m_hovered ? theme.highlight.lighter(1.1) : theme.highlight;
        opacity = 1;
    }
    if (opacity > 0)
        painter.drawRoundedRect(localRect(), {kRadius, theme.smallSpacing, ui::themeCorners()},
                                plate.withAlpha(plate.a / 255.0 * opacity));
    if (!m_icon.empty()) {
        const ui::RectF rect = localRect();
        painter.drawSvg(shellDataFile(m_icon),
                        {centred(rect.width, m_iconSize), centred(rect.height, m_iconSize), double(m_iconSize),
                         double(m_iconSize)},
                        contentColor());
    }
}

// --- CaptureToolbar ---

CaptureToolbar::CaptureToolbar(CaptureOverlay *overlay, ScreenCapture *capture, Shell *shell)
    : m_overlay(overlay)
    , m_capture(capture)
    , m_shell(shell)
{
    m_tooltip = std::make_unique<CaptureTooltip>(this, overlay, shell);

    const auto add = [this](std::unique_ptr<ToolbarButton> button) {
        ToolbarButton *added = button.get();
        addChild(std::move(button));
        m_buttons.push_back(added);
        added->onStateChanged = [this, added] { buttonStateChanged(added); };
        return added;
    };
    add(std::make_unique<ToolbarButton>("capture-close.svg", 16, tr("Close")))->onClicked = [this](uint32_t) {
        m_capture->cancel();
    };
    for (const auto &mode : kModes) {
        add(std::make_unique<ToolbarButton>(mode.icon, 24, tr(mode.tip)))->onClicked = [this, mode](uint32_t) {
            m_capture->setMode(mode.type, mode.source);
        };
    }
    m_options = add(std::make_unique<ToolbarButton>(tr("Options")));
    m_options->setChevron(true);
    m_options->onClicked = [this](uint32_t) {
        if (std::exchange(m_closesOptions, false))
            return;
        openOptions();
    };
    // Picking a window is itself the capture; there is nothing to press.
    m_action = add(std::make_unique<ToolbarButton>(tr("Capture"), true));
    m_action->onClicked = [this](uint32_t modifiers) { m_capture->perform(modifiers & ui::ControlModifier); };
    m_languageChanged = ui::Application::instance()->languageChanged.connect([this] { retranslate(); });
    update();
}

// The Options menu is made when it opens: an open one closes.
void CaptureToolbar::retranslate()
{
    closeOptions();
    closeButton()->setTip(tr("Close"));
    for (const auto &mode : kModes)
        modeButton(mode.type, mode.source)->setTip(tr(mode.tip));
    m_options->setText(tr("Options"));
    update();
}

CaptureToolbar::~CaptureToolbar()
{
    releasePopups();
}

double CaptureToolbar::radius() const
{
    return kRadius;
}

ToolbarButton *CaptureToolbar::modeButton(int type, int source) const
{
    return m_buttons[1 + type * 3 + source];
}

void CaptureToolbar::update()
{
    const bool video = m_capture->type() == ScreenCapture::Video;
    for (int type : {ScreenCapture::Image, ScreenCapture::Video}) {
        for (int source : {ScreenCapture::Screen, ScreenCapture::Window, ScreenCapture::Region}) {
            ToolbarButton *button = modeButton(type, source);
            button->setChecked(m_capture->type() == type && m_capture->source() == source);
            if (type == ScreenCapture::Video)
                button->setEnabled(!m_capture->isRecording());
        }
    }
    m_options->setChecked(m_menuOpen);
    m_action->setText(video ? tr("Record") : tr("Capture"));
    const bool action = m_capture->source() != ScreenCapture::Window;
    if (action != m_action->isVisible())
        m_action->setVisible(action);
    requestLayout();
    requestPaint();
    m_tooltip->refresh();
}

ui::SizeF CaptureToolbar::implicitSize() const
{
    const double scale = m_overlay->window()->scale();
    const double padding = ui::Theme::current().smallSpacing;
    double width = 0;
    int items = 0;
    for (ToolbarButton *button : m_buttons) {
        if (!button->isVisible())
            continue;
        width += button->sizeHint().width;
        ++items;
    }
    // Three separators, a device pixel wide with a small spacing each side.
    width += 3 * (separatorCell(scale) + 2 * padding);
    items += 3;
    width += kSpacing * (items - 1);
    return {width + padding * 2, kButtonHeight + padding * 2};
}

std::vector<double> CaptureToolbar::separatorPositions() const
{
    const double scale = m_overlay->window()->scale();
    const double padding = ui::Theme::current().smallSpacing;
    std::vector<double> positions;
    double x = padding;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        ToolbarButton *button = m_buttons[i];
        if (!button->isVisible())
            continue;
        x += button->sizeHint().width + kSpacing;
        // After Close, the pictures and the recordings.
        if (i == 0 || i == 3 || i == 6) {
            positions.push_back(x + padding);
            x += separatorCell(scale) + 2 * padding + kSpacing;
        }
    }
    return positions;
}

void CaptureToolbar::layout()
{
    const double scale = m_overlay->window()->scale();
    const double padding = ui::Theme::current().smallSpacing;
    double x = padding;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        ToolbarButton *button = m_buttons[i];
        if (!button->isVisible())
            continue;
        const ui::SizeF size = button->sizeHint();
        button->setGeometry({x, padding, size.width, size.height});
        x += size.width + kSpacing;
        if (i == 0 || i == 3 || i == 6)
            x += separatorCell(scale) + 2 * padding + kSpacing;
    }
}

void CaptureToolbar::paint(ui::Painter &painter)
{
    const double scale = m_overlay->window()->scale();
    const double padding = ui::Theme::current().smallSpacing;
    const ui::Color line = ui::Theme::current().text;
    for (double x : separatorPositions())
        painter.fillRect({x, padding + (kButtonHeight - kSeparatorHeight) / 2, 1 / scale, kSeparatorHeight},
                         line.withAlpha(line.a / 255.0 * 0.2));
}

void CaptureToolbar::setSelecting(bool selecting)
{
    if (selecting == m_selecting)
        return;
    const bool before = isDragging();
    m_selecting = selecting;
    if (before != isDragging())
        m_tooltip->draggingChanged();
}

bool CaptureToolbar::isOptionsOpen() const
{
    return m_menuOpen;
}

void CaptureToolbar::closeOptions()
{
    if (m_menu && m_menuOpen)
        m_menu->close();
}

void CaptureToolbar::pressedOutsideMenu()
{
    // A press outside the open menu takes it down before the click lands, so
    // a click on Options only closes it.
    m_closesOptions = m_menuOpen;
    closeOptions();
}

void CaptureToolbar::hideTooltip()
{
    m_tooltip->hideNow();
}

void CaptureToolbar::releasePopups()
{
    if (m_menu) {
        m_menu->onClosed = nullptr;
        m_menu->close();
    }
    m_menuOpen = false;
    m_tooltip->release();
}

void CaptureToolbar::visibilityChanged()
{
    if (!isVisible()) {
        m_pressed = false;
        m_dragActive = false;
        closeOptions();
    }
    m_tooltip->refresh();
}

void CaptureToolbar::buttonStateChanged(ToolbarButton *button)
{
    // Up as soon as the pointer is on the button, like the dock's tooltips.
    if (button->tip().empty())
        return;
    if (button->isPressed())
        m_tooltip->hideNow();
    else if (button->isPointerOver())
        m_tooltip->showFor(button);
    else
        m_tooltip->leave(button);
}

void CaptureToolbar::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    m_dragActive = false;
    const ui::RectF rect = geometry();
    m_pressPoint = {rect.x + event.position.x, rect.y + event.position.y};
    m_pressPosition = {rect.x, rect.y};
}

void CaptureToolbar::pointerMove(const ui::PointerEvent &event)
{
    if (!m_pressed)
        return;
    const ui::RectF rect = geometry();
    const ui::PointF point {rect.x + event.position.x, rect.y + event.position.y};
    const double dx = point.x - m_pressPoint.x;
    const double dy = point.y - m_pressPoint.y;
    if (!m_dragActive) {
        if (std::abs(dx) <= kDragThreshold && std::abs(dy) <= kDragThreshold)
            return;
        m_dragActive = true;
        m_tooltip->draggingChanged();
    }
    // Bounded to the overlay, as the DragHandler's axes are.
    const ui::RectF parentRect = parent() ? parent()->localRect() : ui::RectF {};
    const double x = std::clamp(m_pressPosition.x + dx, 0.0, std::max(0.0, parentRect.width - rect.width));
    const double y = std::clamp(m_pressPosition.y + dy, 0.0, std::max(0.0, parentRect.height - rect.height));
    setGeometry({x, y, rect.width, rect.height});
    m_overlay->update();
}

void CaptureToolbar::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    m_pressed = false;
    if (!std::exchange(m_dragActive, false))
        return;
    m_tooltip->draggingChanged();
    if (onMoved)
        onMoved();
}

void CaptureToolbar::buildMenu()
{
    const bool video = m_capture->type() == ScreenCapture::Video;
    CaptureSettings &settings = m_capture->settings();
    const std::string destination = video ? settings.videoDestination() : settings.imageDestination();
    const std::string custom = video ? settings.customVideoFolder() : settings.customImageFolder();
    const auto setDestination = [&settings, video](const std::string &value) {
        if (video)
            settings.setVideoDestination(value);
        else
            settings.setImageDestination(value);
    };

    // The previous menu goes on the next loop turn: it may be closing.
    if (m_menu) {
        m_menu->onClosed = nullptr;
        core::mainLoop()->post([menu = std::shared_ptr<ui::Menu>(std::move(m_menu))] {});
    }
    m_menu = std::make_unique<ui::Menu>();
    ui::Menu &menu = *m_menu;
    const auto header = [&menu](const std::string &text) { menu.setItemEnabled(menu.addItem(text, {}), false); };
    const auto choice = [&menu](const std::string &text, bool checked, std::function<void()> action) {
        const size_t index = menu.addItem(text, std::move(action));
        menu.setItemChecked(index, checked);
        return index;
    };

    header(tr("Save to"));
    choice(video ? tr("Videos") : tr("Screenshots"), destination == CaptureSettings::kDefault,
           [setDestination] { setDestination(CaptureSettings::kDefault); });
    choice(tr("Desktop"), destination == CaptureSettings::kDesktop,
           [setDestination] { setDestination(CaptureSettings::kDesktop); });
    choice(tr("Documents"), destination == CaptureSettings::kDocuments,
           [setDestination] { setDestination(CaptureSettings::kDocuments); });
    if (!video)
        choice(tr("Clipboard"), destination == CaptureSettings::kClipboard,
               [setDestination] { setDestination(CaptureSettings::kClipboard); });
    if (!custom.empty())
        choice(custom.substr(custom.rfind('/') + 1), destination == CaptureSettings::kCustom,
               [setDestination] { setDestination(CaptureSettings::kCustom); });
    menu.addItem(tr("Other Location…"), [capture = m_capture] {
        core::mainLoop()->post([capture] { capture->chooseFolder(); });
    });
    menu.addSeparator();

    header(tr("Timer"));
    for (const int seconds : {0, 5, 10}) {
        choice(seconds == 0 ? std::string(trc("timer", "None")) : trn("{n} Second", "{n} Seconds", seconds),
               settings.timer() == seconds, [&settings, seconds] { settings.setTimer(seconds); });
    }

    if (video) {
        menu.addSeparator();
        header(tr("Microphone"));
        choice(trc("microphone", "None"), settings.microphone().empty(), [&settings] { settings.setMicrophone({}); });
        if (!m_sources)
            m_sources = std::make_unique<audio::AudioModel>(audio::AudioObject::Source, audio::Backend::shared());
        for (const auto &source : m_sources->objects()) {
            if (source->isVirtualDevice())
                continue;
            const std::string node = source->name();
            const std::string description = source->description();
            const size_t index = choice(description.empty() ? node : description, settings.microphone() == node,
                                        [&settings, node] { settings.setMicrophone(node); });
            // A long device name is elided instead of widening the options.
            menu.setItemMaximumWidth(index, 320);
        }
    }

    menu.addSeparator();
    header(tr("Options"));
    choice(tr("Show Mouse Pointer"), video ? settings.recordPointer() : settings.showPointer(), [&settings, video] {
        if (video)
            settings.setRecordPointer(!settings.recordPointer());
        else
            settings.setShowPointer(!settings.showPointer());
    });
    choice(tr("Remember Last Selection"), settings.rememberSelection(),
           [&settings] { settings.setRememberSelection(!settings.rememberSelection()); });
}

void CaptureToolbar::openOptions()
{
    buildMenu();
    ui::Menu &menu = *m_menu;
    ui::Window *window = m_overlay->window()->window();
    const double gap = ui::Theme::current().smallSpacing;
    const ui::RectF toolbar = geometry();
    const ui::RectF button = m_options->geometry();
    const double start = toolbar.x + button.x;
    const double centre = start + button.width / 2;

    // Above the toolbar with its arrow on the Options button, or below it
    // when the toolbar is dragged to the top of the screen.
    menu.setArrow(ui::BubbleShape::Edge::Bottom, {centre, toolbar.y});
    menu.setContinuousCorners(true);
    const ui::RectF screen = parent() ? parent()->localRect() : ui::RectF {};
    menu.setAvailableGeometry(screen);
    ui::SizeF size = menu.implicitSize(window);
    const bool below = toolbar.y - gap - size.height < 0;
    if (below) {
        menu.setArrow(ui::BubbleShape::Edge::Top, {centre, toolbar.y + toolbar.height});
        size = menu.implicitSize(window);
    }
    const double y = below ? toolbar.y + toolbar.height + gap : toolbar.y - gap - size.height;
    // Starts at the button's leading side, pulled back only as far as the
    // arrow needs to still reach its centre.
    const double x = std::min(start, centre - menu.minimumEdgeLength() / 2);
    menu.onClosed = [this] {
        m_menuOpen = false;
        m_options->setChecked(false);
        m_tooltip->refresh();
        if (onOptionsChanged)
            onOptionsChanged();
    };
    m_tooltip->hideNow();
    menu.popupAt(window, {std::floor(x + 0.5), std::floor(y + 0.5)});
    m_menuOpen = menu.isOpen();
    m_options->setChecked(m_menuOpen);
    m_tooltip->refresh();
    if (onOptionsChanged)
        onOptionsChanged();
}

// --- CaptureTooltip ---

CaptureTooltip::CaptureTooltip(CaptureToolbar *toolbar, CaptureOverlay *overlay, Shell *shell)
    : m_toolbar(toolbar)
    , m_overlay(overlay)
    , m_shell(shell)
{
    m_hideTimer.setSingleShot(true);
    m_hideTimer.onTimeout = [this] {
        m_target = nullptr;
        apply();
    };
}

CaptureTooltip::~CaptureTooltip()
{
    m_window.reset();
}

bool CaptureTooltip::isShown() const
{
    return m_window && !m_window->isHidden();
}

void CaptureTooltip::showFor(ToolbarButton *button)
{
    m_hovered = button;
    if (m_toolbar->isDragging())
        return;
    m_hideTimer.stop();
    m_text = button->tip();
    m_target = button;
    apply();
}

// Leaving a button only hides the tooltip if no other button takes it over
// first, so sliding along the toolbar moves one tooltip instead of reopening it.
void CaptureTooltip::leave(ToolbarButton *button)
{
    if (m_hovered == button)
        m_hovered = nullptr;
    if (m_target == button)
        m_hideTimer.start(kHideDelay);
}

void CaptureTooltip::hideNow()
{
    m_hideTimer.stop();
    m_target = nullptr;
    apply();
}

// A tooltip up when a drag begins goes at once; it would point where the
// button no longer is. The button the drag ends on gets one.
void CaptureTooltip::draggingChanged()
{
    if (m_toolbar->isDragging())
        hideNow();
    else if (m_hovered)
        showFor(m_hovered);
}

void CaptureTooltip::refresh()
{
    apply();
}

void CaptureTooltip::release()
{
    m_released = true;
    m_hideTimer.stop();
    m_window.reset();
    m_label = nullptr;
}

void CaptureTooltip::apply()
{
    if (m_released)
        return;
    const bool shown = m_target && !m_text.empty() && m_toolbar->isVisible() && !m_toolbar->isOptionsOpen()
            && m_overlay->window()->isShown();
    if (!shown) {
        if (m_window)
            m_window->hide();
        return;
    }

    const ui::Theme &theme = ui::Theme::current();
    ui::BubbleShape bubble;
    bubble.radius = theme.mediumRadius;
    bubble.continuousCorners = true;

    // The label's size decides the bubble's, before any window is made,
    // measured as the popup will lay it out: at its parent's scale and hinting.
    ui::Label measure(m_text);
    measure.setElide(ui::Label::Elide::None);
    measure.setTextLayoutMode(m_overlay->window()->window()->textLayoutMode());
    const ui::SizeF textSize = measure.sizeHint();
    const double width = std::ceil(std::max(textSize.width + theme.largeSpacing * 1.5, bubble.minimumEdgeLength()));
    const double height = std::ceil(textSize.height + theme.largeSpacing + bubble.arrowLength);

    // In the overlay's coordinates. Above the toolbar, or below it when the
    // toolbar is dragged to the top of the screen.
    const ui::RectF toolbar = m_toolbar->geometry();
    const double gap = theme.smallSpacing;
    const bool below = toolbar.y - gap - height < 0;
    const double overlayWidth = m_overlay->bounds().width();
    const double top = below ? toolbar.y + toolbar.height + gap : toolbar.y - gap - height;

    // Where the arrow points: the middle of the button's edge facing the tooltip.
    const ui::RectF button = m_target->geometry();
    const double targetX = toolbar.x + button.x + button.width / 2;
    const double x = std::floor(std::max(0.0, std::min(overlayWidth - width, targetX - width / 2)) + 0.5);
    bubble.arrowEdge = below ? ui::BubbleShape::Edge::Top : ui::BubbleShape::Edge::Bottom;
    bubble.width = width;
    bubble.height = height;
    bubble.arrowPosition = targetX - x;
    const ui::RectF body = bubble.bodyRect();

    ui::PopupPlacement placement;
    placement.anchorRect = {x, top, 1, 1};
    placement.anchor = ui::PopupAnchor::TopLeft;
    placement.gravity = ui::PopupAnchor::BottomRight;
    placement.flipX = placement.flipY = placement.slide = false;
    // A popup the toolkit closed with its window's popups stays closed: a
    // hidden one is made again.
    if (!m_window || m_window->isHidden()) {
        if (m_window)
            m_window->hide();
        m_window = ui::Window::createPopup(m_overlay->window()->window(), placement, false);
        auto content = std::make_unique<ui::View>();
        m_label = content->addChild<ui::Label>();
        m_label->setElide(ui::Label::Elide::None);
        m_window->setContentView(std::move(content));
        m_window->resize({width, height});
    } else {
        m_window->resize({width, height});
        m_window->setPopupPlacement(placement);
    }
    m_label->setText(m_text);
    m_body = body;
    m_label->setGeometry({m_body.x + (m_body.width - textSize.width) / 2,
                          m_body.y + (m_body.height - textSize.height) / 2, textSize.width, textSize.height});
    m_window->show();

    ShellWindow *popup = m_shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(m_window.get()));
    if (!popup)
        return;
    // Transparent for input: drags on the overlay go on under it.
    popup->setInputRegion(base::Region());
    // The glass is the body; the arrow outside it continues the glass, and
    // the outline follows the whole bubble.
    const render::GlassParams glass = theme.darkMode ? popupGlass(0.17, 0.17, 0.19, 0.6)
                                                     : popupGlass(1, 1, 1, 0.6);
    const base::Rect region(int(std::floor(body.x)), int(std::floor(body.y)), int(std::ceil(body.width)),
                            int(std::ceil(body.height)));
    const double radius = std::min(bubble.radius, std::min(body.width, body.height) / 2);
    popup->setBackgroundBlur(region, int(std::lround(radius)), glass, bubble.path());
}

} // namespace Lingmo::Shell
