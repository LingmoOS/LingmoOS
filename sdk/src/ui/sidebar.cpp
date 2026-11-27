// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/sidebar.h"

#include <algorithm>
#include <utility>

#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace Lingmo::ui {

static constexpr double kSidebarInset = 8;
static constexpr double kSidebarRowHeight = 40;
static constexpr double kSidebarRowSpacing = 2;
static constexpr double kIconTile = 24;

SidebarItem::SidebarItem(std::string id, std::string title, std::string icon)
    : m_id(std::move(id)), m_icon(addChild<ThemeIcon>(std::move(icon), 16)),
      m_title(addChild<Label>(std::move(title))) {
  m_icon->setColor(ColorId::HighlightedText);
  m_title->setFontSize(Theme::current().fontSize());
  m_title->setVerticallyCentered(true);
  setName("sidebar:" + m_id);
  setTracksHover(true);
  setCursorShape(CursorShape::Pointer);
  setToolTip(m_title->text());
}

void SidebarItem::setCurrent(bool current) {
  if (current == m_current)
    return;
  m_current = current;
  m_title->setColor(m_current ? ColorId::HighlightedText : ColorId::Text);
  if (!m_iconBackgroundVisible)
    m_icon->setColor(m_current ? ColorId::HighlightedText : ColorId::Text);
  requestPaint();
}

void SidebarItem::setIconBackgroundVisible(bool visible) {
  if (visible == m_iconBackgroundVisible)
    return;
  m_iconBackgroundVisible = visible;
  m_icon->setColor(visible || m_current ? ColorId::HighlightedText : ColorId::Text);
  requestPaint();
}

void SidebarItem::layout() {
  const double iconX = (kSidebarRowHeight - kIconTile) / 2;
  m_icon->setGeometry(
      {iconX + (kIconTile - 16) / 2, (geometry().height - 16) / 2, 16, 16});
  m_title->setColor(m_current ? ColorId::HighlightedText : ColorId::Text);
  m_title->setGeometry({kSidebarRowHeight, 0,
                        std::max(0.0, geometry().width - kSidebarRowHeight - 8),
                        geometry().height});
}

void SidebarItem::paint(Painter &painter) {
  const Theme &theme = Theme::current();
  if (m_current || isHovered() || m_pressed) {
    const Color fill =
        m_pressed   ? color(ColorId::ItemPressed)
        : m_current ? theme.highlight
                    : theme.text.withAlpha(theme.darkMode ? 0.065 : 0.05);
    if (theme.controlStyle == 1 && (m_current || m_pressed)) {
        // Liquid Glass: lift the selected/active row with a shadow and rim.
        painter.drawShadow(localRect(), theme.mediumRadius, 5,
                           theme.darkMode ? Color{0, 0, 0, 55} : Color{0, 0, 0, 25},
                           {0, 1.0});
    }
    painter.drawSmoothRectangle(localRect(), theme.mediumRadius, fill);
    if (theme.controlStyle == 1 && (m_current || m_pressed)) {
        const double rimA = theme.darkMode ? 0.22 : 0.40;
        painter.strokeRoundedRect(localRect().shrunk({0.5, 0.5, 0.5, 0.5}),
                                  theme.mediumRadius - 0.5,
                                  Color{255, 255, 255, uint8_t(rimA * 255)}, 1);
    }
  }
  if (m_iconBackgroundVisible) {
    painter.fillRoundedRect({(kSidebarRowHeight - kIconTile) / 2,
                             (geometry().height - kIconTile) / 2, kIconTile,
                             kIconTile},
                            6, theme.highlight);
  }
  if (hasVisibleFocus())
    painter.strokeRoundedRect(localRect().shrunk({1, 1, 1, 1}),
                              theme.mediumRadius, theme.highlight, 2);
}

void SidebarItem::hoverChanged() { requestPaint(); }

void SidebarItem::pointerPress(const PointerEvent &event) {
  if (event.button != MouseButton::Left)
    return;
  m_pressed = true;
  requestPaint();
}

void SidebarItem::pointerRelease(const PointerEvent &event) {
  if (event.button == MouseButton::Right && localRect().contains(event.position)) {
    if (onContextMenu)
      onContextMenu(event.position);
    return;
  }
  if (!m_pressed)
    return;
  m_pressed = false;
  requestPaint();
  if (event.button == MouseButton::Left &&
      localRect().contains(event.position) && onActivated)
    onActivated();
}

bool SidebarItem::keyPress(const KeyEvent &event) {
  if (event.key == key::Return || event.key == key::KeypadEnter ||
      event.key == key::Space) {
    if (onActivated)
      onActivated();
    return true;
  }
  if ((event.key == key::Up || event.key == key::Down) && onMoveRequested) {
    onMoveRequested(event.key == key::Up ? -1 : 1);
    return true;
  }
  return false;
}

SidebarList::SidebarList(std::vector<Entry> entries) {
  setEntries(std::move(entries));
}

void SidebarList::setEntries(std::vector<Entry> entries) {
  while (!children().empty())
    takeChild(children().front().get());
  m_items.clear();
  for (auto &entry : entries) {
    auto *item = addChild<SidebarItem>(
        std::move(entry.id), std::move(entry.title), std::move(entry.icon));
    item->onActivated = [this, id = item->id()] {
      setCurrent(id);
      if (onActivated)
        onActivated(id);
    };
    item->onMoveRequested = [this, index = int(m_items.size())](int delta) {
      moveFocus(index, delta);
    };
    m_items.push_back(item);
  }
  setCurrent(m_current);
  requestLayout();
}

void SidebarList::setCurrent(const std::string &id) {
  m_current = id;
  for (auto *item : m_items)
    item->setCurrent(item->id() == id);
}

RectF SidebarList::itemRect(int index) const {
  return index >= 0 && index < int(m_items.size())
             ? m_items[size_t(index)]->geometry()
             : RectF{};
}

SizeF SidebarList::sizeHint() const {
  return {0, heightForWidth(geometry().width)};
}

double SidebarList::heightForWidth(double) const {
  if (m_items.empty())
    return 0;
  return 2 * kSidebarInset + m_items.size() * kSidebarRowHeight +
         (m_items.size() - 1) * kSidebarRowSpacing;
}

void SidebarList::layout() {
  double y = kSidebarInset;
  for (auto *item : m_items) {
    item->setGeometry({kSidebarInset, y,
                       std::max(0.0, geometry().width - 2 * kSidebarInset),
                       kSidebarRowHeight});
    y += kSidebarRowHeight + kSidebarRowSpacing;
  }
}

void SidebarList::moveFocus(int index, int delta) {
  if (m_items.empty())
    return;
  const int next = std::clamp(index + delta, 0, int(m_items.size()) - 1);
  m_items[size_t(next)]->setFocus(FocusReason::Keyboard);
}

} // namespace Lingmo::ui
