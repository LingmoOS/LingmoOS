// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/signal.h"
#include "display/display_identity.h"
#include "base/timer.h"
#include "core/main_loop.h"
#include "display/native_display_delegate.h"
#include "render/color_transform.h"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct wl_display;
struct wl_global;

namespace Lingmo {
namespace Backend {
class DrmBackend;
class DrmOutput;
} // namespace backend

namespace Compositor {
class Compositor;
struct Output;
} // namespace compositor

namespace Input {
class InputManager;
}
namespace Render {
class Renderer;
class Texture;
} // namespace render

namespace WM {
class WindowManager;
}

namespace Scene {
class Node;
class Scene;
class ScreenTransitionNode;
} // namespace scene

namespace Cursor {
class Cursor;
class OutputCursorRouter;
} // namespace cursor

namespace Display {
class DisplayManager;

class DisplayRuntime
{
public:
    DisplayRuntime(DisplayManager &manager, backend::DrmBackend *backend,
                   std::vector<compositor::Output *> models, std::vector<wl_global *> &globals,
                   wl_global *&primaryGlobal, wl_display *display,
                   compositor::Compositor &compositor, input::InputManager &input,
                   wm::WindowManager &wm, scene::Scene &scene, cursor::Cursor &cursor,
                   cursor::OutputCursorRouter *router, bool &updating);
    ~DisplayRuntime();
    DisplayRuntime(const DisplayRuntime &) = delete;
    DisplayRuntime &operator=(const DisplayRuntime &) = delete;
    static void initialize(DisplayManager &displayManager, backend::DrmBackend *backend,
                           const std::vector<compositor::Output *> &outputModels,
                           const std::string &compositorInstanceId,
                           compositor::Compositor &compositor,
                           input::InputManager &inputManager);
    std::optional<HardwareSnapshot> reprobe();
    bool secureScanout = false;
    void replaceOutputs(std::vector<compositor::Output *> models);
    ConfigurationResult configure(const DisplayConfiguration &configuration,
                                  NativeDisplayDelegate::ConfigureMode mode);
    ConfigurationResult setPower(const std::vector<DisplayId> &displays,
                                 NativeDisplayDelegate::PowerState state);

    bool isPoweredOff(const DisplayId &id) const
    {
        const auto state = powerStates.find(id);
        return state != powerStates.end() && state->second != NativeDisplayDelegate::PowerState::On;
    }

    std::function<void()> lockRequested;
    // Whether the desktop background is up at the outputs' current size; a
    // rotation's transition waits for it as for the desktop window.
    std::function<bool()> backgroundReady;
    std::function<bool(const std::string &)> capturePending;
    // The largest scale of the applied displays, reported before the outputs
    // are re-sent. Xwayland renders at it, and sizes its screen from them.
    std::function<void(double)> maximumScaleChanged;
    // A frame reached this output and its KMS state holds nothing but the
    // compositor's own framebuffer. The session controller needs it per output:
    // a lock is not complete until every screen has shown the shield with no
    // application plane left beside it.
    std::function<void(const std::string &)> securelyPresented;
    // Whether a frame drawn at the given frame time shows the lock transition over.
    std::function<bool(double)> secureContentSettled;

    // The session is blocked. Every output composites, and every commit clears
    // the application planes of the previous session frame. Applies to outputs
    // that appear later as well, so a hotplug or a DPMS wake mid-lock comes up
    // secure rather than inheriting the session's policy.
    void setSecureScanout(bool secure);
    void withdrawSecurePresentation();

    // The night light's tint, for every output and the ones that appear later.
    void setColorTransform(const render::ColorTransform &transform);

    bool isSecureScanout() const { return secureScanout; }

    // `layers` are what `transition` covers, painted in order.
    void setScreenTransition(scene::ScreenTransitionNode *transition,
                             std::vector<scene::Node *> layers);
    // Each lit output's frame of what `paint` draws, keyed by output name, as
    // the desk switch animation starts from.
    std::map<std::string, std::unique_ptr<render::Texture>>
    captureOutputs(const std::function<void(render::Renderer &)> &paint);

    // Called before the first layout is published: the frame each output shows
    // as the compositor starts, another DRM master's last one, stays in `cover`
    // over `layers` until `ready` holds, by default once a desktop covers every
    // output, then fades out. Upstream keeps its BootingAnimationController
    // widget on top for the same reason: no other UI blinks before the screen
    // it leads to is ready.
    void setStartupCover(scene::ScreenTransitionNode *cover, std::vector<scene::Node *> layers,
                         std::function<bool()> ready = {});

    // Whether the startup cover still hides the session, or is yet to. It
    // fades out a still copy, so nothing should move under it meanwhile.
    bool isStartupCoverShowing() const;

    // Blocked outputs carry over the frame they start on as well; for the login
    // screen, which has no session of its own on screen.
    bool carryOverWhileSecure = false;

    using NativeConfigure = std::function<ConfigurationResult(
            const DisplayConfiguration &, NativeDisplayDelegate::ConfigureMode)>;

    void setNativeConfigureForTesting(NativeConfigure configure)
    {
        nativeConfigure = std::move(configure);
    }

private:
    ConfigurationResult configureExact(const DisplayConfiguration &configuration,
                                       NativeDisplayDelegate::ConfigureMode mode);
    NativeConfigure nativeConfigure;
    void requireNativeRecovery();
    void publish();
    void captureOutgoingFrames();
    void startStartupCover();
    void bindFrames();
    static HardwareSnapshot discover(backend::DrmBackend *backend,
                                     const std::vector<compositor::Output *> &models,
                                     const std::string &instance, uint64_t generation,
                                     DisplayIdentity &identity);
    base::Subscription activityConnection;
    std::set<DisplayId> wakeOnInput;
    DisplayIdentity identity;
    std::vector<std::unique_ptr<compositor::Output>> ownedModels;
    uint64_t nextFrameToken = 0;
    struct OutputBinding {
        std::vector<base::Subscription> subscriptions;
        base::Subscription repaint;
    };
    std::map<backend::DrmOutput *, OutputBinding> boundOutputs;
    render::ColorTransform colorTransform;

    DisplayManager &displayManager;
    backend::DrmBackend *backend;
    std::vector<compositor::Output *> outputModels;
    std::vector<wl_global *> &outputGlobals;
    wl_global *&outputGlobal;
    wl_display *waylandDisplay;
    compositor::Compositor &compositor;
    input::InputManager &inputManager;
    wm::WindowManager &windowManager;
    scene::Scene &scene;
    cursor::Cursor &cursor;
    cursor::OutputCursorRouter *cursorRouter;
    bool &updatingOutput;
    std::unordered_map<DisplayId, NativeDisplayDelegate::PowerState> powerStates;
    scene::ScreenTransitionNode *screenTransition = nullptr;
    std::vector<scene::Node *> transitionLayers;
    base::Timer transitionDeadline {core::mainLoop()};
    scene::ScreenTransitionNode *startupCover = nullptr;
    std::vector<scene::Node *> startupCoverLayers;
    std::function<bool()> startupCoverReady;
    bool startupCoverPending = false;
    base::Timer startupCoverDeadline {core::mainLoop()};
};
} // namespace display
} // namespace Lingmo
