// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/auth_view.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include <unicode/uchar.h>
#include <unicode/utf8.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "auth/auth_controller.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "ui/animation.h"
#include "ui/application.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/cell_layout.h"
#include "ui/styled_label.h"
#include "ui/text_field.h"
#include "ui/window.h"

using Lingmo::i18n::tr;

namespace Lingmo::Auth {

namespace {

using ui::rounding::centeredStart;

constexpr ui::Color kWhite {255, 255, 255, 255};
constexpr ui::Color kBlack {0, 0, 0, 255};

ui::Color white(double alpha)
{
    return kWhite.withAlpha(alpha);
}

// The name's first character in upper case, for the picture's initial.
std::string initialOf(const std::string &name)
{
    int32_t offset = 0;
    UChar32 character = 0;
    U8_NEXT(name.data(), offset, int32_t(name.size()), character);
    if (character < 0)
        return name.substr(0, size_t(offset));
    char buffer[U8_MAX_LENGTH];
    int32_t length = 0;
    UBool error = false;
    U8_APPEND(buffer, length, U8_MAX_LENGTH, u_toupper(character), error);
    return std::string(buffer, size_t(length));
}

// A colour animation step: each channel, alpha included, straight.
ui::Color mix(ui::Color from, ui::Color to, double t)
{
    const auto channel = [t](uint8_t a, uint8_t b) {
        return uint8_t(std::lround(a + (double(b) - a) * t));
    };
    return {channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b),
            channel(from.a, to.a)};
}

// A change of the target animates from where the value is now. The first
// value is taken as it is.
template<typename T>
class Behavior
{
public:
    Behavior(ui::Window *window, int milliseconds, ui::Animation::Easing easing,
             std::function<T(T, T, double)> interpolate, std::function<void()> changed)
        : m_animation(window)
        , m_interpolate(std::move(interpolate))
        , m_changed(std::move(changed))
    {
        m_animation.setDuration(std::chrono::milliseconds(milliseconds));
        m_animation.setEasing(easing);
        m_animation.onUpdate = [this](double t) {
            m_value = m_interpolate(m_from, m_to, t);
            m_changed();
        };
    }

    T value() const { return m_value; }

    void setTarget(T target)
    {
        if (!m_initialized) {
            m_initialized = true;
            m_value = m_to = target;
            m_changed();
            return;
        }
        if (target == m_to)
            return;
        m_from = m_value;
        m_to = target;
        m_animation.stop();
        m_animation.start();
    }

private:
    ui::Animation m_animation;
    std::function<T(T, T, double)> m_interpolate;
    std::function<void()> m_changed;
    bool m_initialized = false;
    T m_value {};
    T m_from {};
    T m_to {};
};

using ColorBehavior = Behavior<ui::Color>;
using NumberBehavior = Behavior<double>;

std::function<ui::Color(ui::Color, ui::Color, double)> colorInterpolation()
{
    return mix;
}

std::function<double(double, double, double)> numberInterpolation()
{
    return [](double from, double to, double t) { return from + (to - from) * t; };
}

// FishUI's RoundedRectangle as the panel had it: snapped to device pixels,
// with a border of whole device pixels inside the outline.
void drawPanelRect(ui::Painter &painter, const ui::RectF &rect, double radius, ui::Color fill,
                   double border = 0, ui::Color borderColor = {})
{
    painter.drawRoundedRect(rect, {radius, 0, ui::Corners::Circular}, fill, border, borderColor);
}

} // namespace

// Raised text: the text drawn again in the style
// colour one logical pixel lower, under it.
class RaisedLabel final : public ui::StyledLabel
{
public:
    RaisedLabel(double size, int weight, ui::Color color, ui::Color styleColor)
    {
        setFontSize(size);
        setWeight(weight);
        setColor(color);
        setStyle(Style::Raised, styleColor);
        setElide(Elide::None);
        setFadesWhenDisabled(false);
    }

    void setElidedCentred()
    {
        setElide(Elide::Right);
        setTextAlignment(TextAlignment::Center);
    }
};

// A press and release inside, as MouseArea's clicked, and whether the
// pointer is over it, as its containsMouse with hoverEnabled.
class Clickable : public ui::View
{
public:
    bool acceptsPointer() const override { return true; }
    bool containsMouse() const { return m_hovered; }

    std::function<void()> onClicked;
    std::function<void()> onHoverChanged;

protected:
    void pointerEnter() override { setHovered(true); }
    void pointerLeave() override { setHovered(false); }
    void enabledChanged() override
    {
        if (!isEnabled())
            setHovered(false);
    }
    void pointerPress(const ui::PointerEvent &event) override
    {
        m_pressed = event.button == ui::MouseButton::Left;
    }
    void pointerRelease(const ui::PointerEvent &event) override
    {
        const bool clicked = m_pressed && localRect().contains(event.position);
        m_pressed = false;
        if (clicked && onClicked)
            onClicked();
    }

private:
    void setHovered(bool hovered)
    {
        if (hovered == m_hovered)
            return;
        m_hovered = hovered;
        if (onHoverChanged)
            onHoverChanged();
    }

    bool m_hovered = false;
    bool m_pressed = false;
};

// The account's picture cut to a circle on a light disc, or the account's
// initial where there is none. Where another account can be picked, hovering
// dims it and shows the swap arrows, and a click opens the accounts.
class Avatar final : public Clickable
{
public:
    static constexpr double kSize = 64;

