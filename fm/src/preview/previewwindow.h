// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "preview/previewcontroller.h"

#include "base/signal.h"
#include "base/timer.h"
#include "ui/geometry.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace Lingmo::ui {
class ImageView;
class Window;
}

namespace fm {

class PreviewRoot;

// The Quick Look window. A compact card for most files,
// a window fitted to an image, a text page; the name and "Open with" in the
// header, arrows for several items.
class PreviewWindow
{
public:
    explicit PreviewWindow(PreviewController& controller);
    ~PreviewWindow();

    bool isShown() const { return m_window != nullptr; }
    // A key another process's view passed on, as if pressed in the window.
    void sendKey(uint32_t key, uint32_t modifiers) { keyPressed(key, modifiers, false); }

    // Arrow keys for the source's file view.
    std::function<void(uint32_t key, uint32_t modifiers)> onNavigate;

private:
    friend class PreviewRoot;

    void open();
    void retranslate();
    void updateGlass();
    void destroy();
    void stage(const PreviewFile& file);
    void present(PreviewFile file);
    bool keyPressed(uint32_t key, uint32_t modifiers, bool repeat);

    double displayRatio() const;
    ui::SizeF screenSize() const;
    double textPageWidth() const;
    ui::SizeF sizeFor(const PreviewFile& file) const;
    ui::SizeF fittedPixels(const PreviewFile& file, ui::SizeF size) const;
    bool multiple() const { return m_controller.items().size() > 1; }

    PreviewController& m_controller;
    std::unique_ptr<ui::Window> m_window;
    PreviewRoot* m_root = nullptr;
    // What is on screen; it trails the controller's file while the next
    // image decodes, so size, title and picture change in one frame.
    PreviewFile m_shown;
    std::optional<PreviewFile> m_pending;
    bool m_opening = false;
    std::optional<bool> m_glassDarkMode;
    // Decodes the next image into the cache before it is shown.
    std::unique_ptr<ui::ImageView> m_preloader;
    base::Timer m_stall;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
