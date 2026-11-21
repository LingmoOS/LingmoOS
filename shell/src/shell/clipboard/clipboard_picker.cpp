// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/clipboard/clipboard_picker.h"

#include <cmath>

#include "shell/clipboard/clipboard_history.h"
#include "shell/clipboard/clipboard_picker_view.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/theme.h"
#include "wm/window_manager.h"

namespace Lingmo::Shell {

ClipboardPicker::ClipboardPicker(Shell *shell, std::unique_ptr<ClipboardHistory> history)
    : m_shell(shell)
    , m_history(std::move(history))
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Picker);
    auto view = std::make_unique<ClipboardPickerView>(*m_history);
    m_view = view.get();
    m_view->onCloseRequested = [this] { close(); };
    m_view->onPainted = [this] { applyGlass(); };
    m_window->setContentView(std::move(view));
    m_window->setSize(base::Size(ClipboardPickerView::kWidth, ClipboardPickerView::kHeight));
    m_window->onKeyboardFocusChanged = [this](bool focused) {
        if (focused) {
            m_hadKeyboard = true;
        } else if (m_hadKeyboard && isOpen()) {
            m_hadKeyboard = false;
            close();
        }
    };
    m_window->onCloseRequested = [this] { close(); };
    applyGlass();

    m_history->togglePicker = [this] { toggle(); };
    m_history->closePicker = [this] { close(); };
    m_history->sendPaste = [this] { m_shell->windowManager()->sendPaste(); };

    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] {
        if (isOpen())
            place();
    }));
}

ClipboardPicker::~ClipboardPicker()
{
    m_subscriptions.clear();
    m_history->togglePicker = nullptr;
    m_history->closePicker = nullptr;
    m_history->sendPaste = nullptr;
    m_window.reset();
    m_history.reset();
}

bool ClipboardPicker::isOpen() const
{
    return m_window->isShown();
}

base::Rect ClipboardPicker::geometry() const
{
    const int width = ClipboardPickerView::kWidth;
    const int height = ClipboardPickerView::kHeight;
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return base::Rect(0, 0, width, height);
    const base::Rect area = m_shell->workArea(*output);
    return base::Rect(int(std::lround(area.x() + (area.width() - width) / 2.0)),
                      int(std::lround(area.y() + (area.height() - height) / 2.0)), width, height);
}

void ClipboardPicker::place()
{
    m_window->setGeometry(geometry());
}

void ClipboardPicker::applyGlass()
{
    const bool dark = ui::Theme::current().darkMode;
    if (int(dark) == m_glassDark)
        return;
    m_glassDark = dark;
    // FishUI.PopupGlass with PanelStyle.glassTint, the OSD's.
    const render::GlassParams glass = dark ? popupGlass(0.10, 0.10, 0.11, 0.75)
                                           : popupGlass(0.90, 0.90, 0.92, 0.72);
    m_window->setBackgroundBlur(base::Rect(0, 0, ClipboardPickerView::kWidth, ClipboardPickerView::kHeight),
                                int(ClipboardPickerView::kCardRadius), glass);
}

void ClipboardPicker::open()
{
    if (isOpen())
        return;
    m_hadKeyboard = false;
    place();
    applyGlass();
    m_view->reset();
    // Mapping a picker takes the keyboard grab.
    m_window->show();
}

void ClipboardPicker::close()
{
    m_hadKeyboard = false;
    m_window->hide();
}

void ClipboardPicker::toggle()
{
    if (isOpen())
        close();
    else
        open();
}

} // namespace Lingmo::Shell