    explicit Avatar(ui::Window *window)
        : m_hover(window, 140, ui::Animation::Easing::Linear, numberInterpolation(), [this] {
            // A picture shows through the dim; an initial would cross the arrows.
            m_initial->setOpacity(1 - m_hover.value());
            m_overlay->requestPaint();
        })
    {
        m_disc = addChild<Disc>();
        m_picture = addChild<ui::ImageView>();
        m_picture->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
        m_picture->setCornerRadius(kSize / 2);
        m_picture->onStatusChanged = [this](ui::ImageView::Status) { updateInitial(); };
        m_initial = addChild<ui::Label>();
        m_initial->setFontSize(28);
        m_initial->setWeight(300);
        m_initial->setColor(kWhite);
        m_initial->setElide(ui::Label::Elide::None);
        m_initial->setFadesWhenDisabled(false);
        m_overlay = addChild<SwapOverlay>(this);
        onHoverChanged = [this] { updateHover(); };
        m_hover.setTarget(0);
    }

    void setUser(const std::string &displayName, const std::string &icon)
    {
        m_initial->setText(displayName.empty() ? std::string("?") : initialOf(displayName));
        if (icon != m_icon) {
            m_icon = icon;
            m_picture->setSource(m_icon);
        }
        updateInitial();
        requestLayout();
    }

    void setSwappable(bool swappable)
    {
        if (swappable == m_swappable)
            return;
        m_swappable = swappable;
        setCursorShape(swappable ? ui::CursorShape::Pointer : ui::CursorShape::Default);
        updateHover();
    }

    bool acceptsPointer() const override { return m_swappable; }

    ui::SizeF sizeHint() const override { return {kSize, kSize}; }

protected:
    void layout() override
    {
        const ui::RectF rect = localRect();
        m_disc->setGeometry(rect);
        m_picture->setGeometry(rect);
        m_overlay->setGeometry(rect);
        const ui::SizeF initial = m_initial->sizeHint();
        m_initial->setGeometry({centeredStart(rect.width, initial.width),
                                centeredStart(rect.height, initial.height), initial.width,
                                initial.height});
    }

private:
    class Disc final : public ui::View
    {
    protected:
        void paint(ui::Painter &painter) override
        {
            drawPanelRect(painter, localRect(), geometry().width / 2, white(0.16));
        }
    };

    class SwapOverlay final : public ui::View
    {
    public:
        explicit SwapOverlay(Avatar *avatar)
            : m_avatar(avatar)
        {
        }

    protected:
        void paint(ui::Painter &painter) override
        {
            const double t = m_avatar->m_hover.value();
            if (t <= 0)
                return;
            drawPanelRect(painter, localRect(), geometry().width / 2, kBlack.withAlpha(0.35 * t));
            // The arrows on their 24 by 24 grid, centred.
            const double x = centeredStart(geometry().width, 24);
            const double y = centeredStart(geometry().height, 24);
            ui::Path arrows;
            arrows.moveTo({x + 20.5, y + 14.99});
            arrows.lineTo({x + 15.49, y + 20.01});
            arrows.moveTo({x + 3.5, y + 14.99});
            arrows.lineTo({x + 20.5, y + 14.99});
            arrows.moveTo({x + 3.5, y + 9.01});
            arrows.lineTo({x + 8.51, y + 3.99});
            arrows.moveTo({x + 20.5, y + 9.01});
            arrows.lineTo({x + 3.5, y + 9.01});
            painter.strokePath(arrows, white(t), 1.5, ui::LineCap::Round, ui::LineJoin::Round);
        }

    private:
        Avatar *m_avatar;
    };

    void updateInitial()
    {
        // As an asynchronous Image: the initial until the picture is ready.
        m_initial->setVisible(m_picture->status() != ui::ImageView::Status::Ready);
    }

    void updateHover()
    {
        m_hover.setTarget(m_swappable && containsMouse() ? 1 : 0);
    }

    NumberBehavior m_hover;
    Disc *m_disc;
    ui::ImageView *m_picture;
    ui::Label *m_initial;
    SwapOverlay *m_overlay;
    std::string m_icon;
    bool m_swappable = false;
};

// The account's picture and name over the password field.
// Upstream: LoginUserView.
class UserView final : public ui::View
{
public:
    explicit UserView(ui::Window *window)
    {
        m_avatar = addChild<Avatar>(window);
        m_avatar->setName("user-avatar");
        m_name = addChild<RaisedLabel>(16, 600, kWhite, kBlack.withAlpha(0.3));
    }

    Avatar *avatar() const { return m_avatar; }

    void setUser(const std::string &displayName, const std::string &icon)
    {
        m_name->setText(displayName);
        m_avatar->setUser(displayName, icon);
        requestLayout();
    }

    ui::SizeF sizeHint() const override
    {
        return {std::max(Avatar::kSize, m_name->sizeHint().width),
                Avatar::kSize + 10 + m_name->sizeHint().height};
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        m_avatar->setGeometry({centeredStart(width, Avatar::kSize), 0, Avatar::kSize, Avatar::kSize});
        const ui::SizeF name = m_name->sizeHint();
        m_name->setGeometry({centeredStart(width, name.width), Avatar::kSize + 10, name.width,
                             name.height});
    }

private:
    Avatar *m_avatar;
    RaisedLabel *m_name;
};

// One account among the others: its picture with a halo behind it while
// hovered or chosen from the keyboard, a fainter one for the account picked
// now, and the name under it.
class UserTile final : public Clickable
{
public:
    static constexpr double kWidth = 96;
    static constexpr double kHalo = 6;

    UserTile(ui::Window *window, const std::string &displayName, const std::string &icon)
    {
        setCursorShape(ui::CursorShape::Pointer);
        m_avatar = addChild<Avatar>(window);
        m_avatar->setUser(displayName, icon);
        m_name = addChild<RaisedLabel>(13, 400, kWhite, kBlack.withAlpha(0.3));
        m_name->setText(displayName);
        m_name->setElidedCentred();
        onHoverChanged = [this] { requestPaint(); };
    }

    void setCurrent(bool current)
    {
        m_current = current;
        requestPaint();
    }

    void setHighlighted(bool highlighted)
    {
        m_highlighted = highlighted;
        requestPaint();
    }

