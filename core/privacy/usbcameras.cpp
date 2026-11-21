// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbcameras.h"

#include "base/file.h"
#include "base/logging.h"
#include "base/strings.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>
#include <vector>

static const Lingmo::Base::LogCategory kLog("lingmo.privacy");

static constexpr char kVideoClass[] = "0e";

static std::string readAttribute(const std::string &path)
{
    const auto contents = Lingmo::Base::file::read(path);
    return contents ? std::string(Lingmo::Base::strings::trimmed(*contents)) : std::string();
}

static bool writeAttribute(const std::string &path, const std::string &value)
{
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    const bool written = fd >= 0 && ::write(fd, value.data(), value.size()) == ssize_t(value.size());
    const int error = errno;
    if (fd >= 0)
        ::close(fd);
    if (!written) {
        LM_WARNING(kLog) << "lingmo-privacy: cannot write" << path << std::strerror(error);
        return false;
    }

    return true;
}

UsbCameras::UsbCameras(const std::string &sysfs)
    : m_sysfs(sysfs)
{
}

void UsbCameras::setAuthorized(bool authorized)
{
    const std::string devices = m_sysfs + "/bus/usb/devices";
    const std::string value = authorized ? "1" : "0";
    std::vector<std::string> names;
    std::vector<std::string> probe;

    std::error_code error;
    for (const auto &item : std::filesystem::directory_iterator(devices, error))
        names.push_back(item.path().filename().string());
    std::sort(names.begin(), names.end());

    // Interfaces are named bus-port:configuration.interface; sorted, the video
    // control interface comes before the streaming ones its driver claims.
    for (const std::string &name : names) {
        if (name.find(':') == std::string::npos)
            continue;

        const std::string path = devices + '/' + name;
        if (readAttribute(path + "/bInterfaceClass") != kVideoClass)
            continue;

        if (readAttribute(path + "/authorized") == value)
            continue;

        if (writeAttribute(path + "/authorized", value) && authorized)
            probe.push_back(name);
    }

    // Authorizing an interface does not bind a driver to it again.
    for (const std::string &name : probe)
        writeAttribute(m_sysfs + "/bus/usb/drivers_probe", name);
}
