// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "base/geometry.h"
namespace scene = Lingmo::Scene;

struct wl_client;
struct wl_display;
struct wl_global;
struct wl_resource;

namespace Lingmo::Compositor {
class CaptureSource;
}

namespace Lingmo::Render {
class Renderer;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::ui {
class Bitmap;
}

namespace Lingmo::Wayland {

class ImageCaptureManager
{
public:
    using Authorize = std::function<bool(const wl_client *)>;
    using ToplevelSourceResolver =
            std::function<std::shared_ptr<compositor::CaptureSource>(wl_resource *handle)>;

    // What a capture frame is painted from. `scene` paints the whole output
    // for output sources; a window source paints its own node and adds the
    // cursor layers through `cursors`. `cursorBounds` is the cursor image's
    // global logical rectangle (empty while hidden), which is damage a
    // hardware cursor never puts into the tree; `root` is the tree damage is
    // tracked against for output sources. `composited` says whether the
    // frame the output just composited holds what `scene` would paint, so a
    // copy of it can stand in.
    enum class Composited {
        Unusable,
        WithoutCursor,
        WithCursor,
    };

    struct Painters {
        std::function<void(bool cursors)> scene;
        std::function<Composited()> composited;
        std::function<void()> cursors;
        std::function<base::Rect()> cursorBounds;
        const scene::Node *root = nullptr;
    };

    // `bindable` lets a client hold the managers before it may capture, as the
    // computer-use service does until consent; empty means `authorize`.
    ImageCaptureManager(wl_display *display, Authorize authorize, std::function<void()> repaint,
                        Authorize bindable = {});
    ~ImageCaptureManager();
    ImageCaptureManager(const ImageCaptureManager &) = delete;
    ImageCaptureManager &operator=(const ImageCaptureManager &) = delete;

    // Releases the globals; must run before the display is destroyed.
    void shutdown();

    // Whether capture of the session is permitted right now. While a session
    // is locked no frame is rendered at all, rather than filtering what it
    // paints: no capture path then has to be trusted to leave the desktop out.
    // Requests made meanwhile are held and complete after the unlock, so a
    // screen share survives the lock instead of being torn down by it.
    void setCaptureAllowed(std::function<bool()> allowed);

    // Fails every frame currently in flight as retryable. A capture asked for
    // before the session blocked must not complete afterwards, whatever stage
    // it reached: refusing only new requests would let a frame already rendered
    // be handed over after the screen was supposed to be secure.
    void failPending();

    // Window sources come from ext_foreign_toplevel_handle_v1 objects the
    // toplevel list owns; it installs the lookup here.
    void setToplevelSourceResolver(ToplevelSourceResolver resolver);

    // A picture for the compositor itself, as the shell's screenshots take
    // them: a frame of `source` rendered as a session's would be, at the next
    // composition of its output and under the same lock rules. `done` runs on
    // the main loop with the image in the output's buffer orientation and
    // the wl_output transform that turns it upright, or a null image when
    // the frame failed. Dropping the returned handle abandons the request.
    using CaptureDone = std::function<void(ui::Bitmap image, uint32_t transform)>;
    [[nodiscard]] std::shared_ptr<void> capture(std::shared_ptr<compositor::CaptureSource> source,
                                                bool cursors, CaptureDone done);
    // The whole of the output named `output`, for capture().
    static std::shared_ptr<compositor::CaptureSource> outputSource(const std::string &output);

    // DMA-BUF capture targets are advertised from this renderer's probe; null
    // keeps every session SHM-only.
    void setRenderer(const render::Renderer *renderer);

    wl_global *sourceGlobal() const;
    wl_global *toplevelSourceGlobal() const;
    wl_global *copyGlobal() const;
    bool hasPending(const std::string &output) const;
    void renderPending(render::Renderer &renderer, const Painters &painters);
    void renderPending(render::Renderer &renderer, const std::function<void(bool)> &paint);

    // The composition was handed to the display, to be shown at `aimedNs`.
    // Frames already in their buffers are delivered now: waiting for the flip
    // behind a queued frame would capture every other refresh. GPU readbacks
    // into shared memory still wait for presentation.
    void drawn(uint64_t generation, uint64_t aimedNs);
    void presented(uint64_t generation, uint64_t monotonicNs);
    void discarded(uint64_t generation);

    // The pointer moved or changed image. A hardware cursor leaves the scene
    // untouched, so a session that paints cursors needs a composition to see it.
    void cursorChanged();

    struct State;

private:
    std::shared_ptr<State> m_state;
    wl_global *m_sourceGlobal = nullptr;
    wl_global *m_toplevelSourceGlobal = nullptr;
    wl_global *m_copyGlobal = nullptr;
};

} // namespace Lingmo::Wayland
