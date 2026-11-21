// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/overview/overview_controller.h"

#include <linux/input-event-codes.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

#include "base/i18n.h"
#include "input/cursor_shape.h"
#include "input/seat.h"
#include "scene/window_animation.h"
#include "wm/desks/desk.h"
#include "wm/desks/desk_animation.h"
#include "wm/mru_window_tracker.h"
#include "wm/overview/overview_layout.h"
#include "wm/show_desktop_controller.h"
#include "wm/stacking.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"
namespace scene = Lingmo::Scene;

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace Lingmo::WM {

namespace {

const scene::WindowAnimation *animationOf(const Window *window)
{
    return window ? dynamic_cast<const scene::WindowAnimation *>(window->node()) : nullptr;
}

Window *transientRoot(Window *window)
{
    while (window && window->transientParent())
        window = window->transientParent();
    return window;
}

double distance(const base::PointF &a, const base::PointF &b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

// A preview takes the pointer on its right and bottom edges too; a rect of
// no width and no height takes nothing.
bool containsWithEdges(const base::RectF &rect, const base::PointF &point)
{
    if (rect.width() == 0 && rect.height() == 0)
        return false;
    return point.x() >= rect.left() && point.x() <= rect.right() && point.y() >= rect.top()
            && point.y() <= rect.bottom();
}

} // namespace

OverviewController::OverviewController(WindowManager *manager)
    : m_manager(manager)
{
    m_manager->activation()->addObserver(this);
    m_manager->desks()->addObserver(this);
    m_windowAdded = m_manager->windowAdded.connect([this](Window *) {
        if (isActive())
            rebuild(true);
    });
    // The window is out of the window manager's lists by now.
    m_windowRemoved = m_manager->windowRemoved.connect([this](Window *window) {
        if (!isActive())
            return;
        if (window == m_swipeTarget)
            m_swipeTarget = nullptr;
        rebuild(true);
    });
    m_outputsChanged = m_manager->outputsChanged.connect([this] {
        onOutputsChanged();
    });
}

OverviewController::~OverviewController()
{
    m_windowAdded.reset();
    m_windowRemoved.reset();
    m_outputsChanged.reset();
    m_manager->desks()->removeObserver(this);
    m_manager->activation()->removeObserver(this);
}

void OverviewController::setNodes(scene::OverviewNode *node, scene::OverviewDeskBarNode *bar)
{
    m_node = node;
    m_bar = bar;
    if (m_node) {
        m_node->setFinishedCallback([this](double target) {
            // From inside a frame, which is no place to change the scene.
            m_manager->runLater(std::chrono::milliseconds(0), [this, target] {
                onNodeFinished(target);
            });
        });
    }
}

bool OverviewController::isOverviewWindow(const Window *window) const
{
    if (!window || window->isShellWindow() || layerFor(window) != StackingLayer::Normal
        || !canIncludeWindowInMruList(window) || window->state()->isMinimized()
        || window->isHidden() || window->isDragPreview()
        || !animationOf(window) || window->bounds().isEmpty())
        return false;
    return m_manager->desks()->belongsToActiveDesk(window);
}

std::string OverviewController::deskName(int index) const
{
    const Desk *desk = m_manager->desks()->deskAt(index);
    if (desk && !desk->name().empty())
        return desk->name();
    // TRANSLATORS: the name of a desk that has none of its own, such as "Desk 2".
    return format(tr("Desk {number}"), {{"number", std::to_string(index + 1)}});
}

bool OverviewController::canCloseDesks() const
{
    DesksController *desks = m_manager->desks();
    return desks->desks().size() > 1 && desks->canRemoveDesks();
}

double OverviewController::shownProgress(double timeMs) const
{
    if (!m_node || !m_node->isActive())
        return 0;
    return std::clamp(m_node->progress(timeMs), 0.0, 1.0);
}

void OverviewController::buildBars()
{
    DesksController *desks = m_manager->desks();
    const int count = int(desks->desks().size());
    const bool canAdd = desks->canCreateDesks();
    m_bars.clear();
    m_barDesks.clear();
    for (const auto &output : m_manager->outputAreas()) {
        const base::Rect work = output.workArea.isEmpty() ? output.bounds : output.workArea;
        // The panels along the top edge give way to the overview; the
        // previews keep half the margin they had below them.
        const int top = output.bounds.top();
        const double margin = std::round((work.top() - top + kBarPadding) / 2.0);
        scene::OverviewDeskBarNode::Bar bar;
        bar.output = output.bounds;
        std::vector<Desk *> barDesks;
        double height = std::clamp(std::round(work.height() * 0.1), 56.0, 96.0);
        const double aspect = output.bounds.width() / double(std::max(1, output.bounds.height()));
        double width = std::round(height * aspect);
        double newDesk = canAdd ? std::round(width / 2) : 0;
        const auto total = [&] {
            return count * width + (count - 1) * kBarSpacing
                    + (canAdd ? kBarSpacing + newDesk : 0);
        };
        // Eight desks side by side still fit a narrow output.
        const double room = work.width() - 2 * kMargin;
        if (total() > room && total() > 0) {
            const double shrink = room / total();
            height = std::floor(height * shrink);
            width = std::floor(width * shrink);
            newDesk = canAdd ? std::floor(width / 2) : 0;
        }
        double x = std::round(centerPixel(work).x() + 0.5 - total() / 2);
        const double y = top + margin;
        for (int index = 0; index < count; ++index) {
            scene::OverviewDeskBarNode::Desk desk;
            desk.bounds = base::RectF(x, y, width, height);
            desk.name = deskName(index);
            desk.active = desks->deskAt(index) == desks->chosenDesk();
            desk.id = uintptr_t(desks->deskAt(index));
            bar.desks.push_back(std::move(desk));
            barDesks.push_back(desks->deskAt(index));
            x += width + kBarSpacing;
        }
        if (canAdd)
            bar.newDesk = base::RectF(x, y, newDesk, height);
        bar.bounds = base::RectF(work.left(), top, work.width(),
                                 margin + height + kLabelHeight + kBarPadding / 2);
        m_bars.push_back(std::move(bar));
        m_barDesks.push_back(std::move(barDesks));
    }
    if (m_deskDrag)
        arrangeDraggedDesk();
}

void OverviewController::arrangeDraggedDesk()
{
    const int barIndex = m_deskDrag->bar;
    if (barIndex < 0 || barIndex >= int(m_bars.size())) {
        m_deskDrag.reset();
        return;
    }
    scene::OverviewDeskBarNode::Bar &bar = m_bars[size_t(barIndex)];
    std::vector<Desk *> &barDesks = m_barDesks[size_t(barIndex)];
    const auto found = std::find(barDesks.begin(), barDesks.end(), m_deskDrag->desk);
    if (found == barDesks.end()) {
        m_deskDrag.reset();
        return;
    }
    const int from = int(found - barDesks.begin());
    std::vector<base::RectF> slots;
    for (const auto &desk : bar.desks)
        slots.push_back(desk.bounds);
    // Along the bar only, from the first place to the last.
    const double x = std::round(std::clamp(m_pointer.x() - m_deskDrag->grab, slots.front().x(),
                                           slots.back().x()));
    const double center = x + slots[size_t(from)].width() / 2;
    int target = 0;
    for (int index = 1; index < int(slots.size()); ++index) {
        if (std::abs(slots[size_t(index)].center().x() - center)
            < std::abs(slots[size_t(target)].center().x() - center))
            target = index;
    }
    std::vector<int> order;
    for (int index = 0; index < int(slots.size()); ++index) {
        if (index != from)
            order.push_back(index);
    }
    order.insert(order.begin() + target, from);

    std::vector<scene::OverviewDeskBarNode::Desk> desks;
    std::vector<Desk *> arranged;
    for (int place = 0; place < int(order.size()); ++place) {
        scene::OverviewDeskBarNode::Desk desk = bar.desks[size_t(order[size_t(place)])];
        desk.bounds = slots[size_t(place)];
        if (order[size_t(place)] == from) {
            desk.bounds.moveLeft(x);
            desk.dragged = true;
        }
        desks.push_back(std::move(desk));
        arranged.push_back(barDesks[size_t(order[size_t(place)])]);
    }
    bar.desks = std::move(desks);
    barDesks = std::move(arranged);
    m_deskDrag->target = target;
}

void OverviewController::rebuild(bool animate, bool keepPlaces)
{
    if (!m_node)
        return;
    std::map<Window *, base::RectF> kept;
    if (keepPlaces) {
        for (const Item &item : m_items)
            kept[item.window] = item.bounds;
    }
    // A press stays with the window or the desk it went down on, wherever
    // that is now; with it gone, the press is too.
    Window *pressedWindow = nullptr;
    Desk *pressedDesk = nullptr;
    if (m_press) {
        const Hit &hit = m_press->hit;
        if (hit.type == Hit::Type::Item && hit.index >= 0 && hit.index < int(m_items.size())) {
            pressedWindow = m_items[hit.index].window;
        } else if ((hit.type == Hit::Type::Desk || hit.type == Hit::Type::DeskClose)
                   && hit.bar >= 0 && hit.bar < int(m_barDesks.size()) && hit.index >= 0
                   && hit.index < int(m_barDesks[hit.bar].size())) {
            pressedDesk = m_barDesks[hit.bar][hit.index];
        }
    }

    const std::vector<Window *> &windows = m_manager->windows();
    std::vector<Window *> roots;
    for (Window *window : windows) {
        if (isOverviewWindow(window))
            roots.push_back(window);
    }

    buildBars();
    if (pressedDesk) {
        int index = -1;
        if (m_press->hit.bar < int(m_barDesks.size())) {
            const std::vector<Desk *> &desks = m_barDesks[m_press->hit.bar];
            const auto it = std::find(desks.begin(), desks.end(), pressedDesk);
            if (it != desks.end())
                index = int(it - desks.begin());
        }
        if (index < 0)
            m_press.reset();
        else
            m_press->hit.index = index;
    }
    const auto outputs = m_manager->outputAreas();
    std::map<Window *, base::RectF> places;
    for (size_t index = 0; index < outputs.size(); ++index) {
        const auto &output = outputs[index];
        std::vector<Window *> onOutput;
        std::vector<base::Rect> rects;
        for (Window *window : roots) {
            // Each window is laid out on the output that has its centre, or
            // the nearest one.
            const base::Point centre = centerPixel(window->bounds());
            const bool here = output.bounds.contains(centre)
                    || (std::none_of(outputs.begin(), outputs.end(),
                                     [centre](const auto &other) {
                                         return other.bounds.contains(centre);
                                     })
                        && m_manager->outputBoundsFor(window) == output.bounds);
            if (!here)
                continue;
            onOutput.push_back(window);
            rects.push_back(window->bounds());
        }
        const base::Rect work = output.workArea.isEmpty() ? output.bounds : output.workArea;
        const int top = index < m_bars.size() ? int(std::ceil(m_bars[index].bounds.bottom()))
                                              : work.top();
        const base::Rect area = base::Rect::fromEdges(work.left() + kMargin, top + kMargin / 2,
                                                      work.right() - kMargin, work.bottom() - kMargin);
        const auto spread = overview::spreadLayout(rects, area, kWindowGap, output.scale);
        for (size_t i = 0; i < onOutput.size() && i < spread.size(); ++i) {
            const auto it = kept.find(onOutput[i]);
            places[onOutput[i]] = it != kept.end() ? it->second : spread[i];
        }
    }

    Window *hovered = m_hovered >= 0 && m_hovered < int(m_items.size())
            ? m_items[m_hovered].window
            : nullptr;
    Window *selected = m_selected >= 0 && m_selected < int(m_items.size())
            ? m_items[m_selected].window
            : nullptr;
    Window *dragged = m_dragged >= 0 && m_dragged < int(m_items.size())
            ? m_items[m_dragged].window
            : nullptr;
    m_items.clear();
    m_hovered = m_selected = m_dragged = -1;
    int pressedIndex = -1;
    for (Window *window : roots) {
        const auto place = places.find(window);
        if (place == places.end())
            continue;
        if (window == pressedWindow)
            pressedIndex = int(m_items.size());
        if (window == hovered)
            m_hovered = int(m_items.size());
        if (window == selected)
            m_selected = int(m_items.size());
        if (window == dragged)
            m_dragged = int(m_items.size());
        m_items.push_back({window, place->second});
    }
    if (pressedWindow && pressedIndex < 0)
        m_press.reset();
    else if (pressedWindow)
        m_press->hit.index = pressedIndex;
    updateNodes(animate);
}

void OverviewController::setWallpaperNodes(std::vector<const scene::Node *> nodes)
{
    m_wallpapers = std::move(nodes);
}

void OverviewController::updateNodes(bool animate)
{
    const std::vector<Window *> &windows = m_manager->windows();
    std::vector<const scene::Node *> wallpapers;
    for (const scene::Node *wallpaper : m_wallpapers) {
        if (wallpaper->isVisible())
            wallpapers.push_back(wallpaper);
    }
    std::vector<const scene::Node *> desktops;
    for (Window *window : windows) {
        if (layerFor(window) == StackingLayer::Desktop && window->isVisible() && window->node())
            desktops.push_back(window->node());
    }
    std::vector<const scene::Node *> backdrop = wallpapers;
    backdrop.insert(backdrop.end(), desktops.begin(), desktops.end());

    std::vector<scene::OverviewNode::Item> items;
    for (const Item &item : m_items) {
        scene::OverviewNode::Item node;
        node.windows.push_back(animationOf(item.window));
        // Its dialogs and other transient windows come along, above it.
        for (Window *window : windows) {
            if (window != item.window && window->isVisible() && transientRoot(window) == item.window
                && animationOf(window))
                node.windows.push_back(animationOf(window));
        }
        node.to = item.bounds;
        node.title = item.window->title();
        items.push_back(std::move(node));
    }
    m_node->setBackdrop(wallpapers);
    m_node->setFadingBackdrop(desktops);
    m_node->setItems(std::move(items), animate);
    int raised = -1;
    for (size_t index = 0; index < m_items.size(); ++index) {
        if (m_swipeTarget && m_items[index].window == m_swipeTarget)
            raised = int(index);
    }
    m_node->setRaised(raised);
    if (m_dragged >= 0)
        m_node->setDragged(m_dragged, draggedBounds(m_pointer));

    if (m_bar) {
        DesksController *desks = m_manager->desks();
        std::vector<scene::OverviewDeskBarNode::DeskContent> contents(desks->desks().size());
        for (size_t index = 0; index < contents.size(); ++index)
            contents[index].id = uintptr_t(desks->deskAt(int(index)));
        for (Window *window : windows) {
            if (window->isShellWindow() || layerFor(window) != StackingLayer::Normal
                || window->state()->isMinimized() || window->isHidden() || !animationOf(window)
                || !window->targetVisibility())
                continue;
            const Desk *desk = desks->deskOf(transientRoot(window));
            for (size_t index = 0; index < contents.size(); ++index) {
                if (!desk || desks->deskAt(int(index)) == desk)
                    contents[index].windows.push_back(animationOf(window));
            }
        }
        m_bar->setContent(backdrop, std::move(contents));
    }
    updateChrome();
}

void OverviewController::updateChrome()
{
    if (!m_node)
        return;
    const int count = int(m_items.size());
    if (m_hovered >= count)
        m_hovered = -1;
    if (m_selected >= count)
        m_selected = -1;
    m_node->setChrome({m_hovered, m_selected});
    if (!m_bar)
        return;
    // A window dragged over the bar shows where it would drop.
    const Hit target = m_dragged >= 0 ? hitTest(m_pointer) : m_barHover;
    const bool closable = m_dragged < 0 && canCloseDesks();
    // A desk pressed dims while the pointer or touch is still on it.
    const Hit pressedOn =
            m_press && m_dragged < 0 && !m_deskDrag && m_press->hit.type == Hit::Type::Desk
            ? hitTest(m_press->touch ? m_touchLocation : m_pointer)
            : Hit();
    for (size_t index = 0; index < m_bars.size(); ++index) {
        auto &bar = m_bars[index];
        for (size_t desk = 0; desk < bar.desks.size(); ++desk) {
            const bool on = (target.type == Hit::Type::Desk || target.type == Hit::Type::DeskClose)
                    && target.bar == int(index) && target.index == int(desk);
            bar.desks[desk].highlighted = on && m_dragged >= 0;
            bar.desks[desk].pressed = pressedOn.type == Hit::Type::Desk
                    && pressedOn.bar == m_press->hit.bar && pressedOn.index == m_press->hit.index
                    && pressedOn.bar == int(index) && pressedOn.index == int(desk);
            bar.desks[desk].closeShown = on && closable;
            bar.desks[desk].closeHovered =
                    bar.desks[desk].closeShown && target.type == Hit::Type::DeskClose;
        }
        bar.newDeskHighlighted = target.type == Hit::Type::NewDesk && target.bar == int(index);
    }
    m_bar->setBars(m_bars);
}

bool OverviewController::capturesInput() const
{
    return m_state == State::Entering || m_state == State::Shown;
}

void OverviewController::takePointer()
{
    if (std::exchange(m_pointerTaken, true))
        return;
    if (input::Seat *seat = m_manager->seat()) {
        seat->clearPointerFocus();
        seat->setCompositorCursorShape(input::CursorShape::Default);
    }
}

void OverviewController::releasePointer()
{
    m_pointerTaken = false;
}

void OverviewController::completeTransition()
{
    if (!m_node || m_node->isTracking())
        return;
    if (m_state == State::Entering) {
        m_node->jumpTo(1);
        m_state = State::Shown;
        setHover(hitTest(m_pointer));
    } else if (m_state == State::Exiting) {
        finish();
    }
}

bool OverviewController::acceptsPresses() const
{
    return m_node
            && (m_state == State::Shown || (m_state == State::Entering && !m_node->isTracking()));
}

void OverviewController::landDeskSwitch()
{
    // A desk switch still sliding, whose frames would cover the overview,
    // lands at once: the overview starts from the desk it was heading to.
    if (DeskAnimationBase *animation = m_manager->desks()->animation())
        animation->finishNow();
    if (m_node)
        m_node->setContentHidden(false);
}

void OverviewController::onOutputsChanged()
{
    if (!isActive() || !m_node)
        return;
    // A drag or press is aimed at the old layout; the bars and previews have
    // the old outputs' shapes.
    m_press.reset();
    m_dragged = -1;
    m_deskDrag.reset();
    m_node->setGeometry(m_manager->outputBounds());
    if (m_bar)
        m_bar->setGeometry(m_manager->outputBounds());
    rebuild(false);
    if (m_bar)
        m_bar->invalidatePreviews();
    // What lies under the pointer is found again by a synthesized motion.
    m_manager->pointerTargetsChanged();
}

void OverviewController::activateForExit(Window *window)
{
    if (!window || !m_manager->hasWindow(window))
        return;
    m_activating = true;
    if (window->isHidden())
        m_manager->unhideApplication(window);
    if (m_manager->hasWindow(window))
        m_manager->activation()->activate(window);
    m_activating = false;
    // Raised to the top it draws over the rest; its place and the others' stay.
    rebuild(true, true);
}

void OverviewController::begin()
{
    // The overview lays windows out from their bounds; none stays aside.
    if (ShowDesktopController *showDesktop = m_manager->showDesktop())
        showDesktop->exitImmediately();
    m_hovered = m_selected = m_dragged = -1;
    m_barHover = {};
    m_press.reset();
    m_deskDrag.reset();
    m_node->setGeometry(m_manager->outputBounds());
    if (m_bar)
        m_bar->setGeometry(m_manager->outputBounds());
    rebuild(false);
    m_node->jumpTo(0);
    if (m_bar)
        m_bar->invalidatePreviews();
    // What the pointer was over gets a leave, as the overview takes it.
    takePointer();
    if (input::Seat *seat = m_manager->seat()) {
        seat->setCursorOverride(input::CursorShape::Default);
        m_cursorOverridden = true;
    }
}

void OverviewController::finish()
{
    m_state = State::Hidden;
    m_items.clear();
    m_bars.clear();
    m_barDesks.clear();
    m_hovered = m_selected = m_dragged = -1;
    m_swipeTarget = nullptr;
    m_press.reset();
    m_deskDrag.reset();
    m_buttons.clear();
    m_keys.clear();
    m_touches.clear();
    if (std::exchange(m_cursorOverridden, false)) {
        if (input::Seat *seat = m_manager->seat())
            seat->setCursorOverride(std::nullopt);
    }
    if (m_node) {
        m_node->jumpTo(0);
        m_node->setContentHidden(false);
        m_node->setItems({}, false);
        m_node->setBackdrop({});
        m_node->setFadingBackdrop({});
    }
    if (m_bar) {
        m_bar->setBars({});
        m_bar->setContent({}, {});
    }
    releasePointer();
    // The windows are where they were: whatever is under the pointer enters.
    m_manager->pointerTargetsChanged();
}

void OverviewController::onNodeFinished(double target)
{
    if (!m_node || !m_node->isSettled() || m_node->target() != target)
        return;
    if (target >= 1 && m_state == State::Entering) {
        m_state = State::Shown;
        setHover(hitTest(m_pointer));
    } else if (target <= 0 && m_state == State::Exiting) {
        finish();
    }
}

void OverviewController::toggle()
{
    if (m_state == State::Entering || m_state == State::Shown)
        exit();
    else
        enter();
}

bool OverviewController::enter()
{
    if (!m_node || !m_manager->allowsActivation())
        return false;
    if (m_state == State::Entering || m_state == State::Shown)
        return true;
    // The overview would take the pointer from under the drag, which would
    // then go on without a button held once it leaves.
    if (m_manager->eventHandler()->draggedWindow())
        return false;
    m_manager->dismissPopupGrabs();
    landDeskSwitch();
    if (m_state == State::Hidden)
        begin();
    takePointer();
    m_state = State::Entering;
    m_node->animateTo(1);
    return true;
}

void OverviewController::exit(Window *window)
{
    if (m_state == State::Hidden || (m_state == State::Exiting && !window))
        return;
    landDeskSwitch();
    leave(window);
}

void OverviewController::leave(Window *window)
{
    cancelPress();
    m_swipeTarget = nullptr;
    activateForExit(window);
    m_hovered = m_selected = -1;
    m_barHover = {};
    updateChrome();
    m_state = State::Exiting;
    m_node->animateTo(0);
    releasePointer();
}

void OverviewController::exitImmediately()
{
    if (m_state == State::Hidden)
        return;
    finish();
}

bool OverviewController::startSwipe(bool upward)
{
    if (!m_node || !m_manager->allowsActivation())
        return false;
    if (m_state == State::Hidden && !upward)
        return false;
    m_manager->dismissPopupGrabs();
    landDeskSwitch();
    if (m_state == State::Hidden)
        begin();
    cancelPress();
    // Fingers down over a window take the overview away to it; it rides on
    // top of the rest meanwhile.
    const int target = m_hovered >= 0 ? m_hovered : m_selected;
    m_swipeTarget = !upward && m_state == State::Shown && target >= 0
            ? m_items[target].window
            : nullptr;
    m_node->setRaised(m_swipeTarget ? target : -1);
    m_hovered = m_selected = -1;
    updateChrome();
    takePointer();
    m_state = State::Entering;
    m_node->startTracking();
    return true;
}

void OverviewController::updateSwipe(double delta, double timeMs)
{
    if (m_node && m_node->isTracking())
        m_node->updateTracking(delta, timeMs);
}

void OverviewController::endSwipe(double timeMs)
{
    if (!m_node || !m_node->isTracking())
        return;
    const double target = m_node->endTracking(timeMs);
    Window *window = std::exchange(m_swipeTarget, nullptr);
    if (target >= 1) {
        m_state = State::Entering;
        m_node->setRaised(-1);
    } else {
        m_state = State::Exiting;
        activateForExit(window);
        releasePointer();
    }
}

OverviewController::Hit OverviewController::hitTest(const base::PointF &location) const
{
    Hit hit;
    if (!isActive())
        return hit;
    // The panels, the dock and whatever else is above the desk stay usable,
    // server-side frames too, which windowAt() does not see.
    const auto aboveDesk = [](const Window *window) {
        return window && layerFor(window) > StackingLayer::Normal;
    };
    if (aboveDesk(m_manager->windowAt(location)) || aboveDesk(m_manager->frameAt(location))) {
        hit.type = Hit::Type::Panel;
        return hit;
    }
    // On the way in, what is under the pointer is where it is drawn now.
    const bool moving = m_state != State::Shown && m_node;
    const double progress = moving ? std::clamp(m_node->progress(), 0.0, 1.0) : 1;
    for (size_t bar = 0; bar < m_bars.size(); ++bar) {
        const auto &candidate = m_bars[bar];
        const base::PointF onBar =
                location - base::PointF(0, scene::OverviewDeskBarNode::slideOffset(candidate, progress));
        if (!containsWithEdges(candidate.bounds, onBar))
            continue;
        hit.bar = int(bar);
        for (size_t desk = 0; desk < candidate.desks.size(); ++desk) {
            const base::RectF &bounds = candidate.desks[desk].bounds;
            if (containsWithEdges(bounds, onBar)) {
                const bool close = m_dragged < 0 && canCloseDesks()
                        && containsWithEdges(scene::OverviewDeskBarNode::closeButtonRect(bounds), onBar);
                hit.type = close ? Hit::Type::DeskClose : Hit::Type::Desk;
                hit.index = int(desk);
                return hit;
            }
        }
        hit.type = containsWithEdges(candidate.newDesk, onBar) ? Hit::Type::NewDesk : Hit::Type::Bar;
        return hit;
    }
    for (int index = int(m_items.size()) - 1; index >= 0; --index) {
        const Item &item = m_items[index];
        if (index == m_dragged)
            continue;
        const base::RectF bounds = moving
                ? m_node->presentedRect(index, progress, m_node->clockTime())
                : item.bounds;
        if (containsWithEdges(bounds, location)) {
            hit.type = Hit::Type::Item;
            hit.index = index;
            return hit;
        }
    }
    return hit;
}

void OverviewController::setHover(const Hit &hit)
{
    m_hovered = hit.type == Hit::Type::Item && m_state == State::Shown ? hit.index : -1;
    const bool onBar = hit.type == Hit::Type::Desk || hit.type == Hit::Type::DeskClose
            || hit.type == Hit::Type::NewDesk;
    m_barHover = onBar ? hit : Hit();
    // The pointer takes over from the keyboard.
    if (m_hovered >= 0)
        m_selected = -1;
    updateChrome();
}

void OverviewController::press(const Hit &hit, const base::PointF &location,
                               std::optional<int32_t> touch)
{
    m_press = Press {hit, location, touch};
    updateChrome();
}

void OverviewController::move(const base::PointF &location)
{
    m_pointer = location;
    if (m_press && m_dragged < 0 && m_press->hit.type == Hit::Type::Item
        && m_state == State::Shown && distance(location, m_press->location) >= kDragThreshold) {
        m_dragged = m_press->hit.index;
        // Pressed while the window was still on its way, the grab point may
        // lie off it now.
        const base::RectF &bounds = m_items[m_dragged].bounds;
        const base::PointF grab = m_press->location - bounds.topLeft();
        m_dragOffset = base::PointF(std::clamp(grab.x(), 0.0, bounds.width()),
                               std::clamp(grab.y(), 0.0, bounds.height()));
        m_hovered = -1;
    }
    if (m_press && m_dragged < 0 && !m_deskDrag && m_press->hit.type == Hit::Type::Desk
        && m_state == State::Shown && m_manager->desks()->desks().size() > 1
        && distance(location, m_press->location) >= kDragThreshold) {
        const Hit &hit = m_press->hit;
        if (hit.bar >= 0 && hit.bar < int(m_bars.size()) && hit.index >= 0
            && hit.index < int(m_bars[size_t(hit.bar)].desks.size())) {
            const base::RectF &bounds = m_bars[size_t(hit.bar)].desks[size_t(hit.index)].bounds;
            m_deskDrag = DeskDrag {hit.bar, m_barDesks[size_t(hit.bar)][size_t(hit.index)],
                                   m_press->location.x() - bounds.x(), hit.index};
            m_barHover = {};
        }
    }
    if (m_deskDrag) {
        buildBars();
        updateChrome();
        return;
    }
    if (m_dragged < 0) {
        setHover(hitTest(location));
        return;
    }
    m_node->setDragged(m_dragged, draggedBounds(location));
    updateChrome();
}

base::RectF OverviewController::draggedBounds(const base::PointF &location) const
{
    const Item &item = m_items[m_dragged];
    const base::SizeF full = item.bounds.size();
    base::SizeF size = full;
    for (size_t index = 0; index < m_bars.size(); ++index) {
        const auto &bar = m_bars[index];
        if (!bar.output.contains(location.toPoint()) || full.isEmpty())
            continue;
        // It shrinks from where it sits in the overview up to the bar.
        const double barHeight = bar.bounds.height();
        const double start = item.bounds.top() + m_dragOffset.y();
        const double reach = start - bar.bounds.bottom();
        const double distance = std::max(0.0, location.y() - bar.bounds.bottom());
        if (distance >= reach && reach > 0)
            break;
        // On the bar, the window at the bar's share of the output's height.
        const base::SizeF window = item.window->bounds().size();
        base::SizeF onBar = window * (barHeight / std::max(1, bar.output.height()));
        const double minimum = barHeight * kMinimumSideOnBar;
        const double shorter = std::min(onBar.width(), onBar.height());
        if (shorter > 0 && shorter < minimum)
            onBar = onBar * (minimum / shorter);
        if (onBar.width() > full.width())
            onBar = full;
        const double t = reach > 0 ? 1 - distance / reach : 1;
        size = full + (onBar - full) * t;
        break;
    }
    // Held at the same share of its size as where it was grabbed.
    const base::PointF grab(m_dragOffset.x() / full.width(), m_dragOffset.y() / full.height());
    return base::RectF(location - base::PointF(grab.x() * size.width(), grab.y() * size.height()), size);
}

void OverviewController::release(const base::PointF &location)
{
    if (!m_press)
        return;
    const Press press = *m_press;
    m_press.reset();
    updateChrome();
    if (m_deskDrag) {
        dropDesk();
        return;
    }
    if (m_dragged >= 0) {
        drop(location);
        return;
    }
    // A button acts when let go of over itself; moved off, the press is dropped.
    const Hit over = hitTest(location);
    if (press.hit.type != Hit::Type::Item
        && (over.type != press.hit.type || over.index != press.hit.index
            || over.bar != press.hit.bar))
        return;
    DesksController *desks = m_manager->desks();
    switch (press.hit.type) {
    case Hit::Type::Item:
        activateItem(press.hit.index);
        break;
    case Hit::Type::Desk:
        if (press.hit.bar < int(m_barDesks.size())
            && press.hit.index < int(m_barDesks[press.hit.bar].size())) {
            // A desk takes the overview away to it, as Escape does. Another
            // one slides in on the strip, which starts from the overview, while
            // the desk bar goes and the panels come back.
            Desk *desk = m_barDesks[press.hit.bar][press.hit.index];
            if (!desk || desk == desks->activeDesk()) {
                exit();
                break;
            }
            desks->activateDesk(desk, DesksSwitchSource::MiniViewButton);
            if (!desks->animation()) {
                exit();
                break;
            }
            m_node->setContentHidden(true);
            leave(nullptr);
        }
        break;
    case Hit::Type::DeskClose:
        // Its windows go to the desk beside it.
        if (canCloseDesks() && press.hit.bar < int(m_barDesks.size())
            && press.hit.index < int(m_barDesks[press.hit.bar].size())) {
            if (Desk *desk = m_barDesks[press.hit.bar][press.hit.index])
                desks->removeDesk(desk, DesksCreationRemovalSource::Button);
        }
        setHover(hitTest(location));
        break;
    case Hit::Type::NewDesk:
        if (desks->canCreateDesks())
            desks->newDesk(DesksCreationRemovalSource::Button);
        break;
    case Hit::Type::Panel:
        break;
    // The bar's empty space is the overview's background, as in Ash, whose
    // desk bar lets a press through.
    case Hit::Type::Bar:
    case Hit::Type::None:
        exit();
        break;
    }
}

void OverviewController::cancelPress()
{
    m_press.reset();
    if (m_deskDrag) {
        m_deskDrag.reset();
        buildBars();
    }
    if (m_dragged < 0) {
        updateChrome();
        return;
    }
    m_dragged = -1;
    if (m_node)
        rebuild(true);
}

void OverviewController::dropDesk()
{
    const DeskDrag drag = *m_deskDrag;
    m_deskDrag.reset();
    DesksController *desks = m_manager->desks();
    const int from = desks->deskIndex(drag.desk);
    if (from >= 0 && drag.target >= 0)
        desks->reorderDesk(from, drag.target);
    // Into its place, or back to it when the desks could not be reordered.
    buildBars();
    updateChrome();
}

void OverviewController::drop(const base::PointF &location)
{
    Window *window = m_items[m_dragged].window;
    // The node keeps drawing it where it was let go, which is where it
    // heads home from.
    m_dragged = -1;
    const Hit target = hitTest(location);
    DesksController *desks = m_manager->desks();
    Desk *desk = nullptr;
    if ((target.type == Hit::Type::Desk || target.type == Hit::Type::DeskClose)
        && target.bar < int(m_barDesks.size())
        && target.index < int(m_barDesks[target.bar].size()))
        desk = m_barDesks[target.bar][target.index];
    else if (target.type == Hit::Type::NewDesk && desks->canCreateDesks())
        desk = desks->newDesk(DesksCreationRemovalSource::Button);
    // Back to its place, unless it goes to another desk, which lays the rest
    // out without it.
    if (!desk || desk == desks->activeDesk()
        || !desks->moveWindowFromActiveDeskTo(window, desk)) {
        rebuild(true);
    }
    setHover(hitTest(location));
}

void OverviewController::activateItem(int index)
{
    if (index < 0 || index >= int(m_items.size()))
        return;
    exit(m_items[index].window);
}

void OverviewController::moveSelection(int dx, int dy)
{
    if (m_items.empty())
        return;
    const int from = m_selected >= 0 ? m_selected : m_hovered;
    if (from < 0) {
        // The window used last comes first.
        Window *active = m_manager->activation()->activeWindow();
        m_selected = int(m_items.size()) - 1;
        for (size_t index = 0; index < m_items.size(); ++index) {
            if (m_items[index].window == active)
                m_selected = int(index);
        }
        m_hovered = -1;
        updateChrome();
        return;
    }
    const base::PointF centre = m_items[from].bounds.center();
    int best = -1;
    double bestScore = 0;
    for (int index = 0; index < int(m_items.size()); ++index) {
        if (index == from)
            continue;
        const base::PointF d = m_items[index].bounds.center() - centre;
        const double along = d.x() * dx + d.y() * dy;
        if (along <= 1)
            continue;
        const double across = std::abs(d.x() * dy - d.y() * dx);
        const double score = along + 2 * across;
        if (best < 0 || score < bestScore) {
            best = index;
            bestScore = score;
        }
    }
    if (best >= 0) {
        m_selected = best;
        m_hovered = -1;
        updateChrome();
    }
}

bool OverviewController::onPointerMotion(input::Timestamp timestamp, const base::PointF &location)
{
    m_pointer = location;
    if (!capturesInput())
        return m_next && m_next->onPointerMotion(timestamp, location);
    const Hit hit = hitTest(location);
    if (hit.type == Hit::Type::Panel && !m_press) {
        if (m_pointerTaken) {
            releasePointer();
            setHover({});
        }
        return m_next && m_next->onPointerMotion(timestamp, location);
    }
    // The dock's auto-hide watches every motion.
    m_manager->pointerMoved(location);
    takePointer();
    if (!m_press || !m_press->touch)
        move(location);
    return true;
}

bool OverviewController::onPointerButton(input::Timestamp timestamp, const base::PointF &location,
                                         uint32_t evdevButton, bool pressed)
{
    if (!pressed) {
        if (!m_buttons.erase(evdevButton))
            return m_next && m_next->onPointerButton(timestamp, location, evdevButton, pressed);
        if (evdevButton == BTN_LEFT && m_press && !m_press->touch)
            release(location);
        return true;
    }
    if (!capturesInput())
        return m_next && m_next->onPointerButton(timestamp, location, evdevButton, pressed);
    const Hit hit = hitTest(location);
    if (hit.type == Hit::Type::Panel && !m_press)
        return m_next && m_next->onPointerButton(timestamp, location, evdevButton, pressed);
    m_buttons.insert(evdevButton);
    takePointer();
    if (evdevButton == BTN_LEFT && !m_press && acceptsPresses())
        press(hit, location, std::nullopt);
    return true;
}

bool OverviewController::onPointerAxis(input::Timestamp timestamp, const base::PointF &location,
                                       const base::PointF &offset, bool discrete)
{
    if (!capturesInput() || hitTest(location).type == Hit::Type::Panel)
        return m_next && m_next->onPointerAxis(timestamp, location, offset, discrete);
    return true;
}

bool OverviewController::onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location)
{
    if (!capturesInput() || hitTest(location).type == Hit::Type::Panel)
        return m_next && m_next->onPointerAxisStop(timestamp, location);
    return true;
}

bool OverviewController::onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    // A key held since before the overview, such as the shortcut's modifier,
    // goes up where it went down.
    if (!pressed) {
        if (!m_keys.erase(evdevCode))
            return m_next && m_next->onKey(timestamp, evdevCode, pressed);
        return true;
    }
    if (!capturesInput())
        return m_next && m_next->onKey(timestamp, evdevCode, pressed);
    m_keys.insert(evdevCode);
    switch (evdevCode) {
    case KEY_ESC:
        exit();
        break;
    case KEY_ENTER:
    case KEY_KPENTER:
    case KEY_SPACE:
        if (acceptsPresses())
            activateItem(m_selected >= 0 ? m_selected : m_hovered);
        break;
    case KEY_LEFT:
        moveSelection(-1, 0);
        break;
    case KEY_RIGHT:
        moveSelection(1, 0);
        break;
    case KEY_UP:
        moveSelection(0, -1);
        break;
    case KEY_DOWN:
        moveSelection(0, 1);
        break;
    default:
        break;
    }
    return true;
}

