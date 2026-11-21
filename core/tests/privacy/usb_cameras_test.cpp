// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbcameras.h"

#include "base/file.h"
#include "base/strings.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

static void writeFile(const std::string &path, const std::string &contents)
{
    fs::create_directories(fs::path(path).parent_path());

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(file.good());
    file << contents;
}

static std::string readFile(const std::string &path)
{
    const auto contents = Lingmo::base::file::read(path);
    return contents ? std::string(Lingmo::base::strings::trimmed(*contents)) : std::string();
}

class UsbCamerasTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_sysfs.isValid());

        const std::string devices = m_sysfs.path() + "/bus/usb/devices/";
        // A camera's control and streaming interfaces, a keyboard, and the camera's device.
        writeFile(devices + "3-4:1.0/bInterfaceClass", "0e\n");
        writeFile(devices + "3-4:1.0/authorized", "1\n");
        writeFile(devices + "3-4:1.1/bInterfaceClass", "0e\n");
        writeFile(devices + "3-4:1.1/authorized", "1\n");
        writeFile(devices + "1-1:1.0/bInterfaceClass", "03\n");
        writeFile(devices + "1-1:1.0/authorized", "1\n");
        writeFile(devices + "3-4/authorized", "1\n");
        writeFile(m_sysfs.path() + "/bus/usb/drivers_probe", "");
    }

    std::string attribute(const char *name) const
    {
        return m_sysfs.path() + "/bus/usb/devices/" + name + "/authorized";
    }

    std::string probe() const { return m_sysfs.path() + "/bus/usb/drivers_probe"; }

    Lingmo::base::file::TemporaryDirectory m_sysfs {fs::temp_directory_path().string()};
};

TEST_F(UsbCamerasTest, offTouchesOnlyVideoInterfaces)
{
    UsbCameras(m_sysfs.path()).setAuthorized(false);

    EXPECT_EQ(readFile(attribute("3-4:1.0")), "0");
    EXPECT_EQ(readFile(attribute("3-4:1.1")), "0");
    EXPECT_EQ(readFile(attribute("1-1:1.0")), "1");
    EXPECT_EQ(readFile(attribute("3-4")), "1");
    EXPECT_EQ(readFile(probe()), "");
}

TEST_F(UsbCamerasTest, onAuthorizesAndProbes)
{
    UsbCameras cameras(m_sysfs.path());
    cameras.setAuthorized(false);
    cameras.setAuthorized(true);

    EXPECT_EQ(readFile(attribute("3-4:1.0")), "1");
    EXPECT_EQ(readFile(attribute("3-4:1.1")), "1");
    // The file keeps only the last write: the streaming interface, after its control one.
    EXPECT_EQ(readFile(probe()), "3-4:1.1");
}

TEST_F(UsbCamerasTest, onLeavesAuthorizedInterfacesAlone)
{
    UsbCameras(m_sysfs.path()).setAuthorized(true);

    EXPECT_EQ(readFile(probe()), "");
}
