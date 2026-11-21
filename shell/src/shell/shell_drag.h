// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <memory>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "ui/drag.h"

namespace Lingmo::Compositor {
class Buffer;
class Surface;
}

namespace Lingmo::Input {
class DragOffer;
}

namespace Lingmo::Shell {

class Shell;
class ShellWindow;

// Drags from and onto the shell's windows, through the seat's drag
// controller like a client's wl_data_device drags, as Ash's
// DragDropController serves both exo's surfaces and its own views. A drag
// started in a shell window is a data source the controller offers to
// clients and shell windows alike; a drag over a shell window, whoever
// started it, is offered to its views through ui::DropOffer.
class ShellDrag
{
public:
    explicit ShellDrag(Shell *shell);
    ShellDrag(const ShellDrag &) = delete;
    ShellDrag &operator=(const ShellDrag &) = delete;
    ~ShellDrag();

    // The toolkit's startDrag(): the drag starts on the next loop turn, out
    // of the input dispatch that asked for it, from the pointer button or
    // touch point still held on `window`. False when one is running.
    bool start(ShellWindow *window, ui::DragSource source);

    // The window under a drag, or null.
    ShellWindow *target() const;

    // Called by ShellWindow; not part of the public API.
    void windowDestroying(ShellWindow *window);

private:
    class DataSource;
    class Offer;

    void begin(ShellWindow *window, ui::DragSource source);
    void enter(compositor::Surface *surface, const base::PointF &location);
    void motion(const base::PointF &location);
    void leave();
    void dropped(input::DragOffer *offer);
    void apply(uint32_t action);

    Shell *m_shell;
    // The icon of the drag this shell started, until it ends. A surface only
    // refers to its buffer, as to a client's wl_buffer, so the drag owns it.
    std::unique_ptr<compositor::Surface> m_icon;
    std::shared_ptr<compositor::Buffer> m_iconBuffer;
    bool m_starting = false;
    // The offer to the shell window under the drag; dropped offers stay
    // until their reads are done.
    std::shared_ptr<Offer> m_offer;
    std::vector<std::shared_ptr<Offer>> m_dropped;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

