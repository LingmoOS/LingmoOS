// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountservice.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "base/paths.h"
#include "base/session_language.h"

#include <libmtp.h>
#include <libssh/libssh.h>

#include <csignal>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/file.h>
#include <unistd.h>

using namespace Lingmo::base;

static int s_quitFd = -1;

static void requestQuit(int)
{
    const uint64_t one = 1;
    (void)!::write(s_quitFd, &one, sizeof(one));
}

int main()
{
    Locale::setProcessLocale();
    Lingmo::i18n::setDomain("lingmo-filemanager");
    // One instance per user: a second one, started on a private bus by a test, would
    // take the first one's mounts for leftovers and detach them.
    const std::string lockPath = paths::runtimeDirectory() + "/lingmo-mountd.lock";
    const int lock = ::open(lockPath.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0600);
    if (lock < 0 || ::flock(lock, LOCK_EX | LOCK_NB) < 0)
        return 1;

    ssh_init();
    LIBMTP_Init();

    EventLoop loop;
    dbus::Connection *bus = dbus::Connection::session(&loop);
    if (!bus)
        return 1;

    SessionLanguage language(bus);
    MountService service(bus);
    if (!service.publish() || !bus->requestName("org.lingmo.Mounts1"))
        return 1;

    // The session's end detaches every mount, so none is left answering nothing.
    s_quitFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    loop.watch(s_quitFd, EPOLLIN, [&loop](uint32_t) {
        loop.quit();
    });
    std::signal(SIGTERM, requestQuit);
    std::signal(SIGINT, requestQuit);
    std::signal(SIGHUP, requestQuit);

    const int result = loop.run();
    service.detachAll();
    ssh_finalize();
    // Sessions still serving open files end on their own; nothing waits for them.
    ::_exit(result);
}