bool OverviewController::onSecureKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    return m_next && m_next->onSecureKey(timestamp, evdevCode, pressed);
}

bool OverviewController::onTouchDown(input::Timestamp timestamp, int32_t id,
                                     const base::PointF &location)
{
    if (!capturesInput())
        return m_next && m_next->onTouchDown(timestamp, id, location);
    const Hit hit = hitTest(location);
    if (hit.type == Hit::Type::Panel)
        return m_next && m_next->onTouchDown(timestamp, id, location);
    m_touches.insert(id);
    if (!m_press && acceptsPresses()) {
        m_touchLocation = location;
        press(hit, location, id);
    }
    return true;
}

bool OverviewController::onTouchMotion(input::Timestamp timestamp, int32_t id,
                                       const base::PointF &location)
{
    if (!m_touches.count(id))
        return m_next && m_next->onTouchMotion(timestamp, id, location);
    if (m_press && m_press->touch == id) {
        m_touchLocation = location;
        move(location);
    }
    return true;
}

bool OverviewController::onTouchUp(input::Timestamp timestamp, int32_t id)
{
    if (!m_touches.erase(id))
        return m_next && m_next->onTouchUp(timestamp, id);
    if (m_press && m_press->touch == id)
        release(m_touchLocation);
    return true;
}