    ui::SizeF sizeHint() const override
    {
        return {kWidth, Avatar::kSize + 2 * kHalo + 6 + m_name->sizeHint().height};
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        m_avatar->setGeometry({centeredStart(width, Avatar::kSize), kHalo, Avatar::kSize, Avatar::kSize});
        const double nameHeight = m_name->sizeHint().height;
        m_name->setGeometry({0, Avatar::kSize + 2 * kHalo + 6, width, nameHeight});
    }

    void paint(ui::Painter &painter) override
    {
        const double alpha = containsMouse() || m_highlighted ? 0.24 : m_current ? 0.12 : 0;
        if (alpha <= 0)
            return;
        const double diameter = Avatar::kSize + 2 * kHalo;
        drawPanelRect(painter, {centeredStart(geometry().width, diameter), 0, diameter, diameter},
                      diameter / 2, white(alpha));
    }

private:
    Avatar *m_avatar;
    RaisedLabel *m_name;
    bool m_current = false;
    bool m_highlighted = false;
};

// The accounts to switch to, in rows centred under each other. The arrow keys
// move between them, Return picks one and Escape goes back to the password.
class UserGrid final : public ui::View
{
public:
    explicit UserGrid(ui::Window *window)
        : m_window(window)
    {
        setClickFocusOnly(true);
    }

    std::function<void(int index)> onActivated;
    std::function<void()> onCancelled;

    void setUsers(const std::vector<std::string> &names, const std::vector<std::string> &icons)
    {
        while (!children().empty())
            takeChild(children().back().get());
        m_tiles.clear();
        for (size_t i = 0; i < names.size(); ++i) {
            auto *tile = addChild<UserTile>(m_window, names[i], i < icons.size() ? icons[i] : std::string());
            tile->setName("user-tile-" + std::to_string(i));
            tile->onClicked = [this, i] {
                if (onActivated)
                    onActivated(int(i));
            };
            m_tiles.push_back(tile);
        }
        setCurrentIndex(m_current);
        requestLayout();
    }

    void setCurrentIndex(int index)
    {
        m_current = index;
        for (size_t i = 0; i < m_tiles.size(); ++i)
            m_tiles[i]->setCurrent(int(i) == index);
    }

    // Each time it opens, the keyboard starts from the account picked now,
    // and highlights nothing until it is used.
    void reset()
    {
        m_highlight = std::clamp(m_current, 0, std::max(0, int(m_tiles.size()) - 1));
        m_keyboard = false;
        updateHighlight();
    }

    ui::SizeF sizeFor(double width) const
    {
        const int count = int(m_tiles.size());
        if (count == 0)
            return {};
        const int columns = columnsFor(width);
        const int rows = (count + columns - 1) / columns;
        return {columns * UserTile::kWidth + (columns - 1) * kColumnGap,
                rows * tileHeight() + (rows - 1) * kRowGap};
    }

    bool acceptsFocus() const override { return true; }

protected:
    void layout() override
    {
        const int count = int(m_tiles.size());
        const int columns = columnsFor(geometry().width);
        const double height = tileHeight();
        for (int i = 0; i < count; ++i) {
            const int row = i / columns;
            const int inRow = std::min(columns, count - row * columns);
            const double rowWidth = inRow * UserTile::kWidth + (inRow - 1) * kColumnGap;
            const double x = centeredStart(geometry().width, rowWidth)
                    + (i % columns) * (UserTile::kWidth + kColumnGap);
            m_tiles[size_t(i)]->setGeometry({x, row * (height + kRowGap), UserTile::kWidth, height});
        }
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        const int count = int(m_tiles.size());
        const int columns = columnsFor(geometry().width);
        const auto move = [&](int to) {
            m_highlight = std::clamp(to, 0, std::max(0, count - 1));
            m_keyboard = true;
            updateHighlight();
        };
        switch (event.key) {
        case XKB_KEY_Left:
            move(m_highlight - 1);
            return true;
        case XKB_KEY_Right:
            move(m_highlight + 1);
            return true;
        case XKB_KEY_Up:
            move(m_highlight - columns >= 0 ? m_highlight - columns : m_highlight);
            return true;
        case XKB_KEY_Down:
            move(m_highlight + columns < count ? m_highlight + columns : m_highlight);
            return true;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
            if (count > 0 && onActivated)
                onActivated(m_keyboard ? m_highlight : m_current);
            return true;
        case XKB_KEY_Escape:
            if (onCancelled)
                onCancelled();
            return true;
        default:
            return false;
        }
    }

private:
    static constexpr double kColumnGap = 8;
    static constexpr double kRowGap = 16;

    int columnsFor(double width) const
    {
        const int fit = int(std::floor((width + kColumnGap) / (UserTile::kWidth + kColumnGap)));
        return std::clamp(fit, 1, std::max(1, int(m_tiles.size())));
    }

    double tileHeight() const { return m_tiles.empty() ? 0 : m_tiles.front()->sizeHint().height; }

    void updateHighlight()
    {
        for (size_t i = 0; i < m_tiles.size(); ++i)
            m_tiles[i]->setHighlighted(m_keyboard && int(i) == m_highlight);
    }

    ui::Window *m_window;
    std::vector<UserTile *> m_tiles;
    int m_current = -1;
    int m_highlight = 0;
    bool m_keyboard = false;
};

// One of a few names, the account or the session to log in with: the current
// choice as a pill, and the others in a list while it is open.
class ChoiceMenu final : public ui::View
{
public:
    // The list goes on `layer`, over everything else in the panel, and is
    // placed there whenever the menu is laid out or placeList() is called.
    ChoiceMenu(const std::string &name, ui::View *layer)
    {
        setName(name);
        m_button = addChild<Button>(this);
        m_button->setName(name + "-button");
        m_popup = layer->addChild<Popup>(this);
        m_popup->setName(name + "-list");
        m_popup->setVisible(false);
    }

