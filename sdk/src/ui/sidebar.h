// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

class Label;
class ThemeIcon;

// A selectable destination in a navigation sidebar.
class SidebarItem : public View {
public:
  SidebarItem(std::string id, std::string title, std::string icon);

  const std::string &id() const { return m_id; }
  bool isCurrent() const { return m_current; }
  void setCurrent(bool current);

  SizeF sizeHint() const override { return {0, 40}; }
  bool acceptsPointer() const override { return true; }
  bool acceptsFocus() const override { return true; }

  std::function<void()> onActivated;
  std::function<void(PointF)> onContextMenu;
  std::function<void(int)> onMoveRequested;

protected:
  void layout() override;
  void paint(Painter &painter) override;
  void hoverChanged() override;
  void pointerPress(const PointerEvent &event) override;
  void pointerRelease(const PointerEvent &event) override;
  bool keyPress(const KeyEvent &event) override;

private:
  std::string m_id;
  ThemeIcon *m_icon;
  Label *m_title;
  bool m_current = false;
  bool m_pressed = false;
};

// A simple navigation list. Applications own page routing and provide stable
// IDs; the SDK owns row layout, selection, input and visual states.
class SidebarList : public View {
public:
  struct Entry {
    std::string id;
    std::string title;
    std::string icon;
  };

  explicit SidebarList(std::vector<Entry> entries = {});
  void setEntries(std::vector<Entry> entries);
  void setCurrent(const std::string &id);
  const std::string &current() const { return m_current; }
  int count() const { return int(m_items.size()); }
  RectF itemRect(int index) const;

  SizeF sizeHint() const override;
  double heightForWidth(double width) const override;
  std::function<void(const std::string &)> onActivated;

protected:
  void layout() override;

private:
  void moveFocus(int index, int delta);

  std::vector<SidebarItem *> m_items;
  std::string m_current;
};

} // namespace Lingmo::ui
