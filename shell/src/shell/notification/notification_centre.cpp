// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_centre.h"
#include "shell/notification/notification_widgets.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "base/i18n.h"
#include "core/main_loop.h"
#include "shell/notification/notification_card.h"
#include "shell/notification/notification_controls.h"
#include "shell/notification/notification_models.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_host.h"
#include "shell/shell_window.h"
#include "ui/application.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "toolkit/compositor_colors.h"

using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace Lingmo::Shell {

// PanelStyle.tileColor: the cards' plates.
static ui::Color cardColor()
{
    return ui::color(toolkit::CompositorColor::NotificationCentreCard);
}

// PanelStyle.tileInkColor: what the cards read as, opaque, for their ink.
static ui::Color cardInkColor()
{
    return ui::Theme::current().secondBackground;
}

static double smallSpacing()
{
    return ui::Theme::current().smallSpacing;
}

// On whole device pixels while gliding, so text lands on the same subpixel
// phase every frame and draws from its cached coverage.
static double pixelAligned(const Tween &tween)
{
    if (!tween.isTicking() || !tween.window())
        return tween.value();
    const double scale = tween.window()->scale();
    return std::round(tween.value() * scale) / scale;
}

// "%n more notification(s)".
static std::string moreText(int count)
{
    return trn("{n} more notification", "{n} more notifications", count);
}

// ---------------------------------------------------------------------------
// Row

// The row's sliding part: the peeking strips of a stack and the card's frame.
class NotificationCentre::Row::Content : public ui::View
{
public:
    explicit Content(Row *row)
        : m_row(row)
    {
    }

protected:
    void paint(ui::Painter &painter) override
    {
        // Only the strip below the card is drawn, so the translucent plates
        // never overlap one another.
        const double width = geometry().width;
        const double frame = m_row->frameHeight();
        const ui::RoundedShape shape {kRadius, 0, ui::themeCorners()};
        for (int index = 0; index < 2; ++index) {
            const double opacity = m_row->peekOpacity(index);
            if (opacity <= 0)
                continue;
            const int level = index + 1;
            const double x = level * ui::Theme::current().largeSpacing;
            const ui::RectF strip {x, frame + index * kPeekStep, width - x * 2, kPeekStep};
            painter.save();
            painter.multiplyOpacity(opacity);
            painter.clip(strip);
            painter.drawRoundedRect({strip.x, strip.bottom() - frame, strip.width, frame}, shape, cardColor());
            painter.restore();
        }
    }

private:
    Row *m_row;
};

// The card's plate, its contents clipped to its own rect.
class CardFrame : public ui::View
{
public:
    CardFrame() { setClipsChildren(true); }

protected:
    void paint(ui::Painter &painter) override
    {
        painter.drawRoundedRect(localRect(), {NotificationCentre::kRadius, 0, ui::themeCorners()}, cardColor());
    }
};

NotificationCentre::Row::Row(NotificationCentre *centre, ui::Window *window)
    : m_centre(centre)
    , m_y(window)
    , m_contentY(window)
    , m_opacity(window)
    , m_scale(window)
    , m_frameHeight(window)
    , m_goneTimer(core::mainLoop())
{
    m_content = addChild<Content>(this);
    m_frame = m_content->addChild<CardFrame>();
    m_card = m_frame->addChild<NotificationCard>(window);
    m_card->setSurfaceColor(cardInkColor());
    for (auto &peek : m_peeks) {
        peek = std::make_unique<Tween>(window);
        peek->onChanged = [this] { m_content->requestPaint(); };
        peek->snap(0);
    }
    m_opacity.snap(1);
    m_scale.snap(1);

    m_y.onChanged = [this] { place(); };
    m_contentY.onChanged = [this] { placeContent(); };
    m_opacity.onChanged = [this] { placeContent(); };
    m_scale.onChanged = [this] { placeContent(); };
    m_frameHeight.onChanged = [this] { placeContent(); };

    m_card->onClicked = [this] {
        if (isStacked())
            m_centre->setExpanded(m_groupKey, true);
        else
            m_centre->toggleDetail(this);
    };
    m_card->onCloseClicked = [this] {
        if (isStacked())
            m_centre->dismissGroup(m_groupKey);
        else
            m_centre->dismissRows([this](const Row &row) { return &row == this; });
    };
    // Deferred: the layout pass writes what the target height reads.
    m_card->onImplicitHeightChanged = [this] { m_centre->scheduleLayout(); };

    m_goneTimer.setSingleShot(true);
    m_goneTimer.setInterval(kDismissFade);
    m_goneTimer.onTimeout = [this] {
        gone = true;
        // Faded out: the layout closes the gap.
        m_centre->relayout();
    };
}

NotificationCentre::Row::~Row() = default;

void NotificationCentre::Row::setNotification(const Notification &notification, const std::string &timeText)
{
    m_notification = notification;
    m_groupKey = notification.groupKey();
    // The centre keeps the progress bar, such as an update's, but not the
    // actions: those belong to the banner the user can still act on.
    Notification shown = notification;
    shown.actions.clear();
    m_card->setNotification(shown);
    m_card->setTimeText(timeText);
}

int NotificationCentre::Row::peekCount() const
{
    return isStacked() ? std::min(stackCount, 2) : 0;
}

// The cards are as wide as the list less its insets.
static constexpr double kCardWidth = NotificationCentre::kWidth - NotificationCentre::kInset * 2;

double NotificationCentre::Row::contentHeight() const
{
    return m_card->implicitHeight(kCardWidth) + peekCount() * kPeekStep;
}

double NotificationCentre::Row::targetHeight() const
{
    return isOccupying() ? contentHeight() + smallSpacing() * 1.5 : 0;
}

void NotificationCentre::Row::setDetailed(bool detailed)
{
    m_card->setDetailed(detailed);
}

bool NotificationCentre::Row::isDetailed() const
{
    return m_card->isDetailed();
}

void NotificationCentre::Row::moveTo(double y, bool animate)
{
    m_y.setTarget(y, animate);
    place();
}

bool NotificationCentre::Row::isSeenWithin(double top, double bottom, double y) const
{
    const double height = contentHeight();
    const auto overlaps = [top, bottom, height](double at) { return at < bottom && at + height > top; };
    if (m_opacity.value() > 0 && overlaps(m_y.value() + m_contentY.value()))
        return true;
    return (isShown() || dismissing) && overlaps(y);
}

void NotificationCentre::Row::apply(bool animated)
{
    const bool shown = isShown();

    const std::chrono::milliseconds fade = dismissing ? kDismissFade : kMotionDuration;
    m_opacity.setDuration(fade);
    m_scale.setDuration(fade);
    // Hidden, the content sits just above the row, behind the card before
    // it, so showing it slides it down out of the stack.
    m_contentY.setTarget(shown || dismissing ? 0 : -contentHeight(), animated);
    m_opacity.setTarget(shown ? 1 : 0, animated);
    m_scale.setTarget(shown ? 1 : dismissing ? 0.92 : 0.94, animated);
    m_bottomOrigin = !dismissing;

    for (int index = 0; index < 2; ++index) {
        const int level = index + 1;
        m_peeks[index]->setTarget(index < peekCount() ? 1 - level * 0.3 : 0, animated);
    }
    m_frameHeight.setTarget(m_card->implicitHeight(kCardWidth), animated);
    m_card->setConfirmClose(isStacked());
    m_card->setMoreText(isStacked() ? moreText(stackCount) : "");
    setEnabled(shown);
    place();
}

void NotificationCentre::Row::startGoneTimer()
{
    m_goneTimer.start();
}

void NotificationCentre::Row::disarmUnless(ui::PointF windowPoint)
{
    m_card->disarmUnless(windowPoint);
}

void NotificationCentre::Row::place()
{
    setGeometry({0, pixelAligned(m_y), double(kWidth), targetHeight()});
    placeContent();
}

void NotificationCentre::Row::placeContent()
{
    const double width = kCardWidth;
    const double height = m_card->implicitHeight(width) + peekCount() * kPeekStep;
    m_content->setGeometry({kInset, pixelAligned(m_contentY), width, height});
    m_content->setOpacity(m_opacity.value());
    const double scale = m_scale.value();
    ui::Transform transform;
    if (scale != 1) {
        transform.scale = scale;
        // Scaled about the bottom centre rather than the centre.
        if (m_bottomOrigin)
            transform.translation = {0, height / 2 * (1 - scale)};
    }
    m_content->setTransform(transform);
    m_frame->setGeometry({0, 0, width, m_frameHeight.value()});
    m_card->setGeometry({0, 0, width, m_card->implicitHeight(width)});
    m_content->requestPaint();
}

// ---------------------------------------------------------------------------
// GroupHeader

NotificationCentre::GroupHeader::GroupHeader(NotificationCentre *centre, ui::Window *window, std::string groupKey)
    : m_centre(centre)
    , m_groupKey(std::move(groupKey))
    , m_y(window)
    , m_opacity(window)
{
    m_label = addChild<ui::Label>();
    m_label->setWeight(600);
    m_showLess = addChild<PillButton>(tr("Show Less"));
    m_showLess->onClicked = [this] { m_centre->setExpanded(m_groupKey, false); };
    m_clear = addChild<ClearButton>(window, ClearButton::Kind::Clear);
    m_clear->onClicked = [this] { m_centre->dismissGroup(m_groupKey); };
    m_clear->onWidthChanged = [this] { requestLayout(); };

    m_opacity.snap(0);
    m_y.onChanged = [this] { place(); };
    m_opacity.onChanged = [this] { place(); };
    setVisible(false);
    setEnabled(false);
}

NotificationCentre::GroupHeader::~GroupHeader() = default;

void NotificationCentre::GroupHeader::setAppName(const std::string &name)
{
    m_label->setText(name);
}

void NotificationCentre::GroupHeader::retranslate()
{
    m_showLess->setText(tr("Show Less"));
    requestLayout();
}

double NotificationCentre::GroupHeader::targetHeight() const
{
    return kGroupHeaderHeight + smallSpacing() * 1.5;
}

void NotificationCentre::GroupHeader::setShown(bool shown)
{
    m_shown = shown;
    // A faded header still sits over the stack that took its place:
    // disabled, it leaves the pointer to the stack.
    setEnabled(shown);
    m_opacity.setTarget(shown ? 1 : 0, m_centre->m_motion);
    place();
}

void NotificationCentre::GroupHeader::moveTo(double y, bool animate)
{
    m_y.setTarget(y, animate);
    place();
}

void NotificationCentre::GroupHeader::disarmUnless(ui::PointF windowPoint)
{
    m_clear->disarmUnless(windowPoint);
}

void NotificationCentre::GroupHeader::place()
{
    setGeometry({0, pixelAligned(m_y), double(kWidth), targetHeight()});
    setOpacity(m_opacity.value());
    const bool visible = m_shown || m_opacity.value() > 0;
    if (visible != isVisible()) {
        setVisible(visible);
        if (!visible)
            m_clear->reset();
    }
}

void NotificationCentre::GroupHeader::layout()
{
    const double width = geometry().width;
    const double spacing = smallSpacing();
    const double clearWidth = m_clear->currentWidth();
    const double clearX = width - kInset - clearWidth;
    m_clear->setGeometry({clearX, 0, clearWidth, m_clear->buttonHeight()});
    const double pillWidth = m_showLess->preferredWidth();
    const double pillX = clearX - spacing - pillWidth;
    m_showLess->setGeometry({pillX, 0, pillWidth, 26});
    const double left = kInset + spacing;
    const ui::SizeF size = m_label->sizeHint();
    m_label->setGeometry({left, (kGroupHeaderHeight - size.height) / 2, std::max(0.0, pillX - spacing - left),
                          size.height});
}

// ---------------------------------------------------------------------------
// List and window content

// The scrolled content: rows and headers where the layout pass put them,
// as tall as the animated bottom of the list plus the bottom margin.
class NotificationCentre::List : public ui::View
{
public:
    List(NotificationCentre *centre, ui::Window *window)
        : m_centre(centre)
        , m_bottom(window)
    {
        m_bottom.onChanged = [this] {
            if (parent())
                parent()->requestLayout();
            requestLayout();
        };
    }

    void setTargetBottom(double bottom) { m_bottom.setTarget(bottom, m_centre->m_motion); }
    double bottom() const { return m_bottom.value(); }

    double heightForWidth(double width) const override {
        auto* widgets=m_centre->m_widgets;
        const double widgetHeight=widgets && widgets->isVisible()?widgets->heightForWidth(std::max(0.0,width-12)):0;
        return m_bottom.value() + (widgetHeight>0?widgetHeight+24:0) + ui::Theme::current().largeSpacing;
    }
    void layout() override {
        if(auto* widgets=m_centre->m_widgets; widgets && widgets->isVisible())
            widgets->setGeometry({6,m_bottom.value()+16,std::max(0.0,geometry().width-12),widgets->heightForWidth(std::max(0.0,geometry().width-12))});
    }
    ui::SizeF sizeHint() const override { return {0, heightForWidth(0)}; }

private:
    NotificationCentre *m_centre;
    Tween m_bottom;
};

class NotificationCentre::Content : public ui::View
{
public:
    explicit Content(NotificationCentre *centre)
        : m_centre(centre)
    {
    }

protected:
    void layout() override
    {
        const ui::Theme &theme = ui::Theme::current();
        const double width = geometry().width;
        const double top = theme.largeSpacing;
        ClearButton *clear = m_centre->m_clear;
        const double clearWidth = clear->currentWidth();
        const double clearX = width - kInset - clearWidth;
        clear->setGeometry({clearX, top + (kHeaderHeight - clear->buttonHeight()) / 2, clearWidth,
                            clear->buttonHeight()});

        ui::Label *title = m_centre->m_title;
        title->setFontSize(14.0 * 4 / 3);
        const ui::SizeF size = title->sizeHint();
        title->setGeometry({kInset, top + (kHeaderHeight - size.height) / 2,
                            std::max(0.0, clearX - theme.largeSpacing - kInset), size.height});

        const double listTop = top + kHeaderHeight + theme.largeSpacing;
        const ui::RectF list {0, listTop, width, std::max(0.0, geometry().height - listTop)};
        m_centre->m_scroll->setGeometry(list);

        ui::Label *empty = m_centre->m_empty;
        empty->setFontSize(13.0 * 4 / 3);
        empty->setColor(ui::color(toolkit::CompositorColor::NotificationCentreEmptyText));
        const ui::SizeF emptySize = empty->sizeHint();
        empty->setGeometry({list.x + (list.width - emptySize.width) / 2,
                            m_centre->m_widgets && m_centre->m_widgets->isVisible() ? list.y+12 : list.y + (list.height - emptySize.height) / 2, emptySize.width, emptySize.height});
    }

    void paint(ui::Painter &) override
    {
        const uint64_t generation = ui::Theme::generation();
        if (generation == m_theme)
            return;
        m_theme = generation;
        m_centre->applyGlass();
        for (Row *row : m_centre->m_rows)
            row->card()->setSurfaceColor(cardInkColor());
    }

private:
    NotificationCentre *m_centre;
    uint64_t m_theme = 0;
};

// ---------------------------------------------------------------------------
// NotificationCentre

NotificationCentre::NotificationCentre(Shell *shell, NotificationHistory &history, NotificationBanners &banners)
    : m_shell(shell)
    , m_history(history)
    , m_banners(banners)
    , m_motionTimer(core::mainLoop())
    , m_removalTimer(core::mainLoop())
    , m_timeTimer(core::mainLoop())
{
    m_motionTimer.setSingleShot(true);
    m_motionTimer.onTimeout = [this] { m_motion = false; };
    m_removalTimer.setSingleShot(true);
    m_removalTimer.setInterval(kRemovalDelay);
    m_removalTimer.onTimeout = [this] { commitRemovals(); };
    // Relative times go stale while it is open.
    m_timeTimer.setInterval(std::chrono::minutes(1));
    m_timeTimer.onTimeout = [this] { m_history.updateTime(); };

    m_subscriptions.push_back(history.rowsInserted.connect([this](int first, int count) { insertRows(first, count); }));
    m_subscriptions.push_back(history.rowsRemoved.connect([this](int first, int count) { removeRows(first, count); }));
    m_subscriptions.push_back(history.rowMoved.connect([this](int from, int to) { moveRow(from, to); }));
    m_subscriptions.push_back(history.dataChanged.connect([this](int first, int last) { updateRows(first, last); }));
    m_subscriptions.push_back(history.modelReset.connect([this] { resetRows(); }));
    m_subscriptions.push_back(shell->windowManager()->outputsChanged.connect([this] {
        if (!m_open || !m_parent)
            return;
        // The work area changed under it: as tall as the new one.
        const Placement placement = placementFor(m_parent, m_placedGeometry.topLeft());
        if (placement.geometry.size() != m_geometry.size()) {
            m_geometry = placement.geometry;
            m_window->resize({double(m_geometry.width()), double(m_geometry.height())});
        }
    }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
}

NotificationCentre::~NotificationCentre()
{
    *m_alive = false;
    m_subscriptions.clear();
    if (m_open)
        m_banners.setBannersSuppressed(false);
    destroyWindow();
}

ShellWindow *NotificationCentre::shellWindow() const
{
    if (!m_window || !m_open)
        return nullptr;
    return m_shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(m_window.get()));
}

NotificationCentre::Placement NotificationCentre::placementFor(ShellWindow *parent, const base::Point &position) const
{
    Placement placement;
    placement.parent = parent;
    const Shell::Output *output = m_shell->output(parent ? parent->outputName() : std::string());
    if (!output)
        return placement;
    const base::Rect screen = output->bounds;
    const base::Rect area = m_shell->workArea(*output);
    int x = position.x();
    if (x + kWidth >= screen.x() + screen.width())
        x = screen.x() + screen.width() - kWidth - kMargin;
    placement.geometry = base::Rect(x, area.y() + kMargin, kWidth, area.height() - kMargin * 2);
    return placement;
}

void NotificationCentre::toggle(ShellWindow *parent, const base::Point &position)
{
    if (m_open)
        close();
    else
        open(parent, position);
}

void NotificationCentre::open(ShellWindow *parent, const base::Point &position)
{
    if (!parent || !parent->window() || !parent->isShown())
        return;
    const Placement placement = placementFor(parent, position);
    if (placement.geometry.isEmpty())
        return;
    if (m_open) {
        if (placement.parent == m_parent && placement.geometry == m_geometry)
            return;
        close();
    }
    // A popup's place is fixed when it is made: another place takes a new one.
    if (!m_window || placement.parent != m_parent || placement.geometry != m_geometry)
        build(placement);
    m_placedGeometry = base::Rect(position, base::Size(1, 1));

    m_open = true;
    if(m_widgets)NotificationWidgetProvider::instance().active(m_widgets,true);
    relayout();
    // With the centre open the user is looking at every notification already.
    m_banners.setBannersSuppressed(true);
    m_history.updateTime();
    m_scroll->setScrollPosition({0, 0});
    m_window->show();
    applyGlass();
    m_timeTimer.start();
    openChanged();
}

void NotificationCentre::close()
{
    if (!m_open)
        return;
    m_open = false;
    if(m_widgets)NotificationWidgetProvider::instance().active(m_widgets,false);
    m_timeTimer.stop();
    if (m_window)
        m_window->hide();
    onHidden();
    m_banners.setBannersSuppressed(false);
    openChanged();
}

void NotificationCentre::onHidden()
{
    // Everything folds back up while hidden, with no motion, so the next
    // opening shows the settled list straight away.
    m_motion = false;
    m_motionTimer.stop();
    if (m_removalTimer.isActive()) {
        m_removalTimer.stop();
        commitRemovals();
    }
    disarmClears(std::nullopt);
    m_expanded.clear();
    for (Row *row : m_rows)
        row->setDetailed(false);
    relayout();
    pruneHeaders();
}

// The list stays as it is, open or not: the labels, the stacks' counts and
// the relative times are taken again. The clear buttons follow on their own.
void NotificationCentre::retranslate()
{
    if (!m_window)
        return;
    m_title->setText(tr("Notification Center"));
    m_empty->setText(tr("No Notifications"));
    if(m_widgets && m_open)NotificationWidgetProvider::instance().active(m_widgets,true);
    for (GroupHeader *header : m_headers)
        header->retranslate();
    m_history.updateTime();
    m_content->requestLayout();
    relayout();
}

void NotificationCentre::destroyWindow()
{
    m_rows.clear();
    m_headers.clear();
    m_content = nullptr;
    m_title = nullptr;
    m_clear = nullptr;
    m_scroll = nullptr;
    m_list = nullptr;
    m_empty = nullptr;
    m_widgets = nullptr;
    m_window.reset();
    m_parent = nullptr;
}

void NotificationCentre::build(const Placement &placement)
{
    destroyWindow();
    m_parent = placement.parent;
    m_geometry = placement.geometry;

    const base::Rect parent = m_parent->geometry();
    ui::PopupPlacement popup;
    popup.anchorRect = {double(m_geometry.x() - parent.x()), double(m_geometry.y() - parent.y()), 1, 1};
    popup.anchor = ui::PopupAnchor::TopLeft;
    popup.gravity = ui::PopupAnchor::BottomRight;
    popup.flipX = popup.flipY = popup.slide = false;
    m_window = ui::Window::createPopup(m_parent->window(), popup, true);
    ui::Window *window = m_window.get();

    auto content = std::make_unique<Content>(this);
    m_content = content.get();
    m_title = content->addChild<ui::Label>(tr("Notification Center"));
    m_clear = content->addChild<ClearButton>(window, ClearButton::Kind::Clear);
    m_clear->onClicked = [this] { dismissRows([](const Row &) { return true; }); };
    m_clear->onWidthChanged = [this] { m_content->requestLayout(); };
    m_scroll = content->addChild<ui::ScrollView>();
    m_scroll->setElastic(false);
    m_list = m_scroll->setContent<List>(this, window);
    if(const auto& provider=NotificationWidgetProvider::instance();provider.create) {
        auto widgets=std::unique_ptr<ui::View>(provider.create());m_widgets=widgets.get();m_widgets->setVisible(false);m_list->addChild(std::move(widgets));
    }
    m_empty = content->addChild<ui::Label>(tr("No Notifications"));
    m_empty->setElide(ui::Label::Elide::None);
    m_window->setContentView(std::move(content));
    m_window->resize({double(m_geometry.width()), double(m_geometry.height())});

    m_window->onCloseRequested = [this] { close(); };
    m_window->addShortcut(0, ui::key::Escape, [this] { close(); });
    // A press anywhere backs every armed clear button out but the one under it.
    m_window->onPointerPressed = [this](ui::PointF position, ui::MouseButton) { disarmClears(position); };

    resetRows();
}

void NotificationCentre::applyGlass()
{
    ShellWindow *window = shellWindow();
    if (!window)
        return;
    // PanelGlass: PopupGlass with a heavier blur and saturation, tinted
    // PanelStyle.panelTint.
    const bool dark = ui::Theme::current().darkMode;
    render::GlassParams glass = dark ? popupGlass(0.13, 0.13, 0.14, 0.86) : popupGlass(0.94, 0.94, 0.95, 0.86);
    glass.blurSigma = glassFixed(20.0);
    glass.saturation = glassFixed(2.4);
    window->setBackgroundBlur(base::Rect(0, 0, m_geometry.width(), m_geometry.height()), int(kRadius), glass);
}

// --- The model's rows -------------------------------------------------------

NotificationCentre::Row *NotificationCentre::makeRow(int index)
{
    auto row = std::make_unique<Row>(this, m_window.get());
    row->setNotification(m_history.notifications()[size_t(index)], m_history.createdText(index));
    Row *view = row.get();
    m_list->addChild(std::move(row));
    view->place();
    return view;
}

void NotificationCentre::insertRows(int first, int count)
{
    if (!m_window)
        return;
    for (int index = first; index < first + count; ++index)
        m_rows.insert(m_rows.begin() + index, makeRow(index));
    scheduleLayout();
}

void NotificationCentre::removeRows(int first, int count)
{
    if (!m_window)
        return;
    for (int index = first; index < first + count && index < int(m_rows.size()); ++index)
        m_list->takeChild(m_rows[size_t(index)]);
    m_rows.erase(m_rows.begin() + first, m_rows.begin() + std::min(int(m_rows.size()), first + count));
    scheduleLayout();
}

void NotificationCentre::moveRow(int from, int to)
{
    if (!m_window || from < 0 || from >= int(m_rows.size()) || to < 0 || to >= int(m_rows.size()))
        return;
    Row *row = m_rows[size_t(from)];
    m_rows.erase(m_rows.begin() + from);
    m_rows.insert(m_rows.begin() + to, row);
    scheduleLayout();
}

void NotificationCentre::updateRows(int first, int last)
{
    if (!m_window)
        return;
    for (int index = first; index <= last && index < int(m_rows.size()); ++index)
        m_rows[size_t(index)]->setNotification(m_history.notifications()[size_t(index)],
                                               m_history.createdText(index));
}

void NotificationCentre::resetRows()
{
    if (!m_window)
        return;
    for (Row *row : m_rows)
        m_list->takeChild(row);
    m_rows.clear();
    for (int index = 0; index < m_history.count(); ++index)
        m_rows.push_back(makeRow(index));
    scheduleLayout();
}

// --- Layout -----------------------------------------------------------------

void NotificationCentre::scheduleLayout()
{
    // Once, after what is going on now.
    if (m_layoutScheduled)
        return;
    m_layoutScheduled = true;
    core::mainLoop()->post([this, alive = m_alive] {
        if (!*alive)
            return;
        m_layoutScheduled = false;
        relayout();
    });
}

void NotificationCentre::relayout()
{
    if (!m_window)
        return;
    // Rows only ever move towards the targets set here, so every row,
    // header and the scroll extent start and land together.
    if (m_inLayout) {
        m_layoutDirty = true;
        return;
    }
    m_inLayout = true;
    int passes = 0;
    do {
        m_layoutDirty = false;
        layoutPass();
    } while (m_layoutDirty && ++passes < 4);
    m_inLayout = false;
}

void NotificationCentre::layoutPass()
{
    // `live` rows stay; `present` ones still take space, fading ones included.
    std::map<std::string, int> live;
    std::map<std::string, int> present;
    for (Row *row : m_rows) {
        if (!row->dismissing)
            ++live[row->groupKey()];
        if (!row->gone)
            ++present[row->groupKey()];
    }

    // Rows glide only where they can be seen; the rest go straight to where
    // they end, so a long group opens at the cost of the cards in view.
    const double viewTop = m_scroll->scrollPosition().y;
    const double viewBottom = viewTop + m_scroll->geometry().height;

    double y = 0;
    std::map<std::string, bool> leads;
    std::set<std::string> headers;
    int liveTotal = 0;

    for (size_t i = 0; i < m_rows.size(); ++i) {
        Row *row = m_rows[i];
        const std::string &key = row->groupKey();
        const int count = live.count(key) ? live[key] : 0;
        const bool expanded = count > 1 && isExpanded(key);

        if (!leads.count(key)) {
            leads[key] = false;
            // The header keeps its slot until its rows have faded.
            if (isExpanded(key) && present[key] > 1) {
                GroupHeader *header = headerFor(key);
                header->setAppName(row->notification().appName);
                if (!header->isShown() && !header->isVisible())
                    header->placed = false;
                header->moveTo(y, m_motion && header->placed);
                header->placed = true;
                header->setShown(expanded);
                headers.insert(key);
                y += header->targetHeight();
            }
        }

        const bool lead = !row->dismissing && !leads[key];
        if (lead)
            leads[key] = true;

        // A row on its way out keeps looking as it did when it left.
        if (!row->dismissing) {
            row->lead = lead;
            row->groupExpanded = expanded;
            row->stackCount = lead ? count - 1 : 0;
        }
        const bool animate = m_motion && row->placed && row->isSeenWithin(viewTop, viewBottom, y);
        row->apply(animate);
        row->moveTo(y, animate);
        row->placed = true;

        y += row->targetHeight();
        if (!row->gone)
            ++liveTotal;
    }

    for (GroupHeader *header : m_headers) {
        if (!headers.count(header->groupKey()))
            header->setShown(false);
    }

    // Later rows pass behind earlier ones as they slide in and out; headers
    // above them all.
    for (auto it = m_rows.rbegin(); it != m_rows.rend(); ++it)
        m_list->raiseChild(*it);
    for (GroupHeader *header : m_headers)
        m_list->raiseChild(header);

    m_list->setTargetBottom(liveTotal==0 && m_widgets && m_widgets->isVisible()?48:y);
    m_scroll->requestLayout();
    m_liveCount = liveTotal;
    m_clear->setVisible(liveTotal > 0);
    if (!m_clear->isVisible())
        m_clear->reset();
    m_empty->setVisible(liveTotal == 0);
}

NotificationCentre::GroupHeader *NotificationCentre::headerFor(const std::string &key)
{
    for (GroupHeader *header : m_headers) {
        if (header->groupKey() == key)
            return header;
    }
    auto header = std::make_unique<GroupHeader>(this, m_window.get(), key);
    GroupHeader *view = header.get();
    m_list->addChild(std::move(header));
    m_headers.push_back(view);
    return view;
}

void NotificationCentre::pruneHeaders()
{
    for (auto it = m_headers.begin(); it != m_headers.end();) {
        if ((*it)->isShown()) {
            ++it;
            continue;
        }
        m_list->takeChild(*it);
        it = m_headers.erase(it);
    }
}

void NotificationCentre::setExpanded(const std::string &key, bool expanded)
{
    beginMotion();
    if (expanded)
        m_expanded.insert(key);
    else
        m_expanded.erase(key);
    relayout();
}

void NotificationCentre::beginMotion(std::chrono::milliseconds duration)
{
    m_motion = true;
    std::chrono::nanoseconds interval = duration;
    if (m_motionTimer.isActive())
        interval = std::max(interval, m_motionTimer.interval());
    m_motionTimer.start(interval);
}

// Rows animate out first and leave the model only once they take no
// space, so the removal itself moves nothing.
void NotificationCentre::dismissRows(const std::function<bool(const Row &)> &predicate)
{
    // Fade in place, then close the gap: the removal timer covers both.
    beginMotion(kRemovalDelay + std::chrono::milliseconds(50));
    for (Row *row : m_rows) {
        if (!row->dismissing && predicate(*row)) {
            row->dismissing = true;
            row->apply(m_motion);
            row->startGoneTimer();
        }
    }
    relayout();
    m_removalTimer.start();
}

void NotificationCentre::dismissGroup(const std::string &key)
{
    dismissRows([&key](const Row &row) { return row.groupKey() == key; });
}

void NotificationCentre::toggleDetail(Row *row)
{
    beginMotion();
    row->setDetailed(!row->isDetailed());
    relayout();
}

void NotificationCentre::commitRemovals()
{
    std::vector<int> indexes;
    bool everything = true;
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i]->dismissing)
            indexes.push_back(int(i));
        else
            everything = false;
    }
    if (indexes.empty())
        return;
    if (everything)
        m_history.clearAll();
    else
        m_history.remove(indexes);

    std::set<std::string> groups;
    for (Row *row : m_rows) {
        if (isExpanded(row->groupKey()))
            groups.insert(row->groupKey());
    }
    m_expanded = std::move(groups);
}

void NotificationCentre::disarmClears(std::optional<ui::PointF> windowPoint)
{
    if (!m_window)
        return;
    const ui::PointF point = windowPoint.value_or(ui::PointF {-1e9, -1e9});
    m_clear->disarmUnless(point);
    for (Row *row : m_rows)
        row->disarmUnless(point);
    for (GroupHeader *header : m_headers)
        header->disarmUnless(point);
}

} // namespace Lingmo::Shell