    std::function<void(int index)> onActivated;

    // After the menu moved without being laid out itself.
    void placeList()
    {
        m_popup->place();
    }

    void setNames(const std::vector<std::string> &names)
    {
        m_names = names;
        m_popup->rebuild();
        m_button->setCurrent(currentName());
        requestLayout();
    }

    void setCurrentIndex(int index)
    {
        m_current = index;
        m_popup->rebuild();
        m_button->setCurrent(currentName());
        requestLayout();
    }

    void setOpen(bool open)
    {
        if (open == m_open)
            return;
        m_open = open;
        m_popup->setVisible(open && isVisible());
        m_button->requestPaint();
    }

    // A press elsewhere closes the list and still reaches what it pressed.
    void pressedAt(ui::PointF windowPosition)
    {
        if (!m_open)
            return;
        const auto inside = [&](const ui::View *view) {
            const ui::PointF local = view->mapFromWindow(windowPosition);
            return view->isVisible() && view->localRect().contains(local);
        };
        if (!inside(m_button) && !inside(m_popup))
            setOpen(false);
    }

    void setMenuEnabled(bool enabled)
    {
        setEnabled(enabled);
        if (!enabled)
            setOpen(false);
    }

    void setMenuVisible(bool visible)
    {
        setVisible(visible);
        m_popup->setVisible(visible && m_open);
    }

    ui::SizeF sizeHint() const override { return {std::max(m_button->sizeHint().width, 120.0), 30}; }

protected:
    void layout() override
    {
        const ui::SizeF button = m_button->sizeHint();
        m_button->setGeometry({centeredStart(geometry().width, button.width), 0, button.width, 30});
        m_popup->place();
    }

private:
    class Button final : public Clickable
    {
    public:
        explicit Button(ChoiceMenu *menu)
            : m_menu(menu)
        {
            m_label = addChild<ui::Label>();
            m_label->setFontSize(13);
            m_label->setColor(kWhite);
            m_label->setElide(ui::Label::Elide::None);
            m_label->setFadesWhenDisabled(false);
            onHoverChanged = [this] { requestPaint(); };
            onClicked = [this] {
                if (!m_menu->m_open) {
                    const double bottom = m_menu->mapToWindow({0, m_menu->geometry().height}).y;
                    const ui::View *root = m_menu;
                    while (root->parent())
                        root = root->parent();
                    m_menu->m_upward =
                            bottom + m_menu->m_popup->sizeHint().height + 6 > root->geometry().height;
                }
                m_menu->setOpen(!m_menu->m_open);
                m_menu->requestLayout();
            };
        }

        void setCurrent(const std::string &name)
        {
            m_label->setText(name);
            requestLayout();
        }

        ui::SizeF sizeHint() const override { return {m_label->sizeHint().width + 28, 30}; }

    protected:
        void layout() override
        {
            const ui::SizeF label = m_label->sizeHint();
            m_label->setGeometry({centeredStart(geometry().width, label.width),
                                  centeredStart(geometry().height, label.height), label.width,
                                  label.height});
        }

        void paint(ui::Painter &painter) override
        {
            drawPanelRect(painter, localRect(), geometry().height / 2,
                          white(containsMouse() || m_menu->m_open ? 0.24 : 0.16));
        }

    private:
        ChoiceMenu *m_menu;
        ui::Label *m_label;
    };

    class Entry final : public Clickable
    {
    public:
        Entry(const std::string &name, bool current)
        {
            m_label = addChild<ui::Label>();
            m_label->setText(name);
            m_label->setFontSize(13);
            m_label->setWeight(current ? 600 : 400);
            m_label->setColor(kWhite);
            m_label->setElide(ui::Label::Elide::None);
            onHoverChanged = [this] { requestPaint(); };
        }

    protected:
        void layout() override
        {
            const ui::SizeF label = m_label->sizeHint();
            m_label->setGeometry({12, centeredStart(geometry().height, label.height), label.width,
                                  label.height});
        }

        void paint(ui::Painter &painter) override
        {
            if (containsMouse())
                drawPanelRect(painter, localRect(), 8, white(0.16));
        }

    private:
        ui::Label *m_label;
    };

    class Popup final : public ui::View
    {
    public:
        explicit Popup(ChoiceMenu *menu)
            : m_menu(menu)
        {
        }

        void rebuild()
        {
            while (!children().empty())
                takeChild(children().back().get());
            // Every entry as wide as the widest name, so their highlights line up.
            double widest = 0;
            for (const std::string &name : m_menu->m_names) {
                ui::Label measure;
                measure.setTextLayoutMode(textLayoutMode());
                measure.setText(name);
                measure.setFontSize(13);
                measure.setWeight(600);
                widest = std::max(widest, measure.sizeHint().width);
            }
            m_entryWidth = std::max(160.0, std::ceil(widest) + 24);
            for (size_t i = 0; i < m_menu->m_names.size(); ++i) {
                auto *entry = addChild<Entry>(m_menu->m_names[i], int(i) == m_menu->m_current);
                entry->onClicked = [this, i] {
                    m_menu->setOpen(false);
                    if (m_menu->onActivated)
                        m_menu->onActivated(int(i));
                };
            }
            requestLayout();
        }

        ui::SizeF sizeHint() const override
        {
            return {m_entryWidth + 12, 30.0 * double(m_menu->m_names.size()) + 12};
        }

        // In the menu's place, in the coordinates of the layer it is on.
        void place()
        {
            const ui::SizeF size = sizeHint();
            const double menuWidth = m_menu->geometry().width;
            const ui::PointF origin = parent()->mapFromWindow(m_menu->mapToWindow({0, 0}));
            const double y = m_menu->m_upward ? -size.height - 6 : m_menu->geometry().height + 6;
            setGeometry({origin.x + centeredStart(menuWidth, size.width), origin.y + y, size.width,
                         size.height});
        }

