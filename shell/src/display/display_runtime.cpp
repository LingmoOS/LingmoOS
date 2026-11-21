// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display/display_runtime.h"
#include "backend/drm/drm_backend.h"
#include "backend/drm/drm_mode_fallback_planner.h"
#include "backend/drm/drm_output.h"
#include "backend/drm/drm_selection.h"
#include "base/strings.h"
#include "compositor/compositor.h"
#include "compositor/output.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "cursor/cursor.h"
#include "cursor/output_cursor_router.h"
#include "display/display_configuration_validator.h"
#include "display/display_manager.h"
#include "display/display_preferences.h"
#include "display/edid.h"
#include "display/mirror_transform.h"
#include "input/input_manager.h"
#include "render/egl_renderer.h"
#include "render/renderer.h"
#include "scene/scene.h"
#include "scene/screen_transition_node.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/server_util.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <format>
namespace scene = Lingmo::Scene;

namespace Lingmo::Display {

namespace {

// The output's logical rectangle.
Lingmo::Base::Rect logicalBounds(const Lingmo::Compositor::Output &model)
{
    return Lingmo::Base::Rect(model.position, model.logicalSize());
}

Lingmo::Render::OutputRenderContext renderContext(const Lingmo::Compositor::Output &model,
                                                    uint64_t frame)
{
    return {model.name, logicalBounds(model), model.pixelSize, model.renderScale,
            static_cast<uint32_t>(model.transform), frame, model.nativeContentRect};
}

// How long the outgoing frame may wait for the shell to redraw. Exo holds its
// orientation compositor lock for up to 2.5 s; the shell redraws far faster.
constexpr std::chrono::milliseconds kScreenTransitionTimeout(1000);

// How long the startup cover waits for the desktop's wallpaper. Upstream's booting
// animation waits at most 5 s for the device before it plays anyway.
constexpr std::chrono::milliseconds kStartupCoverTimeout(5000);

} // namespace

DisplayRuntime::DisplayRuntime(DisplayManager &manager, backend::DrmBackend *nativeBackend,
                               std::vector<compositor::Output *> models,
                               std::vector<wl_global *> &globals, wl_global *&primaryGlobal,
                               wl_display *display, compositor::Compositor &compositor,
                               input::InputManager &input, wm::WindowManager &wm,
                               scene::Scene &scene, cursor::Cursor &cursor,
                               cursor::OutputCursorRouter *router, bool &updating)
    : displayManager(manager)
    , backend(nativeBackend)
    , outputModels(std::move(models))
    , outputGlobals(globals)
    , outputGlobal(primaryGlobal)
    , waylandDisplay(display)
    , compositor(compositor)
    , inputManager(input)
    , windowManager(wm)
    , scene(scene)
    , cursor(cursor)
    , cursorRouter(router)
    , updatingOutput(updating)
{
    activityConnection = inputManager.userActivity.connect([this] {
        if (wakeOnInput.empty())
            return;
        const auto result = setPower({wakeOnInput.begin(), wakeOnInput.end()},
                                     NativeDisplayDelegate::PowerState::On);
        if (!result.succeeded)
            LM_WARNING(base::LogCategory::defaultCategory())
                    << "Unable to wake displays:" << result.error.message;
    });
    identity.seed(displayManager.hardware()->displays);
    transitionDeadline.setSingleShot(true);
    transitionDeadline.onTimeout = [this] {
        if (screenTransition)
            screenTransition->notifyChanged();
    };
    startupCoverDeadline.setSingleShot(true);
    startupCoverDeadline.onTimeout = [this] {
        if (startupCover)
            startupCover->notifyChanged();
    };
    bindFrames();
    displayManager.setPublisher([this] {
        publish();
    });
    inputManager.setDisplayManager(&displayManager);
    publish();
}

DisplayRuntime::~DisplayRuntime()
{
    activityConnection.reset();
    if (backend) {
        const auto detach = [](const auto &outputs) {
            for (const auto &output : outputs) {
                output->presentationSubmitted = {};
                output->setSecureContentCheck({});
                output->setBeginFrameCallback({});
                output->setPrepareCallback({});
                output->setRenderCallback({});
                output->setScanoutPlanner({});
                output->setFullRepaintQuery({});
            }
        };
        detach(backend->allOutputs());
        detach(backend->allOutputs(true));
    }
    for (auto &[output, binding] : boundOutputs)
        binding.repaint.reset();
    boundOutputs.clear();
    cursor.setSink(nullptr);
    displayManager.setPublisher({});
    inputManager.setDisplayManager(nullptr);
    // The readiness check refers to this runtime.
    if (screenTransition)
        screenTransition->releaseRenderResources();
}

HardwareSnapshot DisplayRuntime::discover(backend::DrmBackend *backend,
                                          const std::vector<compositor::Output *> &models,
                                          const std::string &instance, uint64_t generation,
                                          DisplayIdentity &identity)
{
    std::vector<Lingmo::Display::DisplaySnapshot> physicalDisplays;
    for (size_t index = 0; index < models.size(); ++index) {
        const auto *model = models[index];
        auto *nativeBackend = backend && index < backend->allOutputs().size()
                ? backend->allOutputs()[index]->backend()
                : nullptr;
        const auto displayMode = [](const drmModeModeInfo &nativeMode) {
            Lingmo::Display::DisplayMode mode;
            mode.pixelSize = base::Size(nativeMode.hdisplay, nativeMode.vdisplay);
            mode.refreshMilliHz = static_cast<int>(
                    std::lround(Lingmo::Backend::refreshRate(nativeMode) * 1000.0));
            mode.pixelClockKHz = nativeMode.clock;
            mode.horizontalTotal = nativeMode.htotal;
            mode.verticalTotal = nativeMode.vtotal;
            mode.flags = nativeMode.flags;
            mode.name = base::strings::latin1ToUtf8(std::string_view(
                    nativeMode.name, strnlen(nativeMode.name, sizeof(nativeMode.name))));
            mode.preferred = nativeMode.type & DRM_MODE_TYPE_PREFERRED;
            return mode;
        };
        Lingmo::Display::DisplayMode mode;
        std::vector<Lingmo::Display::DisplayMode> modes;
        if (backend && index < backend->allOutputs().size()) {
            const auto *drmOutput = backend->allOutputs()[index];
            mode = displayMode(drmOutput->mode());
            for (const auto &nativeMode : drmOutput->modes())
                modes.push_back(displayMode(nativeMode));
        } else {
            mode.pixelSize = model->pixelSize;
            mode.refreshMilliHz = model->refreshMilliHz;
            mode.pixelClockKHz = model->pixelSize.width() * model->pixelSize.height() * 60 / 1000;
            mode.horizontalTotal = model->pixelSize.width();
            mode.verticalTotal = model->pixelSize.height();
            mode.name = std::format("{}x{}@{}", mode.pixelSize.width(), mode.pixelSize.height(),
                                    mode.refreshMilliHz);
            mode.preferred = model->preferred;
            modes = {mode};
            for (const base::Size size : {base::Size(1600, 900), base::Size(1280, 720)}) {
                auto alternative = mode;
                alternative.pixelSize = size;
                alternative.pixelClockKHz = size.width() * size.height() * 60 / 1000;
                alternative.horizontalTotal = size.width();
                alternative.verticalTotal = size.height();
                alternative.name = std::format("{}x{}@60000", size.width(), size.height());
                alternative.preferred = false;
                if (alternative.stableKey() != mode.stableKey())
                    modes.push_back(alternative);
            }
        }
        Lingmo::Display::DisplaySnapshot physical;
        EdidInfo edid;
        const std::string modelName = model->model;
        const std::string outputName = model->name;
        physical.persistentKey = "session:" + modelName;
        physical.portKey = (nativeBackend ? nativeBackend->deviceKey() : std::string("virtual"))
                + '/' + outputName;
        if (backend && index < backend->allOutputs().size()) {
            physical.portKey =
                    nativeBackend->deviceKey() + '/' + backend->allOutputs()[index]->portPath();
            edid = parseEdid(backend->allOutputs()[index]->edid());
            physical.edidKey = edid.key;
        }
        const std::string route = backend && index < backend->allOutputs().size()
                ? backend->allOutputs()[index]->portPath()
                : outputName;
        using base::strings::CaseSensitivity;
        const std::vector<std::string> connectorPath = base::strings::split(route, '-');
        physical.baseConnectorKey =
                base::strings::equalsIgnoreAsciiCase(connectorPath.front(), "DP")
                        && connectorPath.size() > 2
                ? connectorPath[0] + '-' + connectorPath[1]
                : route;
        physical.pathTopology = connectorPath;
        physical.connectorName = outputName;
        physical.devicePath = nativeBackend ? nativeBackend->devicePath() : std::string("virtual");
        physical.make = edid.make.empty() ? model->make : edid.make;
        physical.model = edid.model.empty() ? modelName : edid.model;
        physical.physicalSizeMm = model->physicalSizeMm;
        const std::string connector =
                nativeBackend ? backend->allOutputs()[index]->name() : outputName;
        const std::string &driver = nativeBackend ? nativeBackend->driverName() : std::string();
        physical.connectionType =
                (!nativeBackend || driver == "virtio_gpu" || driver == "vmwgfx" || driver == "qxl"
                 || driver == "vkms")
                ? Lingmo::Display::DisplayConnectionType::Virtual
                : (base::strings::startsWith(connector, "eDP", CaseSensitivity::Insensitive)
                   || base::strings::startsWith(connector, "LVDS", CaseSensitivity::Insensitive)
                   || base::strings::startsWith(connector, "DSI", CaseSensitivity::Insensitive))
                ? Lingmo::Display::DisplayConnectionType::Internal
                : Lingmo::Display::DisplayConnectionType::External;
        physical.capabilities = Lingmo::Display::DisplayCapability::Scale
                | Lingmo::Display::DisplayCapability::Rotation
                | Lingmo::Display::DisplayCapability::SoftwareMirror
                | Lingmo::Display::DisplayCapability::Power;
        // Through the CRTC, or blended over the frame where it cannot.
        if (backend && index < backend->allOutputs().size())
            physical.capabilities |= Lingmo::Display::DisplayCapability::ColorTransform;
        physical.modes = modes;
        physical.currentMode = mode;
        physical.nativeMode = mode;
        for (const auto &candidate : modes) {
            if (candidate.preferred) {
                physical.nativeMode = candidate;
                break;
            }
        }
        physicalDisplays.push_back(std::move(physical));
    }
    identity.assign(physicalDisplays);
    for (auto *model : models) {
        for (const auto &physical : physicalDisplays) {
            if (physical.connectorName == model->name)
                model->displayId = physical.sessionId.value();
        }
    }
    return {instance, "seat0", generation, physicalDisplays};
}

void DisplayRuntime::initialize(DisplayManager &displayManager, backend::DrmBackend *backend,
                                const std::vector<compositor::Output *> &outputModels,
                                const std::string &compositorInstanceId,
                                compositor::Compositor &compositor,
                                input::InputManager &inputManager)
{
    const auto &models = outputModels;
    DisplayIdentity identity;
    identity.rememberPersistentKeys(DisplayPreferences().persistentKeys());
    const auto snapshot = discover(backend, models, compositorInstanceId, 1, identity);
    if (!displayManager.installHardware(snapshot))
        return;

    std::vector<Lingmo::Display::LogicalDisplay> logicalDisplays;
    std::vector<Lingmo::Compositor::Compositor::OutputView> views;
    std::vector<base::Rect> topology;
    for (size_t index = 0; index < models.size(); ++index) {
        const auto *model = models[index];
        const Lingmo::Display::DisplayId id(model->displayId);
        Lingmo::Display::LogicalDisplay logical;
        logical.id = id;
        logical.destinations = {id};
        logical.waylandName = model->name;
        logical.bounds = base::Rect(model->position, model->logicalSize());
        logical.workArea = logical.bounds;
        logical.scale = model->scale;
        logical.primary = index == 0;
        logicalDisplays.push_back(logical);
        views.push_back({model->displayId, logical.bounds, logical.scale, model->name});
        topology.push_back(logical.bounds);
    }
    Lingmo::Display::AppliedConfiguration applied;
    applied.compositorInstanceId = compositorInstanceId;
    applied.topologyGeneration = 1;
    applied.revision = displayManager.applied() ? displayManager.applied()->revision + 1 : 1;
    applied.physicalDisplays = displayManager.hardware()->displays;
    applied.logicalDisplays = std::move(logicalDisplays);
    for (size_t index = 1; index < applied.logicalDisplays.size(); ++index)
        applied.placements.push_back({applied.logicalDisplays[index].id,
                                   applied.logicalDisplays[index - 1].id,
                                   Lingmo::Display::LayoutEdge::Right, 0});
    displayManager.installApplied(std::move(applied));
    compositor.setOutputViews(std::move(views));
    inputManager.setTopology(topology, topology.empty() ? base::Rect() : topology.front());
}

void DisplayRuntime::replaceOutputs(std::vector<compositor::Output *> models)
{
    cursor.setSink(nullptr);
    if (cursorRouter)
        cursorRouter->setTargets({});
    // A connector that is still there keeps its wl_output global, with the
    // bindings and capture sources clients hold on it; publish() sends it the
    // new metrics. Only a connector that went away is withdrawn. Withdrawing
    // and re-announcing an unchanged output on every reprobe - the seat
    // activation at startup included - is a hotplug clients need not see, and
    // one SDL2 cannot survive while it recreates its window.
    std::vector<wl_global *> kept(models.size(), nullptr);
    for (int index = 0; index < outputModels.size() && index < int(outputGlobals.size());
         ++index) {
        auto *global = outputGlobals[index];
        if (!global)
            continue;
        int next = -1;
        for (int candidate = 0; candidate < models.size() && next < 0; ++candidate) {
            if (models.at(candidate)->name == outputModels.at(index)->name && !kept[candidate])
                next = candidate;
        }
        if (next < 0)
            wayland::removeOutputGlobal(global);
        else
            kept[next] = global;
    }
    outputGlobal = nullptr;
    outputGlobals = std::move(kept);
    outputModels = std::move(models);
    bindFrames();
}

std::optional<HardwareSnapshot> DisplayRuntime::reprobe()
{
    if (!backend)
        return std::nullopt;
    cursor.setSink(nullptr);
    if (cursorRouter)
        cursorRouter->setTargets({});
    if (!backend->reprobe())
        return std::nullopt;
    std::vector<std::unique_ptr<compositor::Output>> replacement;
    std::vector<compositor::Output *> models;
    for (const auto &output : backend->allOutputs()) {
        auto model = std::make_unique<compositor::Output>();
        model->name = output->publicName();
        model->model = output->name();
        model->pixelSize = output->size();
        model->physicalSizeMm = output->physicalSizeMm();
        model->subpixel = output->subpixel();
        models.push_back(model.get());
        replacement.push_back(std::move(model));
    }
    const auto hardware = discover(backend, models, displayManager.hardware()->compositorInstanceId,
                                   displayManager.hardware()->topologyGeneration + 1, identity);
    replaceOutputs(models);
    ownedModels = std::move(replacement);
    // A display that went away takes its power state and wake-on-input entry
    // along, or every later input event would try to power it on.
    std::set<DisplayId> present;
    for (const auto &display : hardware.displays)
        present.insert(display.sessionId);
    std::erase_if(powerStates, [&](const auto &entry) { return !present.contains(entry.first); });
    std::erase_if(wakeOnInput, [&](const auto &id) { return !present.contains(id); });
    return hardware;
}

void DisplayRuntime::setSecureScanout(bool secure)
{
    if (secureScanout == secure)
        return;
    secureScanout = secure;
    if (!backend)
        return;
    for (const auto &output : backend->allOutputs())
        output->setSecureScanout(secure);
}

void DisplayRuntime::withdrawSecurePresentation()
{
    if (!backend)
        return;
    for (const auto &output : backend->allOutputs())
        output->withdrawSecurePresentation();
}

void DisplayRuntime::setColorTransform(const render::ColorTransform &transform)
{
    colorTransform = transform;
    if (!backend)
        return;
    for (const auto &output : backend->allOutputs())
        output->setColorTransform(transform);
}

void DisplayRuntime::setScreenTransition(scene::ScreenTransitionNode *transition,
                                         std::vector<scene::Node *> layers)
{
    screenTransition = transition;
    transitionLayers = std::move(layers);
}

std::map<std::string, std::unique_ptr<render::Texture>>
DisplayRuntime::captureOutputs(const std::function<void(render::Renderer &)> &paint)
{
    std::map<std::string, std::unique_ptr<render::Texture>> frames;
    if (!backend)
        return frames;
    const auto outputs = backend->allOutputs();
    for (int index = 0; index < outputModels.size() && index < int(outputs.size()); ++index) {
        auto *model = outputModels.at(index);
        if (model->logicalExtent.isEmpty())
            continue;
        auto texture = outputs[index]->renderOffscreen(
                renderContext(*model, 0), paint);
        if (texture)
            frames[model->name] = std::move(texture);
    }
    return frames;
}

void DisplayRuntime::captureOutgoingFrames()
{
    if (!screenTransition || !backend)
        return;
    screenTransition->stopAnimating();
    const auto &applied = *displayManager.applied();
    const auto paintOutgoing = [this](render::Renderer &renderer) {
        renderer.clear(scene.background());
        for (auto *layer : transitionLayers)
            layer->render(renderer);
        screenTransition->render(renderer);
    };

    scene::ScreenTransitionNode::Outputs rotated;
    std::vector<base::Rect> outgoing;
    std::vector<base::Rect> covers;
    for (int index = 0; index < outputModels.size() && index < int(backend->allOutputs().size());
         ++index) {
        auto *model = outputModels.at(index);
        // Only a published model has a frame on screen to hold.
        if (model->logicalExtent.isEmpty())
            continue;
        const DisplayId destination(model->displayId);
        const LogicalDisplay *logical = nullptr;
        for (const auto &candidate : applied.logicalDisplays) {
            if (candidate.hasDestination(destination)) {
                logical = &candidate;
                break;
            }
        }
        const int quarterTurns = logical ? int(logical->rotation) - int(model->transform) : 0;
        if (quarterTurns == 0)
            continue;
        const base::Rect outgoingBounds = logicalBounds(*model);
        auto texture = backend->allOutputs()[index]->renderOffscreen(
                renderContext(*model, 0), paintOutgoing);
        if (!texture)
            continue;
        // Renderer transforms turn logical content by -90 degrees per step, so
        // the outgoing frame stays put on the panel when turned by the difference.
        static constexpr int kDegrees[] = {0, 90, -180, -90};
        rotated[model->name] = {std::move(texture), kDegrees[(quarterTurns % 4 + 4) % 4]};
        outgoing.push_back(outgoingBounds);
        covers.push_back(logical->bounds);
    }
    if (rotated.empty())
        return;

    // Upstream extends the compositor lock until the wallpaper and apps have
    // resized. Here the shell's surfaces on a rotated output have to commit
    // again, every desktop has to cover its output and the wallpaper has to be
    // decoded at the new size.
    struct TrackedSurface {
        uint64_t window;
        uint64_t revision;
    };

    std::vector<TrackedSurface> tracked;
    bool hasDesktop = false;
    for (auto *window : windowManager.windows()) {
        const auto role = window->role();
        if (role != wm::WindowRole::Desktop && role != wm::WindowRole::Panel
            && role != wm::WindowRole::Dock)
            continue;
        const auto *node = window->rootSurface() ? window->rootSurface()->node() : nullptr;
        if (!node || !node->isEffectivelyVisible())
            continue;
        hasDesktop |= role == wm::WindowRole::Desktop;
        if (std::any_of(outgoing.begin(), outgoing.end(), [&](const base::Rect &bounds) {
                return node->absoluteGeometry().intersects(bounds);
            }))
            tracked.push_back({window->id(), node->contentRevision()});
    }
    if (!hasDesktop)
        covers.clear();
    const auto ready = [this, tracked = std::move(tracked), covers = std::move(covers)] {
        if (backgroundReady && !backgroundReady())
            return false;
        for (const auto &surface : tracked) {
            auto *window = windowManager.windowById(surface.window);
            const auto *node =
                    window && window->rootSurface() ? window->rootSurface()->node() : nullptr;
            if (node && node->isEffectivelyVisible() && node->contentRevision() == surface.revision)
                return false;
        }
        for (const base::Rect &bounds : covers) {
            const auto &windows = windowManager.windows();
            if (std::none_of(windows.begin(), windows.end(), [&](const wm::Window *window) {
                    const auto *node =
                            window->rootSurface() ? window->rootSurface()->node() : nullptr;
                    return window->role() == wm::WindowRole::Desktop && node
                            && node->isEffectivelyVisible()
                            && node->absoluteGeometry().contains(bounds);
                }))
                return false;
        }
        return true;
    };
    screenTransition->start(
            std::move(rotated),
            [this](render::Renderer &renderer) {
                renderer.clear(scene.background());
                for (auto *layer : transitionLayers)
                    layer->render(renderer);
            },
            ready, int(kScreenTransitionTimeout.count()));
    transitionDeadline.start(kScreenTransitionTimeout);
}

void DisplayRuntime::setStartupCover(scene::ScreenTransitionNode *cover,
                                     std::vector<scene::Node *> layers,
                                     std::function<bool()> ready)
{
    startupCover = cover;
    startupCoverLayers = std::move(layers);
    startupCoverReady = std::move(ready);
    startupCoverPending = cover != nullptr;
    if (!backend)
        return;
    for (auto *output : backend->allOutputs())
        output->setKeepCarriedFrame(startupCoverPending);
}

bool DisplayRuntime::isStartupCoverShowing() const
{
    return startupCover && backend && (startupCoverPending || startupCover->isActive());
}

void DisplayRuntime::startStartupCover()
{
    const auto &applied = *displayManager.applied();
    // A reconciliation publishes an empty layout before it configures: only
    // the first real one has carried frames, and only it gets the cover. A
    // later one, a VT switch back included, shows the session at once.
    if (!startupCoverPending || !startupCover || !backend || applied.logicalDisplays.empty())
        return;
    startupCoverPending = false;
    const auto outputs = backend->allOutputs();
    scene::ScreenTransitionNode::Outputs carried;
    std::vector<base::Rect> covers;
    for (int index = 0; index < int(outputs.size()); ++index) {
        outputs[index]->setKeepCarriedFrame(false);
        auto frame = outputs[index]->takeCarriedFrame();
        if (!frame || index >= outputModels.size())
            continue;
        const auto *model = outputModels.at(index);
        const DisplayId destination(model->displayId);
        const auto logical = std::find_if(applied.logicalDisplays.begin(),
                                          applied.logicalDisplays.end(), [&](const auto &candidate) {
                                              return candidate.hasDestination(destination);
                                          });
        if (logical == applied.logicalDisplays.end())
            continue;
        // The carried framebuffer is the panel's, unturned; like an outgoing
        // frame, it stays put when turned by the new rotation.
        static constexpr int kDegrees[] = {0, 90, -180, -90};
        carried[model->name] = {std::move(frame), kDegrees[int(logical->rotation) % 4]};
        covers.push_back(logical->bounds);
    }
    if (carried.empty())
        return;
    auto desktopsCover = [this, covers = std::move(covers)] {
        const auto &windows = windowManager.windows();
        return std::all_of(covers.begin(), covers.end(), [&](const base::Rect &bounds) {
            return std::any_of(windows.begin(), windows.end(), [&](const wm::Window *window) {
                const auto *node = window->rootSurface() ? window->rootSurface()->node() : nullptr;
                return window->role() == wm::WindowRole::Desktop && node
                        && node->isEffectivelyVisible()
                        && node->absoluteGeometry().contains(bounds);
            });
        });
    };
    startupCover->start(
            std::move(carried),
            [this](render::Renderer &renderer) {
                renderer.clear(scene.background());
                for (auto *layer : startupCoverLayers)
                    layer->render(renderer);
            },
            startupCoverReady ? startupCoverReady : std::function<bool()>(std::move(desktopsCover)),
            int(kStartupCoverTimeout.count()));
    startupCoverDeadline.start(kStartupCoverTimeout);
}

void DisplayRuntime::bindFrames()
{
    if (!backend)
        return;
    for (int index = 0; index < int(backend->allOutputs().size()); ++index) {
        auto *output = backend->allOutputs()[index];
        if (boundOutputs.contains(output))
            continue;
        auto &binding = boundOutputs[output];
        binding.subscriptions.push_back(output->destroyed.connect([this, output] {
            boundOutputs[output].repaint.reset();
            boundOutputs.erase(output);
        }));
        auto *model = outputModels.at(index);
        auto tokens = std::make_shared<std::unordered_map<uint64_t, uint64_t>>();
        output->presentationOutput = model->presentation;
        output->setDirectScanoutEnabled(true);
        output->setSecureScanout(secureScanout);
        output->setSecureContentCheck([this](double frameTime) {
            return !secureContentSettled || secureContentSettled(frameTime);
        });
        output->setKeepCarriedFrame(startupCoverPending);
        output->setCarryOverWhileSecure(carryOverWhileSecure);
        // Before its first modeset, which carries it.
        output->setColorTransform(colorTransform);
        output->setScanoutPlanner([this, model](const base::Size &size, size_t maxOverlays) {
            const auto &applied = displayManager.applied();
            if (!applied || applied->logicalDisplays.size() != 1
                || applied->logicalDisplays.front().destinations.size() != 1
                || (capturePending && capturePending(model->name))
                || !model->position.isOrigin()
                || model->transform != compositor::Output::Transform::Normal
                || !base::fuzzyCompare(model->renderScale, model->scale.value()))
                return scene::OverlayPlan();
            return scene::proposeOverlays(*scene.root(), size, maxOverlays,
                                          model->scale.value());
        });
        output->setBeginFrameCallback([this, model](double frameTime) {
            compositor.frameStarted(frameTime);
            // Like viz's Display without damage, an output that shows nothing
            // new and owes no client a frame draws and flips nothing.
            return scene.hasDamage(renderContext(*model, 0))
                    || compositor.awaitsFrame(base::Rect(model->position, model->logicalSize()))
                    || (capturePending && capturePending(model->name));
        });
        output->setPrepareCallback([this, model, tokens](render::Renderer &renderer,
                                                         uint64_t generation) {
            const auto token = ++nextFrameToken;
            tokens->insert_or_assign(generation, token);
            renderer.setOutputContext(renderContext(*model, token));
            compositor.updateTextures(renderer);
            compositor.frameSubmittedForOutput(base::Rect(model->position, model->logicalSize()),
                                               token);
        });
        output->setRenderCallback([this](render::Renderer &renderer, uint64_t) {
            scene.render(renderer);
            compositor.outputRendered(renderer);
        });
        output->presentationSubmitted = [this, model](const auto &submission) {
            compositor.abandonUnsubmittedPresentation(
                    base::Rect(model->position, model->logicalSize()), submission);
        };
        binding.subscriptions.push_back(output->imagePresented.connect(
                [this, tokens](uint64_t generation, uint64_t timeNs) {
                    if (tokens->contains(generation))
                        compositor.outputPresented(tokens->at(generation), timeNs);
                }));
        // exo runs wl_surface.frame on the ack viz sends from
        // Display::DrawAndSwap (RunDrawCallbacks), not on presentation. Paced
        // by flips, a client behind a queued frame draws every other refresh.
        binding.subscriptions.push_back(output->frameDrawn.connect(
                [this, tokens](uint64_t generation, uint64_t aimedNs) {
                    if (!tokens->contains(generation))
                        return;
                    compositor.frameDrawn(Lingmo::Wayland::nowInMilliseconds(), tokens->at(generation));
                    compositor.outputDrawn(tokens->at(generation), aimedNs);
                }));
        binding.subscriptions.push_back(output->framePresented.connect(
                [this, tokens, model, output](uint32_t, uint64_t generation) {
                    if (!tokens->erase(generation))
                        return;
                    if (securelyPresented && output->lastPresentedFrameWasSecure())
                        securelyPresented(model->displayId);
                }));
        binding.subscriptions.push_back(
                output->frameDiscarded.connect([this, tokens](uint64_t generation) {
                    if (const auto token = tokens->extract(generation))
                        compositor.frameDiscarded(token.mapped());
                }));
        output->setFullRepaintQuery([this, model] {
            return scene.requiresFullRepaint(logicalBounds(*model));
        });
        binding.repaint = compositor.repaintNeeded.connect([output](bool decoration) {
            output->scheduleFrame(decoration ? backend::DrmOutput::Reason::Decoration
                                             : backend::DrmOutput::Reason::Surface);
        });
    }
}

ConfigurationResult DisplayRuntime::configure(const DisplayConfiguration &configuration,
                                              NativeDisplayDelegate::ConfigureMode mode)
{
    const auto exact = [&](const DisplayConfiguration &candidate,
                           NativeDisplayDelegate::ConfigureMode phase) {
        auto result = nativeConfigure ? nativeConfigure(candidate, phase)
                                      : configureExact(candidate, phase);
        if (!result.succeeded && result.error.code == DisplayErrorCode::RevertFailed
            && displayManager.applied())
            requireNativeRecovery();
        return result;
    };
    if (!configuration.allowModeFallback || mode != NativeDisplayDelegate::ConfigureMode::TestOnly)
        return exact(configuration, mode);
    if (!displayManager.hardware())
        return {false,
                false,
                {DisplayErrorCode::Unavailable, "No hardware snapshot", {}},
                std::nullopt};
    const auto hardware = *displayManager.hardware();
    const uint64_t revision = displayManager.applied() ? displayManager.applied()->revision : 0;
    const auto validated =
            DisplayConfigurationValidator::validateExtended(hardware, revision, configuration);
    if (!validated.succeeded)
        return {false, false, validated.error, std::nullopt};
    std::vector<backend::DrmFallbackHead> heads;
    for (const auto &request : validated.normalized.displays) {
        const auto physical = std::find_if(hardware.displays.begin(), hardware.displays.end(),
                                           [&](const auto &item) {
                                               return item.sessionId == request.display;
                                           });
        backend::DrmFallbackHead head;
        head.connectorName = physical->connectorName;
        head.baseConnectorKey = physical->devicePath + ':' + physical->baseConnectorKey;
        head.internal =
                !request.enabled || physical->connectionType != DisplayConnectionType::External;
        const auto append = [&](const DisplayMode &candidate, bool preferred) {
            head.modes.push_back({candidate.stableKey(), candidate.pixelSize.width(),
                               candidate.pixelSize.height(), candidate.refreshMilliHz,
                               candidate.pixelClockKHz, preferred});
        };
        append(request.mode, true);
        if (!head.internal) {
            for (const auto &candidate : physical->modes) {
                if (candidate.pixelClockKHz > 0
                    && candidate.pixelClockKHz < request.mode.pixelClockKHz
                    && candidate.pixelSize.width() <= request.mode.pixelSize.width()
                    && candidate.pixelSize.height() <= request.mode.pixelSize.height()
                    && candidate.refreshMilliHz <= request.mode.refreshMilliHz)
                    append(candidate, false);
            }
        }
        heads.push_back(head);
    }
    ConfigurationResult last;
    DisplayConfiguration selected;
    const uint64_t epoch = backend ? backend->topologyEpoch() : 0;
    int testRounds = 0;
    std::vector<std::string> diagnostics;
    auto plan = backend::DrmModeFallbackPlanner::choose(
            heads,
            [&](const auto &modes) {
                const auto current = [&] {
                    return displayManager.hardware()
                            && displayManager.hardware()->topologyGeneration
                            == hardware.topologyGeneration
                            && (!backend || backend->topologyEpoch() == epoch);
                };
                if (testRounds >= 7)
                    return backend::DrmFallbackTestResult {false, {}, last.error.message, false};
                if (!current()) {
                    last = {false,
                            false,
                            {DisplayErrorCode::StaleConfiguration,
                             "Hardware invalidated during recovery testing",
                             {}},
                            std::nullopt};
                    return backend::DrmFallbackTestResult {false, {}, last.error.message, false};
                }
                selected = validated.normalized;
                selected.allowModeFallback = false;
                for (size_t i = 0; i < modes.size(); ++i) {
                    auto &request = selected.displays[i];
                    for (const auto &physical : hardware.displays) {
                        if (physical.sessionId != request.display)
                            continue;
                        for (const auto &candidate : physical.modes)
                            if (candidate.stableKey() == modes[i].key)
                                request.mode = candidate;
                    }
                }
                const auto checked = DisplayConfigurationValidator::validateExtended(
                        hardware, revision, selected);
                if (!checked.succeeded) {
                    last = {false, false, checked.error, std::nullopt};
                    return backend::DrmFallbackTestResult {false, {}, last.error.message};
                }
                selected = checked.normalized;
                ++testRounds;
                last = exact(selected, NativeDisplayDelegate::ConfigureMode::TestOnly);
                int resources = 0;
                while (!last.succeeded && last.error.code == DisplayErrorCode::TestFailed
                       && current() && testRounds < 7 && resources < 3 && backend
                       && backend->tryNextConfigurationResources()) {
                    if (!current())
                        break;
                    bindFrames();
                    ++resources;
                    ++testRounds;
                    LM_INFO(lingmoAurora)
                            << "Retrying native route/buffer allocation, round" << testRounds;
                    last = exact(selected, NativeDisplayDelegate::ConfigureMode::TestOnly);
                }
                std::vector<std::string> keys;
                for (size_t i = 0; i < modes.size(); ++i)
                    keys.push_back(heads[i].connectorName + '=' + modes[i].key);
                diagnostics.push_back(
                        std::format("round {} [{}]: {}", testRounds, base::strings::join(keys, ","),
                                    last.succeeded ? std::string("passed") : last.error.message));
                LM_INFO(lingmoAurora) << "Display recovery" << diagnostics.back();
                if (!current())
                    last = {false,
                            false,
                            {DisplayErrorCode::StaleConfiguration,
                             "Hardware invalidated during recovery testing",
                             {}},
                            std::nullopt};
                std::vector<std::string> failedGroups;
                for (const auto &id : last.error.affectedDisplays) {
                    for (const auto &physical : hardware.displays)
                        if (physical.sessionId == id)
                            failedGroups.push_back(physical.devicePath + ':'
                                                   + physical.baseConnectorKey);
                }
                return backend::DrmFallbackTestResult {
                        last.succeeded, failedGroups, last.error.message,
                        last.error.code == DisplayErrorCode::TestFailed};
            },
            7);
    if (!plan && last.error.code == DisplayErrorCode::TestFailed && testRounds < 8
        && validated.normalized.displays.size() > 1 && displayManager.hardware()
        && displayManager.hardware()->topologyGeneration == hardware.topologyGeneration
        && (!backend || backend->topologyEpoch() == epoch)) {
        selected = validated.normalized;
        selected.allowModeFallback = false;
        DisplayId survivor = selected.primary;
        for (const auto &physical : hardware.displays)
            if (physical.connectionType == DisplayConnectionType::Internal)
                for (const auto &request : selected.displays)
                    if (request.enabled && request.display == physical.sessionId)
                        survivor = physical.sessionId;
        selected.primary = survivor;
        selected.placements.clear();
        selected.mirrorGroups.clear();
        std::vector<std::string> disabled;
        for (auto &request : selected.displays) {
            if (request.display != survivor && request.enabled)
                disabled.push_back(request.display.value());
            request.enabled = request.display == survivor;
        }
        if (!disabled.empty()) {
            const auto checked =
                    DisplayConfigurationValidator::validateExtended(hardware, revision, selected);
            if (checked.succeeded) {
                selected = checked.normalized;
                ++testRounds;
                LM_WARNING(lingmoAurora) << "Testing single-output rescue on" << survivor.value();
                last = exact(selected, NativeDisplayDelegate::ConfigureMode::TestOnly);
                if (!displayManager.hardware()
                    || displayManager.hardware()->topologyGeneration != hardware.topologyGeneration
                    || (backend && backend->topologyEpoch() != epoch))
                    last = {false,
                            false,
                            {DisplayErrorCode::StaleConfiguration,
                             "Hardware invalidated during single-output rescue",
                             {}},
                            std::nullopt};
                if (last.succeeded)
                    plan = backend::DrmFallbackPlan {
                            {}, disabled, "Single-output rescue", testRounds};
            }
        }
    }
    if (!plan) {
        last.succeeded = false;
        last.error.message +=
                std::format("; recovery exhausted after {} candidate rounds", testRounds);
        return last;
    }
    last.selected = selected;
    last.degraded = !plan->downgradedConnectors.empty();
    last.error.code = DisplayErrorCode::None;
    last.error.affectedDisplays.clear();
    for (size_t i = 0; i < selected.displays.size(); ++i)
        if (selected.displays[i].mode.stableKey()
                    != validated.normalized.displays[i].mode.stableKey()
            || selected.displays[i].enabled != validated.normalized.displays[i].enabled)
            last.error.affectedDisplays.push_back(selected.displays[i].display);
    if (last.degraded)
        last.error.message = std::format(
                "Recovery selected a degraded configuration for {} after {} test rounds. {}",
                base::strings::join(plan->downgradedConnectors, ","), testRounds,
                base::strings::join(diagnostics, "; "));
    return last;
}

ConfigurationResult DisplayRuntime::configureExact(const DisplayConfiguration &configuration,
                                                   NativeDisplayDelegate::ConfigureMode mode)
{
    const uint64_t revision = displayManager.applied() ? displayManager.applied()->revision : 0;
    const auto result = Lingmo::Display::DisplayConfigurationValidator::validateExtended(
            *displayManager.hardware(), revision, configuration);
    if (!result.succeeded)
        return Lingmo::Display::ConfigurationResult {false, false, result.error, std::nullopt};
    if (backend) {
        std::vector<Lingmo::Backend::DrmOutput::Configuration> changes;
        bool changed = false;
        for (size_t index = 0; index < backend->allOutputs().size(); ++index) {
            auto *output = backend->allOutputs()[index];
            const Lingmo::Display::DisplayId id(outputModels.at(index)->displayId);
            for (const auto &request : configuration.displays) {
                if (request.display != id)
                    continue;
                auto selected = output->mode();
                bool found = !request.enabled;
                for (const auto &native : output->modes()) {
                    if (base::Size(native.hdisplay, native.vdisplay) == request.mode.pixelSize
                        && native.clock == request.mode.pixelClockKHz
                        && native.htotal == request.mode.horizontalTotal
                        && native.vtotal == request.mode.verticalTotal
                        && native.flags == request.mode.flags) {
                        selected = native;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return Lingmo::Display::ConfigurationResult {
                            false,
                            false,
                            {Lingmo::Display::DisplayErrorCode::UnsupportedMode,
                             "The requested native mode is unavailable",
                             {id}},
                            std::nullopt};
                const bool blanked =
                        request.enabled && isPoweredOff(id) && output->requiresSoftwareBlanking();
                const bool enabled = request.enabled && (!isPoweredOff(id) || blanked);
                changed |= output->isEnabled() != enabled || output->isBlanked() != blanked
                        || output->size() != request.mode.pixelSize
                        || output->mode().clock != selected.clock
                        || output->mode().flags != selected.flags
                        || output->mode().htotal != selected.htotal
                        || output->mode().vtotal != selected.vtotal;
                changes.push_back({output, selected, enabled, blanked});
            }
        }
        backend::DrmCommitStatus status = backend::DrmCommitStatus::Success;
        backend::DrmDeviceApi *failedDevice = nullptr;
        std::string diagnostic;
        if (changed
            && !Lingmo::Backend::DrmOutput::configure(
                    changes,
                    mode == Lingmo::Display::NativeDisplayDelegate::ConfigureMode::TestOnly,
                    &status, &diagnostic, &failedDevice)) {
            std::vector<DisplayId> affected;
            const auto outputs = backend->allOutputs();
            for (size_t i = 0; i < outputs.size(); ++i)
                if (outputs[i]->backend()->device() == failedDevice)
                    affected.push_back(DisplayId(outputModels[i]->displayId));
            if (status == backend::DrmCommitStatus::RollbackFailed)
                requireNativeRecovery();
            return Lingmo::Display::ConfigurationResult {
                    false,
                    false,
                    {status == backend::DrmCommitStatus::RollbackFailed
                             ? DisplayErrorCode::RevertFailed
                             : status == backend::DrmCommitStatus::TestFailed
                                     || status == backend::DrmCommitStatus::TransferUnavailable
                             ? DisplayErrorCode::TestFailed
                             : status == backend::DrmCommitStatus::Invalidated
                             ? DisplayErrorCode::StaleConfiguration
                             : status == backend::DrmCommitStatus::PreparationFailed
                             ? DisplayErrorCode::ResourceExhausted
                             : DisplayErrorCode::CommitFailed,
                     diagnostic, affected},
                    std::nullopt};
        }
    }
    return Lingmo::Display::ConfigurationResult {true, false, {}, std::nullopt};
}

ConfigurationResult DisplayRuntime::setPower(const std::vector<DisplayId> &displays,
                                             NativeDisplayDelegate::PowerState state)
{
    if (!displayManager.applied())
        return {false,
                false,
                {DisplayErrorCode::Unavailable, "No applied display state", {}},
                std::nullopt};
    auto next = powerStates;
    const auto nextState = [&next](const DisplayId &id) {
        const auto found = next.find(id);
        return found == next.end() ? NativeDisplayDelegate::PowerState::On : found->second;
    };
    for (const auto &id : displays) {
        bool known = false;
        for (const auto &physical : displayManager.applied()->physicalDisplays)
            known |= physical.sessionId == id;
        if (!known)
            return {false,
                    false,
                    {DisplayErrorCode::UnknownOutput, "Unknown power target", {id}},
                    std::nullopt};
    }
    for (const auto &physical : displayManager.applied()->physicalDisplays) {
        if (displays.empty() || std::ranges::find(displays, physical.sessionId) != displays.end())
            next.insert_or_assign(physical.sessionId, state);
    }
    bool hasActiveOutput = false;
    bool allOff = true;
    for (const auto &logical : displayManager.applied()->logicalDisplays) {
        for (const auto &id : logical.destinations) {
            hasActiveOutput = true;
            allOff &= nextState(id) != NativeDisplayDelegate::PowerState::On;
        }
    }
    // Install the shield and secure KMS policy before powering off the last display.
    if (state != NativeDisplayDelegate::PowerState::On && hasActiveOutput && allOff
        && lockRequested)
        lockRequested();
    if (backend) {
        std::vector<backend::DrmOutput::Configuration> changes;
        for (size_t index = 0; index < backend->allOutputs().size(); ++index) {
            const DisplayId id(outputModels.at(index)->displayId);
            auto *output = backend->allOutputs()[index];
            bool active = false;
            for (const auto &logical : displayManager.applied()->logicalDisplays)
                active |= logical.hasDestination(id);
            auto selected = output->mode();
            for (const auto &physical : displayManager.applied()->physicalDisplays) {
                if (physical.sessionId != id || !physical.currentMode)
                    continue;
                bool found = false;
                for (const auto &native : output->modes()) {
                    const auto &mode = *physical.currentMode;
                    if (base::Size(native.hdisplay, native.vdisplay) == mode.pixelSize
                        && native.clock == mode.pixelClockKHz
                        && native.htotal == mode.horizontalTotal
                        && native.vtotal == mode.verticalTotal && native.flags == mode.flags) {
                        selected = native;
                        found = true;
                        break;
                    }
                }
                if (active && !found)
                    return {false,
                            false,
                            {DisplayErrorCode::UnsupportedMode,
                             "The applied native mode is unavailable",
                             {id}},
                            std::nullopt};
            }
            const bool off = nextState(id) != NativeDisplayDelegate::PowerState::On;
            // Virtual scanouts need a black framebuffer instead of a detached CRTC.
            const bool blanked = active && off && output->requiresSoftwareBlanking();
            changes.push_back({output, selected, active && (!off || blanked), blanked});
        }
        backend::DrmCommitStatus status;
        std::string diagnostic;
        if (!backend::DrmOutput::configure(changes, false, &status, &diagnostic)) {
            if (status == backend::DrmCommitStatus::RollbackFailed)
                requireNativeRecovery();
            return {false,
                    false,
                    {status == backend::DrmCommitStatus::RollbackFailed
                             ? DisplayErrorCode::RevertFailed
                             : DisplayErrorCode::CommitFailed,
                     diagnostic,
                     {}},
                    std::nullopt};
        }
    }
    for (const auto &physical : displayManager.applied()->physicalDisplays) {
        if (!displays.empty() && std::ranges::find(displays, physical.sessionId) == displays.end())
            continue;
        if (state == NativeDisplayDelegate::PowerState::On)
            wakeOnInput.erase(physical.sessionId);
        // Explicit per-display power-off also serves the closed-lid policy.
        else if (displays.empty() && !isPoweredOff(physical.sessionId))
            wakeOnInput.insert(physical.sessionId);
    }
    powerStates = std::move(next);
    // With every display off nothing draws, so closing windows are done now
    // rather than kept, snapshot and all, until a display comes back.
    if (std::all_of(displayManager.applied()->physicalDisplays.begin(),
                    displayManager.applied()->physicalDisplays.end(),
                    [this](const auto &physical) { return isPoweredOff(physical.sessionId); }))
        scene.retireClosing();
    if (backend && secureScanout && securelyPresented) {
        for (size_t index = 0; index < backend->allOutputs().size(); ++index) {
            if (backend->allOutputs()[index]->lastPresentedFrameWasSecure())
                securelyPresented(outputModels.at(index)->displayId);
        }
    }

    if (cursorRouter)
        cursorRouter->refresh();
    return {true, false, {}, displayManager.applied()};
}

void DisplayRuntime::requireNativeRecovery()
{
    displayManager.clearApplied(displayManager.hardware()->topologyGeneration);
    if (backend)
        backend->invalidateOutputsLater();
}

void DisplayRuntime::publish()
{
    if (!displayManager.applied()) {
        // Unpublished, not gone: the connectors are still there, so their
        // wl_output globals stay announced with their last metrics and are
        // brought up to date by the next publication. replaceOutputs()
        // withdraws the ones a reprobe finds missing.
        outputGlobal = nullptr;
        compositor.setOutputViews({});
        inputManager.setTopology({});
        windowManager.setOutputBounds({});
        scene.setSize({0, 0});
        cursor.setSink(nullptr);
        if (cursorRouter)
            cursorRouter->setTargets({});
        return;
    }
    captureOutgoingFrames();
    startStartupCover();
    updatingOutput = true;
    if (maximumScaleChanged) {
        double scale = 0;
        for (const auto &logical : displayManager.applied()->logicalDisplays)
            scale = std::max(scale, logical.scale.value());
        if (scale > 0)
            maximumScaleChanged(scale);
    }
    const auto &models = outputModels;
    // The topology coordinator's empty baseline, between a reprobe and the
    // configuration it reconciles: no display is disabled by it, so no global
    // is withdrawn for it.
    const bool baseline = displayManager.applied()->logicalDisplays.empty();
    std::vector<Lingmo::Compositor::Compositor::OutputView> views;
    std::vector<base::Rect> topology;
    std::set<std::string> logicalViews;
    base::Rect desktop;
    for (int index = 0; index < models.size(); ++index) {
        auto *model = models.at(index);
        const Lingmo::Display::DisplayId destination(models.at(index)->displayId);
        for (const auto &physical : displayManager.applied()->physicalDisplays) {
            if (physical.sessionId == destination && physical.currentMode) {
                model->pixelSize = physical.currentMode->pixelSize;
                model->refreshMilliHz = physical.currentMode->refreshMilliHz;
                model->preferred = physical.currentMode->preferred;
            }
        }
        const Lingmo::Display::LogicalDisplay *logical = nullptr;
        for (const auto &candidate : displayManager.applied()->logicalDisplays) {
            if (candidate.hasDestination(destination)) {
                logical = &candidate;
                break;
            }
        }
        const bool published = logical && logical->destinations.front() == destination;
        if (backend && index < int(backend->allOutputs().size()))
            backend->allOutputs()[index]->setDirectScanoutEnabled(
                    published && logical->primary
                    && displayManager.applied()->logicalDisplays.size() == 1
                    && logical->destinations.size() == 1);
        if (!published && !baseline && index < int(outputGlobals.size()) && outputGlobals[index]) {
            if (outputGlobal == outputGlobals[index])
                outputGlobal = nullptr;
            Lingmo::Wayland::removeOutputGlobal(outputGlobals[index]);
            outputGlobals[index] = nullptr;
        }
        if (!logical)
            continue;
        model->logicalExtent = logical->bounds.size();
        model->position = logical->bounds.topLeft();
        model->scale = logical->scale;
        model->transform = static_cast<Lingmo::Compositor::Output::Transform>(logical->rotation);
        const auto mapping = displayManager.mappingFor(destination);
        assert(mapping);
        model->renderScale = mapping->renderScale;
        model->nativeContentRect = mapping->nativeContent;
        const base::Rect bounds = logical->bounds;
        if (logicalViews.insert(logical->id.value()).second) {
            views.push_back({logical->id.value(), logical->bounds, logical->scale, model->name});
            topology.push_back(bounds);
        }
        desktop = desktop.united(bounds);
        if (published && index < int(outputGlobals.size()) && !outputGlobals[index])
            outputGlobals[index] = Lingmo::Wayland::createOutputGlobal(waylandDisplay, model);
        if (published && index < int(outputGlobals.size()))
            Lingmo::Wayland::updateOutputGlobal(outputGlobals[index], model);
    }
    for (auto *global : outputGlobals) {
        if (global) {
            outputGlobal = global;
            break;
        }
    }
    compositor.setOutputViews(std::move(views));
    base::Rect primary;
    for (const auto &logical : displayManager.applied()->logicalDisplays) {
        if (logical.primary)
            primary = logical.bounds;
    }
    inputManager.setTopology(topology, primary);
    windowManager.setOutputBounds(desktop);
    scene.setSize(desktop.size());
    if (screenTransition)
        screenTransition->setGeometry(base::Rect(desktop.size()));
    if (startupCover)
        startupCover->setGeometry(base::Rect(desktop.size()));
    if (cursorRouter && backend) {
        std::vector<Lingmo::Cursor::OutputCursorRouter::Target> targets;
        for (int index = 0; index < models.size() && index < int(backend->allOutputs().size());
             ++index) {
            const Lingmo::Display::DisplayId destination(models.at(index)->displayId);
            const Lingmo::Display::LogicalDisplay *logical = nullptr;
            for (const auto &candidate : displayManager.applied()->logicalDisplays) {
                if (candidate.hasDestination(destination)) {
                    logical = &candidate;
                    break;
                }
            }
            if (!logical)
                continue;
            targets.push_back({logical->bounds, backend->allOutputs()[index],
                               displayManager.mappingFor(destination)->transform()});
        }
        cursorRouter->setTargets(std::move(targets));
        const auto *primary = displayManager.primaryDisplay();
        bool uniform = primary != nullptr;
        for (const auto &logical : displayManager.applied()->logicalDisplays) {
            uniform &= primary && logical.scale == primary->scale
                    && logical.rotation == Lingmo::Display::DisplayRotation::Normal
                    && logical.destinations.size() == 1;
        }
        cursor.setSink(uniform ? cursorRouter : nullptr);
        if (primary)
            cursor.setScale(primary->scale.value());
    }
    updatingOutput = false;
    for (auto *global : outputGlobals)
        Lingmo::Wayland::updateOutputMembership(global);
}
} // namespace Lingmo::Display