void OverviewController::onTouchCancel(input::Timestamp timestamp)
{
    m_touches.clear();
    if (m_press && m_press->touch)
        cancelPress();
    if (m_next)
        m_next->onTouchCancel(timestamp);
}

void OverviewController::onWindowActivated(Window *gainedActive, Window *lostActive)
{
    if (m_state == State::Hidden || m_activating || !gainedActive)
        return;
    // Activation moving on from a window that closed, or to a window of the
    // desk switched to, leaves the overview up; anything else the user
    // brought up - a dock task, the launcher, a dialog - ends it.
    if (lostActive && !m_manager->hasWindow(lostActive))
        return;
    if (m_manager->desks()->areDesksBeingModified())
        return;
    exit();
}

void OverviewController::onDeskAdded(const Desk *)
{
    if (isActive())
        rebuild(true);
}

void OverviewController::onDeskRemoved(const Desk *)
{
    if (isActive())
        rebuild(true);
}

void OverviewController::onDeskReordered(int, int)
{
    if (isActive())
        rebuild(true);
}

void OverviewController::onDeskActivationChanged(const Desk *, const Desk *)
{
    // The desk switch slides frames of the overview; the ending one has to
    // show the new desk's windows already in place.
    if (isActive())
        rebuild(false);
}

void OverviewController::onDeskNameChanged(const Desk *)
{
    if (!isActive())
        return;
    buildBars();
    updateChrome();
}

void OverviewController::retranslate()
{
    onDeskNameChanged(nullptr);
}

void OverviewController::onChosenDeskChanged()
{
    if (!isActive())
        return;
    buildBars();
    updateChrome();
}

void OverviewController::onWindowDeskChanged(Window *)
{
    if (isActive())
        rebuild(true);
}

} // namespace Lingmo::WM
