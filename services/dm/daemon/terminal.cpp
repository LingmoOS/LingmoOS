#include "terminal.h"

#include "base/dbus.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/strings.h"

#include <fcntl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace Lingmo;

namespace dm {

static int openVt(int vt)
{
    return ::open(("/dev/tty" + std::to_string(vt)).c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC);
}

bool setGraphicsMode(int vt)
{
    const int fd = openVt(vt);
    if (fd < 0)
        return false;

    const bool set = ioctl(fd, KDSETMODE, KD_GRAPHICS) == 0;
    ::close(fd);
    return set;
}

void restoreTextMode(int vt)
{
    const int fd = openVt(vt);
    if (fd < 0)
        return;

    vt_mode mode {};
    int kdMode = KD_TEXT;
    if (ioctl(fd, VT_GETMODE, &mode) == 0 && mode.mode == VT_AUTO
        && ioctl(fd, KDGETMODE, &kdMode) == 0 && kdMode == KD_GRAPHICS)
        ioctl(fd, KDSETMODE, KD_TEXT);

    ::close(fd);
}

bool isControlled(int vt)
{
    const int fd = openVt(vt);
    if (fd < 0)
        return false;

    vt_mode mode {};
    const bool controlled = ioctl(fd, VT_GETMODE, &mode) == 0 && mode.mode == VT_PROCESS;
    ::close(fd);
    return controlled;
}

int activeVt()
{
    const std::optional<std::string> active = base::file::read("/sys/class/tty/tty0/active");
    if (!active)
        return 0;

    const std::string_view name = base::strings::trimmed(*active);
    return name.size() > 3 ? base::strings::toNumber<int>(name.substr(3)).value_or(0) : 0;
}

bool activateSession(const std::string &sessionId)
{
    const base::dbus::Proxy manager(base::dbus::Connection::system(), "org.freedesktop.login1",
                                    "/org/freedesktop/login1", "org.freedesktop.login1.Manager");
    const base::dbus::Reply reply = manager.call("ActivateSession", sessionId);
    if (reply.isError())
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot activate session" << sessionId << reply.error().message;

    return !reply.isError();
}

} // namespace dm