        bool acceptsPointer() const override { return true; }

    protected:
        void layout() override
        {
            const double listHeight = 30.0 * double(children().size());
            const double x = centeredStart(geometry().width, m_entryWidth);
            const double top = centeredStart(geometry().height, listHeight);
            for (size_t i = 0; i < children().size(); ++i)
                children()[i]->setGeometry({x, top + 30.0 * double(i), m_entryWidth, 30});
        }

        void paint(ui::Painter &painter) override
        {
            drawPanelRect(painter, localRect(), 12, kBlack.withAlpha(0.55));
        }

    private:
        ChoiceMenu *m_menu;
        double m_entryWidth = 160;
    };

    std::string currentName() const
    {
        return m_current >= 0 && m_current < int(m_names.size()) ? m_names[size_t(m_current)]
                                                                 : std::string();
    }

    Button *m_button;
    Popup *m_popup;
    std::vector<std::string> m_names;
    int m_current = -1;
    bool m_open = false;
    // The panel is an image of a fixed size; a list that would run past its
    // bottom opens upward instead of being cut off.
    bool m_upward = false;
};

// The password field's TextInput: no frame, no clipboard and no undo. Each of
// those is a copy of the credential that would outlive the attempt.
class PasswordInput final : public ui::TextField
{
public:
    PasswordInput()
    {
        setFrameVisible(false);
        setTextMargins(0, 0);
        setFadesWhenDisabled(false);
        setEchoMode(EchoMode::Password);
        setPasswordCharacter("\u2022");
        // What "monospace" resolved to before.
        setFontFamily("DejaVu Sans Mono");
        setFontSize(13);
        setSelectionColors(white(0.35), kWhite);
        setCaretWidth(2);
        setClipboardEnabled(false);
        setContextMenuEnabled(false);
        updateColor();
    }

    void updateColor() { setTextColor(isEnabled() ? kWhite : white(0.45)); }

protected:
    void enabledChanged() override
    {
        ui::TextField::enabledChanged();
        updateColor();
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        // The clipboard keys are refused, which no setting covers.
        switch (event.key) {
        case XKB_KEY_XF86Copy:
        case XKB_KEY_XF86Cut:
        case XKB_KEY_XF86Paste:
            return true;
        case XKB_KEY_Insert:
            if (event.modifiers & (ui::ControlModifier | ui::ShiftModifier))
                return true;
            break;
        default:
            break;
        }
        return ui::TextField::keyPress(event);
    }
};

// The typed user name's TextInput.
class NameInput final : public ui::TextField
{
public:
    NameInput()
    {
        setFrameVisible(false);
        setTextMargins(0, 0);
        setFadesWhenDisabled(false);
        setFontSize(12);
        setTextColor(kWhite);
        setSelectionColors(white(0.35), kWhite);
        setCaretWidth(2);
        setContextMenuEnabled(false);
    }
};

// Where the name is typed, where no account could be listed.
class UserNameField final : public ui::View
{
public:
    UserNameField()
    {
        m_placeholder = addChild<ui::Label>();
        m_placeholder->setText(tr("User name"));
        m_placeholder->setFontSize(12);
        m_placeholder->setColor(ui::Color::fromArgb(0xaaffffff));
        m_placeholder->setElide(ui::Label::Elide::None);
        m_input = addChild<NameInput>();
        m_input->onTextChanged = [this] { m_placeholder->setVisible(m_input->text().empty()); };
    }

    NameInput *input() const { return m_input; }

    void retranslate()
    {
        m_placeholder->setText(tr("User name"));
        requestLayout();
    }

    ui::SizeF sizeHint() const override { return {0, 40}; }

protected:
    void layout() override
    {
        const ui::RectF inner {10, 10, geometry().width - 20, geometry().height - 20};
        // TextInput's default alignment is the top: its line, not the box, is placed.
        m_input->setGeometry({inner.x, inner.y, inner.width, m_input->sizeHint().height});
        const ui::SizeF placeholder = m_placeholder->sizeHint();
        m_placeholder->setGeometry({inner.x + centeredStart(inner.width, placeholder.width),
                                    inner.y + centeredStart(inner.height, placeholder.height),
                                    placeholder.width, placeholder.height});
    }

    void paint(ui::Painter &painter) override
    {
        drawPanelRect(painter, localRect(), 20, white(0.16));
    }

private:
    ui::Label *m_placeholder;
    NameInput *m_input;
};

// The password pill: the prompt, the masked text and a round arrow to submit.
// Upstream: LoginPasswordView.
class PasswordField final : public ui::View
{
public:
    explicit PasswordField(ui::Window *window)
        : m_fill(window, 140, ui::Animation::Easing::Linear, colorInterpolation(),
                 [this] { m_pill->requestPaint(); })
        , m_border(window, 140, ui::Animation::Easing::Linear, colorInterpolation(),
                   [this] { m_pill->requestPaint(); })
        , m_shake(window)
    {
        m_pill = addChild<Pill>(this);
        m_prompt = m_pill->addChild<ui::Label>();
        m_prompt->setFontSize(13);
        m_prompt->setColor(white(0.72));
        m_prompt->setElide(ui::Label::Elide::None);
        m_prompt->setFadesWhenDisabled(false);
        m_input = m_pill->addChild<PasswordInput>();
        m_input->onTextChanged = [this] { update(); };
        m_input->onTextEdited = [this] {
            if (onEdited)
                onEdited();
        };
        m_input->onAccepted = [this] {
            // The text stays, greyed out while the field is disabled; only a
            // rejection clears it (LoginAuthUserView::OnAuthComplete).
            if (m_input->text().empty() || !onAccepted)
                return;
            onAccepted(m_input->text());
        };
        m_go = m_pill->addChild<GoButton>(window, this);

        m_shake.setDuration(std::chrono::milliseconds(400));
        m_shake.setEasing(ui::Animation::Easing::Linear);
        m_shake.onUpdate = [this](double t) { setShakeOffset(shakeAt(t * 400)); };
        m_shake.onFinished = [this] { setShakeOffset(0); };
    }

