#include <csignal>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "base/command_line.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "base/logging.h"

#include "config.h"
#include "daemon.h"

using namespace Lingmo;

int main(int argc, char *argv[])
{
    base::Locale::setProcessLocale();
    i18n::setDomain("lingmo-dm");

    // Through signalfd, so SIGTERM runs the daemon's teardown.
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    const int signalFd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);

    base::EventLoop loop;

    base::CommandLineParser parser;
    parser.addHelpOption();
    const base::CommandLineOption configOption("config", "Configuration file to read.", "path",
                                               "/etc/lingmo-dm.conf");
    parser.addOption(configOption);
    parser.process(argc, argv);

    if (geteuid() != 0) {
        LM_CRITICAL(base::LogCategory::defaultCategory()) << "lingmo-dm has to run as root";
        return 1;
    }

    dm::Daemon daemon(dm::Config::load(parser.value(configOption)));

    if (signalFd >= 0) {
        loop.watch(signalFd, EPOLLIN, [signalFd, &loop](uint32_t) {
            signalfd_siginfo info;
            while (read(signalFd, &info, sizeof(info)) == sizeof(info)) {
            }
            loop.quit();
        });
    }

    if (!daemon.start())
        return 1;

    return loop.run();
}
