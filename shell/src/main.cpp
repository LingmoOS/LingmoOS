// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <csignal>
#include <memory>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "core/compositor_options.h"
#include "core/compositor_shell.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "core/malloc_policy.h"
#include "core/scoped_fd.h"
#include "scene/motion.h"

namespace {

// Blocks SIGINT/SIGTERM/SIGHUP and returns a signalfd for them, so shutdown runs
// on the event loop rather than in a signal handler. SIGHUP is what the session's
// terminal sends when its leader exits; dying of it would skip the teardown that
// leaves the last frame on screen for the next DRM master.
int createSignalFd()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &mask, nullptr) != 0)
        return -1;

    return signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
}

} // namespace

int main(int argc, char *argv[])
{
    Lingmo::Core::applyMallocPolicy();
    signal(SIGPIPE, SIG_IGN);
    // The user's locale from the start, not only once the embedded toolkit
    // sets it when the lock screen first appears.
    Lingmo::base::Locale::setProcessLocale();
    Lingmo::i18n::setDomain("lingmo-aurora");
    const Lingmo::ScopedFd signalFdOwner(createSignalFd());
    const int signalFd = signalFdOwner.get();
    // Outlives the shell, whose objects may watch, time or post on it.
    const Lingmo::Core::MainLoop mainLoop;

    const auto options = Lingmo::Core::CompositorOptions::parse(argc, argv);
    if (!options)
        return 1;
    if (!options->logRules.empty())
        Lingmo::Core::setLoggingRules(options->logRules);
    Lingmo::Scene::motion::setDurationScale(options->animationDurationScale);

    auto shell = std::make_unique<Lingmo::Core::CompositorShell>(*options);
    if (!shell->init())
        return 1;

    int signalWatch = 0;
    if (signalFd >= 0) {
        signalWatch = mainLoop.loop().watch(signalFd, EPOLLIN, [&](uint32_t) {
            signalfd_siginfo info;
            while (read(signalFd, &info, sizeof(info)) == sizeof(info))
                ;
            LM_INFO(lingmoWayland) << "Shutting down";
            shell->stop();
            mainLoop.loop().quit();
        });
    } else {
        LM_WARNING(lingmoWayland) << "Failed to create signalfd; Ctrl-C will not shut down cleanly";
    }

    LM_INFO(lingmoWayland) << "lingmo-aurora started, WAYLAND_DISPLAY=" + shell->socketName();

    if (!shell->notifyReady())
        return 1;

    const int result = mainLoop.loop().run();
    shell.reset();

    if (signalWatch)
        mainLoop.loop().unwatch(signalWatch);
    return result;
}