    std::function<void(const std::string &password)> onAccepted;
    std::function<void()> onEdited;

    PasswordInput *input() const { return m_input; }

    void setPlaceholder(const std::string &text)
    {
        m_prompt->setText(text);
        m_pill->requestLayout();
    }

    void setFieldEnabled(bool enabled)
    {
        m_input->setEnabled(enabled);
        m_enabled = enabled;
        update();
    }

    void setKeyboardFocused(bool focused)
    {
        m_keyboardFocused = focused;
        update();
    }

    void clear()
    {
        m_input->setText({});
        update();
    }

    // Ash shakes the login pod on a rejection; this shakes the field, which is
    // where the user is looking.
    void shake()
    {
        m_shakeFrom = m_shakeOffset;
        m_shake.stop();
        m_shake.start();
    }

    // The pill where it is drawn, the shake included.
    ui::RectF pillRect() const { return m_pill->geometry(); }

    void update()
    {
        const bool focused = m_input->hasFocus() && m_keyboardFocused;
        m_fill.setTarget(white(!m_enabled ? 0.12 : focused ? 0.26 : 0.20));
        m_border.setTarget(white(!m_enabled ? 0.16 : focused ? 0.50 : 0.32));
        m_prompt->setVisible(m_input->text().empty());
        m_go->update();
    }

    ui::SizeF sizeHint() const override { return {0, 32}; }

protected:
    void layout() override
    {
        const ui::RectF rect = localRect();
        // The shake moves the pill, not the field, which the layout places.
        m_pill->setGeometry({rect.x + m_shakeOffset, rect.y, rect.width, rect.height});
        const ui::SizeF prompt = m_prompt->sizeHint();
        m_prompt->setGeometry({14, centeredStart(rect.height, prompt.height), prompt.width,
                               prompt.height});
        m_input->setGeometry({14, 0, rect.width - 14 - 40, rect.height});
        m_go->setGeometry({rect.width - 4 - 24, centeredStart(rect.height, 24), 24, 24});
    }

private:
    class Pill final : public ui::View
    {
    public:
        explicit Pill(PasswordField *field)
            : m_field(field)
        {
        }

    protected:
        void paint(ui::Painter &painter) override
        {
            // One logical pixel of border as whole device pixels, rounded down
            // as before: 1 at 1.5, 2 at 2.
            const double border = std::max(1.0, std::floor(painter.scale() + 1e-3)) / painter.scale();
            drawPanelRect(painter, localRect(), geometry().height / 2, m_field->m_fill.value(),
                          border, m_field->m_border.value());
        }

    private:
        PasswordField *m_field;
    };

    class GoButton final : public Clickable
    {
    public:
        GoButton(ui::Window *window, PasswordField *field)
            : m_field(field)
            , m_color(window, 120, ui::Animation::Easing::Linear, colorInterpolation(),
                      [this] { requestPaint(); })
            , m_opacity(window, 140, ui::Animation::Easing::Linear, numberInterpolation(), [this] {
                setOpacity(m_opacity.value());
                setVisible(m_opacity.value() > 0);
            })
        {
            setCursorShape(ui::CursorShape::Pointer);
            onHoverChanged = [this] { update(); };
            onClicked = [this] { m_field->m_input->onAccepted(); };
        }

        void update()
        {
            const bool active = m_field->m_enabled && !m_field->m_input->text().empty();
            setEnabled(active);
            m_color.setTarget(white(containsMouse() && active ? 0.32 : 0.22));
            m_opacity.setTarget(active ? 1 : 0.35);
        }

    protected:
        void paint(ui::Painter &painter) override
        {
            drawPanelRect(painter, localRect(), geometry().width / 2, m_color.value());
            const double cx = geometry().width / 2;
            const double cy = geometry().height / 2;
            ui::Path arrow;
            arrow.moveTo({cx - 4, cy});
            arrow.lineTo({cx + 4, cy});
            arrow.moveTo({cx + 1, cy - 3});
            arrow.lineTo({cx + 4, cy});
            arrow.lineTo({cx + 1, cy + 3});
            painter.strokePath(arrow, kWhite, 1.5, ui::LineCap::Round, ui::LineJoin::Round);
        }

    private:
        PasswordField *m_field;
        ColorBehavior m_color;
        NumberBehavior m_opacity;
    };

    // Two rounds of: out to the left, across to the right, back to the middle.
    double shakeAt(double milliseconds) const
    {
        const bool first = milliseconds < 200;
        const double t = first ? milliseconds : milliseconds - 200;
        const double start = first ? m_shakeFrom : 0;
        const auto outQuad = [](double x) { return 1 - (1 - x) * (1 - x); };
        const auto inQuad = [](double x) { return x * x; };
        const auto inOutQuad = [](double x) {
            return x < 0.5 ? 2 * x * x : 1 - std::pow(-2 * x + 2, 2) / 2;
        };
        if (t < 50)
            return start + (-8 - start) * outQuad(t / 50);
        if (t < 150)
            return -8 + 16 * inOutQuad((t - 50) / 100);
        return 8 - 8 * inQuad(std::min(1.0, (t - 150) / 50));
    }

    void setShakeOffset(double offset)
    {
        if (offset == m_shakeOffset)
            return;
        m_shakeOffset = offset;
        requestLayout();
    }

    Pill *m_pill = nullptr;
    ui::Label *m_prompt = nullptr;
    PasswordInput *m_input = nullptr;
    GoButton *m_go = nullptr;
    ColorBehavior m_fill;
    ColorBehavior m_border;
    ui::Animation m_shake;
    double m_shakeOffset = 0;
    double m_shakeFrom = 0;
    bool m_enabled = true;
    bool m_keyboardFocused = false;
};

