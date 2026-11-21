#include "filechooserportal.h"
#include "inhibitportal.h"
#include "screencastportal.h"
#include "settingsportal.h"

#include "base/command_line.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/logging.h"
#include "base/paths.h"

#include <csignal>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace Lingmo;

static int g_signalFds[2] = {-1, -1};

static void onSignal(int)
{
    const char byte = 0;
    (void)!write(g_signalFds[0], &byte, 1);
}

// A termination signal ends the event loop instead of the process, so the
// portals and the leak checker of a sanitizer build get their shutdown.
static void quitOnSignal(base::EventLoop &loop)
{
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, g_signalFds) != 0)
        return;
    loop.watch(g_signalFds[1], EPOLLIN, [&loop](uint32_t) {
        char byte;
        (void)!read(g_signalFds[1], &byte, 1);
        loop.quit();
    });
    struct sigaction action = {};
    action.sa_handler = onSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);
}

int main(int argc, char *argv[])
{
    base::EventLoop loop;
    quitOnSignal(loop);

    base::CommandLineParser parser;
    parser.setApplicationDescription("Lingmo desktop portal backend");
    parser.setApplicationVersion(PROJECT_VERSION);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(argc, argv);

    base::dbus::Connection *bus = base::dbus::Connection::session(&loop);
    if (!bus) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "Cannot connect to the session bus";
        return 1;
    }

    base::dbus::MatchRule disconnected;
    disconnected.path = "/org/freedesktop/DBus/Local";
    disconnected.interface = "org.freedesktop.DBus.Local";
    disconnected.member = "Disconnected";
    const base::dbus::Match quitOnDisconnect =
            bus->subscribe(disconnected, [&loop](const base::dbus::Message &) { loop.quit(); });

    SettingsPortal settings(bus, base::paths::lingmoConfig("services"));
    InhibitPortal inhibit(bus);
    FileChooserPortal fileChooser(bus);
    ScreenCastPortal screenCast(bus);
    if (!settings.publish() || !inhibit.publish() || !fileChooser.publish() || !screenCast.publish()) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "Cannot export the portals";
        return 1;
    }

    if (!bus->requestName("org.freedesktop.impl.portal.desktop.lingmo")) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "Cannot own the Lingmo portal service";
        return 1;
    }

    return loop.run();
}
