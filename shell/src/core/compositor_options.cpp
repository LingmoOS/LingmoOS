// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/compositor_options.h"

#include <fcntl.h>

#include "base/command_line.h"
#include "base/strings.h"
#include "core/logging.h"

namespace Lingmo::Core {

std::optional<CompositorOptions> CompositorOptions::parse(int argc, char **argv)
{
    base::CommandLineParser parser;
    parser.setApplicationDescription("Lingmo Aurora Wayland compositor");
    parser.setApplicationVersion("1.0");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"socket", "Wayland socket name to listen on.", "name"});
    parser.addOption({"drm-device", "DRM device to drive, e.g. /dev/dri/card0.", "path"});
    parser.addOption({"no-drm", "Run the Wayland server only, without acquiring a display."});
    parser.addOption({"headless-outputs", "Number of deterministic virtual outputs in --no-drm mode.",
                      "count", "1"});
    parser.addOption({"wm-demo", "Enable development-only F1–F9 WM shortcuts."});
    parser.addOption({"wm-demo-script",
                      "Development only: drive the window manager through a timed sequence "
                      "instead of waiting for the demo keys."});
    parser.addOption({"shell-control-socket",
                      "Path of the control socket the session passes trusted shell connections "
                      "over.",
                      "path"});
    parser.addOption({"login", "Start the internal login screen for greetd session handoff."});
    parser.addOption({"ready-fd", "Session startup notification file descriptor.", "fd"});
    parser.addOption({"clipboard-client",
                      "Executable permitted to manage clipboard contents; may be repeated.", "path"});
    parser.addOption({"capture-client", "Allow this executable to capture outputs (repeatable).",
                      "path"});
    parser.addOption({"computer-control-client",
                      "Allow this executable to ask for control of the computer (repeatable).",
                      "path"});
    parser.addOption({"window-placement-client",
                      "Allow this executable to place its own windows (repeatable).", "path"});
    parser.addOption({"input-method",
                      "Development only: run this input-method desktop file id instead of the "
                      "saved selection, also without a display.",
                      "id"});
    parser.addOption({"scale", "Output scale, a decimal from 1.0 through 4.0.", "factor", "1.0"});
    parser.addOption({"display-config-service",
                      "Session D-Bus name for native display configuration.", "name",
                      "org.lingmo.DisplayConfig"});
#ifdef LINGMO_ENABLE_XWAYLAND
    parser.addOption({"xwayland", "Enable the X11 frontend (enabled by default)."});
    parser.addOption({"no-xwayland", "Disable the X11 frontend; overrides --xwayland."});
    parser.addOption({"xwayland-shm", "Use Xwayland software buffers for diagnostics."});
#endif
    parser.addOption({"xwayland-ready-file", "Publish DISPLAY/XAUTHORITY as JSON when available.",
                      "path"});
    parser.addOption({"animation-duration-scale",
                      "Stretch every animation by this factor, to judge motion slowly.", "factor",
                      "1"});
    parser.addOption({"log-rules",
                      "Logging rules, e.g. \"lingmo.aurora.debug=true;lingmo.drm=true\".",
                      "rules"});
    parser.process(argc, argv);

    CompositorOptions options;

    const auto scale = compositor::OutputScale::parse(parser.value("scale"));
    if (!scale) {
        LM_CRITICAL(base::LogCategory::defaultCategory())
                << "Invalid output scale; expected a decimal from 1.0 through 4.0";
        return std::nullopt;
    }
    options.scale = *scale;

    const auto durationScale =
            base::strings::toDouble(parser.value("animation-duration-scale"));
    if (!durationScale || !(*durationScale > 0)) {
        LM_CRITICAL(base::LogCategory::defaultCategory())
                << "Invalid --animation-duration-scale; expected a positive number";
        return std::nullopt;
    }
    options.animationDurationScale = *durationScale;

    if (parser.isSet("ready-fd")) {
        const auto fd = base::strings::toNumber<int>(parser.value("ready-fd"));
        if (!fd || *fd < 3 || fcntl(*fd, F_SETFD, FD_CLOEXEC) < 0) {
            LM_CRITICAL(base::LogCategory::defaultCategory())
                    << "Invalid session startup file descriptor";
            return std::nullopt;
        }
        options.readyFd = *fd;
    }

    options.noDrm = parser.isSet("no-drm");
    if (options.noDrm) {
        const auto count = base::strings::toNumber<int>(parser.value("headless-outputs"));
        if (!count || *count < 1 || *count > 16) {
            LM_CRITICAL(base::LogCategory::defaultCategory())
                    << "Invalid --headless-outputs count; expected 1 through 16";
            return std::nullopt;
        }
        options.headlessOutputs = *count;
    }

    options.socketName = parser.value("socket");
    options.drmDevice = parser.value("drm-device");
    options.wmDemo = parser.isSet("wm-demo");
    options.wmDemoScript = parser.isSet("wm-demo-script");
    options.shellControlSocket = parser.value("shell-control-socket");
    options.login = parser.isSet("login");
    options.clipboardClients = parser.values("clipboard-client");
    options.captureClients = parser.values("capture-client");
    options.computerControlClients = parser.values("computer-control-client");
    options.windowPlacementClients = parser.values("window-placement-client");
    options.inputMethodId = parser.value("input-method");
    options.displayConfigService = parser.value("display-config-service");
#ifdef LINGMO_ENABLE_XWAYLAND
    options.xwayland = !parser.isSet("no-xwayland");
    options.xwaylandSoftware = parser.isSet("xwayland-shm");
#else
    options.xwayland = false;
#endif
    options.xwaylandReadyFile = parser.value("xwayland-ready-file");
    options.logRules = parser.value("log-rules");

    return options;
}

std::vector<std::string> captureExecutables(const CompositorOptions &options)
{
    std::vector<std::string> executables {LINGMO_PORTAL_BACKEND};
    executables.insert(executables.end(), options.captureClients.begin(), options.captureClients.end());
    return executables;
}

} // namespace Lingmo::Core