// The reason an attempt failed, fading in under the field.
class ErrorLabel final : public ui::View
{
public:
    explicit ErrorLabel(ui::Window *window)
        : m_opacity(window, 180, ui::Animation::Easing::OutCubic, numberInterpolation(),
                    [this] { setOpacity(m_opacity.value()); })
    {
        m_label = addChild<RaisedLabel>(13, 400, ui::Color {255, 184, 184, 255},
                                        kBlack.withAlpha(0.35));
        m_label->setElidedCentred();
        m_opacity.setTarget(0);
    }

    void setMessage(const std::string &message, bool shown)
    {
        m_label->setText(message);
        m_opacity.setTarget(shown && !message.empty() ? 1 : 0);
        requestLayout();
    }

    ui::SizeF sizeHint() const override { return {0, 20}; }

protected:
    void layout() override
    {
        const double height = m_label->sizeHint().height;
        m_label->setGeometry({0, centeredStart(geometry().height, height), geometry().width, height});
    }

private:
    NumberBehavior m_opacity;
    RaisedLabel *m_label;
};

AuthScreen::AuthScreen(ui::Window *window, AuthController *controller)
    : m_window(window)
    , m_controller(controller)
{
    setName("auth-screen");
    m_date = addChild<RaisedLabel>(20, 600, kWhite, kBlack.withAlpha(0.3));
    m_date->setName("date");
    m_time = addChild<RaisedLabel>(84, 600, kWhite, kBlack.withAlpha(0.3));
    m_time->setName("time");

    m_pod = addChild<ui::ColumnLayout>();
    m_pod->setSpacing(18);
    // The lists are added after the column, so they open over everything in it.
    ui::View *popups = this;

    const auto centred = [](bool fill) {
        ui::LayoutHints hints;
        hints.horizontalAlignment = ui::Alignment::Center;
        hints.fillWidth = fill;
        return hints;
    };
    m_user = m_pod->addChild<UserView>(window);
    m_user->setName("user");
    m_user->setLayoutHints(centred(false));
    m_userName = m_pod->addChild<UserNameField>();
    m_userName->setName("user-name-field");
    {
        ui::LayoutHints hints;
        hints.fillWidth = true;
        m_userName->setLayoutHints(hints);
    }
    m_password = m_pod->addChild<PasswordField>(window);
    m_password->setName("password-field");
    m_password->setLayoutHints(centred(true));
    m_password->setPlaceholder(tr("Enter Password"));
    m_error = m_pod->addChild<ErrorLabel>(window);
    m_error->setName("error");
    m_error->setLayoutHints(centred(true));
    m_sessions = m_pod->addChild<ChoiceMenu>("session-menu", popups);
    m_sessions->setLayoutHints(centred(false));

    m_picker = addChild<UserGrid>(window);
    m_picker->setName("user-grid");
    m_picker->setVisible(false);
    m_picker->setEnabled(false);
    m_picker->setOpacity(0);
    m_pickerFade = std::make_unique<ui::Animation>(window);
    m_pickerFade->setDuration(std::chrono::milliseconds(180));
    m_pickerFade->setEasing(ui::Animation::Easing::OutCubic);
    m_pickerFade->onUpdate = [this](double t) {
        m_pickerProgress = m_pickerFrom + ((m_pickerOpen ? 1 : 0) - m_pickerFrom) * t;
        updatePicker();
    };

    m_user->avatar()->onClicked = [this] { setPickerOpen(true); };
    m_picker->onActivated = [this](int index) {
        if (index != m_controller->userIndex()) {
            m_controller->setUserIndex(index);
            if (m_controller->failed())
                m_controller->reset();
            m_password->clear();
        }
        setPickerOpen(false);
    };
    m_picker->onCancelled = [this] { setPickerOpen(false); };
    m_sessions->onActivated = [this](int index) {
        m_controller->setSessionIndex(index);
        focusPassword();
    };
    m_password->onAccepted = [this](const std::string &password) {
        m_controller->authenticate(password);
    };
    // Any edit means the user has moved on from the rejection; the error goes
    // away without waiting for another attempt.
    m_password->onEdited = [this] {
        if (m_controller->failed())
            m_controller->reset();
    };
    NameInput *name = m_userName->input();
    name->onTextEdited = [this, name] {
        m_controller->setUserName(name->text());
    };
    name->onAccepted = [this] { focusPassword(); };
    m_window->onPointerPressed = [this](ui::PointF position, ui::MouseButton) {
        closeMenus(position);
    };

    const auto userChanged = [this] {
        m_user->setUser(m_controller->displayName(), m_controller->iconSource());
        m_picker->setCurrentIndex(m_controller->userIndex());
        NameInput *input = m_userName->input();
        const std::string text = m_controller->userName();
        if (input->text() != text)
            input->setText(text);
    };
    const auto usersChanged = [this] {
        m_picker->setUsers(m_controller->userNames(), m_controller->userIcons());
        m_userName->setVisible(m_controller->typesUserName());
        updateSwappable();
    };
    const auto sessionsChanged = [this] {
        m_sessions->setNames(m_controller->sessionNames());
        m_sessions->setCurrentIndex(m_controller->sessionIndex());
        m_sessions->setMenuVisible(m_controller->canSelectSession());
    };
    const auto stateChanged = [this] {
        const bool enabled = !m_controller->busy() && !m_controller->succeeded();
        m_password->setFieldEnabled(enabled);
        m_userName->input()->setEnabled(enabled);
        m_sessions->setMenuEnabled(enabled);
        updateSwappable();
        m_error->setMessage(m_controller->errorMessage(), m_controller->failed());
    };
    m_subscriptions.push_back(m_controller->userChanged.connect(userChanged));
    m_subscriptions.push_back(m_controller->usersChanged.connect(usersChanged));
    m_subscriptions.push_back(m_controller->sessionsChanged.connect(sessionsChanged));
    m_subscriptions.push_back(m_controller->sessionChanged.connect(sessionsChanged));
    m_subscriptions.push_back(m_controller->stateChanged.connect(stateChanged));
    m_subscriptions.push_back(m_controller->authenticationFailed.connect([this] {
        m_password->clear();
        m_password->shake();
        focusPassword();
    }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    usersChanged();
    userChanged();
    sessionsChanged();
    stateChanged();

    m_clockTimer.setSingleShot(true);
    m_clockTimer.onTimeout = [this] { tick(); };
    tick();
}

AuthScreen::~AuthScreen()
{
    m_window->onPointerPressed = nullptr;
}

// The typed name and password, the account and the session stay.
void AuthScreen::retranslate()
{
    m_userName->retranslate();
    m_password->setPlaceholder(tr("Enter Password"));
    tick();
}

void AuthScreen::setBlocked(bool blocked)
{
    if (blocked == m_blocked)
        return;
    m_blocked = blocked;
    tick();
}

void AuthScreen::setKeyboardFocused(bool focused)
{
    m_password->setKeyboardFocused(focused);
}

void AuthScreen::focusPassword()
{
    if (m_pickerOpen)
        return;
    // The keyboard is the compositor's to give; when it arrives it belongs in
    // the password field and nowhere else.
    m_password->input()->setFocus();
    m_password->update();
}

base::RectF AuthScreen::fieldBounds() const
{
    if (!m_pod->isVisible() || !m_password->isVisible())
        return {};
    const ui::RectF pill = m_password->pillRect();
    const ui::PointF origin = m_password->mapToWindow({pill.x, pill.y});
    return base::RectF(origin.x, origin.y, pill.width, pill.height);
}

double AuthScreen::fieldOpacity() const
{
    return 1 - m_pickerProgress;
}

bool AuthScreen::canSwitchUser() const
{
    return m_controller->canPickUser() && !m_controller->busy() && !m_controller->succeeded();
}

void AuthScreen::updateSwappable()
{
    m_user->avatar()->setSwappable(canSwitchUser());
    if (!canSwitchUser())
        setPickerOpen(false);
}

void AuthScreen::setPickerOpen(bool open)
{
    if (open == m_pickerOpen || (open && !canSwitchUser()))
        return;
    m_pickerOpen = open;
    // The side going away takes no input while it fades.
    m_pod->setEnabled(!open);
    m_picker->setEnabled(open);
    if (open) {
        m_sessions->setOpen(false);
        m_picker->reset();
    }
    updatePicker();
    if (open)
        m_picker->setFocus();
    else
        focusPassword();
    m_pickerFrom = m_pickerProgress;
    m_pickerFade->stop();
    m_pickerFade->start();
}

void AuthScreen::updatePicker()
{
    m_pod->setOpacity(1 - m_pickerProgress);
    m_pod->setVisible(!m_pickerOpen || m_pickerProgress < 1);
    m_picker->setOpacity(m_pickerProgress);
    m_picker->setVisible(m_pickerOpen || m_pickerProgress > 0);
}

void AuthScreen::setClockForTesting(std::time_t now)
{
    m_fixedClock = now;
    tick();
}

void AuthScreen::tick()
{
    timespec now {};
    clock_gettime(CLOCK_REALTIME, &now);
    const std::time_t seconds = m_fixedClock.value_or(now.tv_sec);
    const long milliseconds = m_fixedClock ? 0 : now.tv_nsec / 1000000;
    std::tm local {};
    localtime_r(&seconds, &local);
    const base::Locale &locale = base::Locale::system();
    m_date->setText(locale.toString(
            base::CalendarDate {local.tm_year + 1900, local.tm_mon + 1, local.tm_mday}, "ddd MMM d"));
    m_time->setText(locale.toString(base::ClockTime {local.tm_hour, local.tm_min}, "hh:mm"));
    requestLayout();
    // Just past the next minute, so the time never trails the real one.
    if (m_blocked)
        m_clockTimer.start(
                std::chrono::milliseconds(60000 - local.tm_sec * 1000 - milliseconds + 20));
    else
        m_clockTimer.stop();
}

void AuthScreen::closeMenus(ui::PointF position)
{
    m_sessions->pressedAt(position);
    if (m_pickerOpen) {
        const ui::PointF local = m_picker->mapFromWindow(position);
        if (!m_picker->localRect().contains(local))
            setPickerOpen(false);
    }
}

void AuthScreen::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    const ui::SizeF date = m_date->sizeHint();
    const ui::SizeF time = m_time->sizeHint();
    // A Column of the two, 4 closer than touching, centred at the top.
    const double column = std::max(date.width, time.width);
    const double left = centeredStart(width, column);
    m_date->setGeometry({left + centeredStart(column, date.width), 56, date.width, date.height});
    m_time->setGeometry({left + centeredStart(column, time.width), 56 + date.height - 4, time.width,
                         time.height});

    const double podWidth = std::min(width - 48, 240.0);
    const double podHeight = m_pod->verticalExtent(podWidth).preferred;
    m_pod->setGeometry({centeredStart(width, podWidth), height - 64 - podHeight, podWidth, podHeight});

    // The accounts' pictures in the row of the one picked now, as long as
    // they fit above the bottom margin.
    const ui::SizeF grid = m_picker->sizeFor(width - 48);
    const double gridTop = std::min(height - 64 - podHeight - UserTile::kHalo, height - 64 - grid.height);
    m_picker->setGeometry({centeredStart(width, grid.width), gridTop, grid.width, grid.height});
}

// The menu's list lies outside the pod, by its menu wherever the pod placed it.
void AuthScreen::childrenLaidOut()
{
    m_sessions->placeList();
}

} // namespace Lingmo::Auth
