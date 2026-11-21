// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "base/file.h"
#include "display/display_preferences.h"

using namespace Lingmo;

namespace {

class DisplayPreferencesTest : public ::testing::Test
{
protected:
    std::string path() const { return m_directory.path() + "/display.json"; }

private:
    base::file::TemporaryDirectory m_directory {std::filesystem::temp_directory_path().string()};
};

display::HardwareSnapshot hardware()
{
    display::DisplaySnapshot display;
    display.sessionId = display::DisplayId("output-a");
    display.persistentKey = "edid-a";
    display.portKey = "port-a";
    display.connectorName = "eDP-1";
    display.devicePath = "/dev/dri/card0";
    display.physicalSizeMm = base::Size(300, 190);
    display.modes = {{base::Size(1920, 1080), 60000, 148500, 2020, 1130, 0, "mode", true}};
    display.currentMode = display.modes.front();
    return {"instance", "seat0", 1, {display}};
}

display::DisplayConfiguration configuration(const display::HardwareSnapshot &hardware)
{
    display::DisplayConfiguration result;
    result.base = {hardware.compositorInstanceId, hardware.topologyGeneration, 1};
    result.primary = display::DisplayId("output-a");
    result.displays = {{result.primary, true, hardware.displays.front().modes.front(),
                        *compositor::OutputScale::fromNumerator(150),
                        display::DisplayRotation::Normal}};
    return result;
}

} // namespace

// display.json is shared with every earlier build: keys sorted at every level,
// four-space indentation, empty containers closed on their own line.
TEST_F(DisplayPreferencesTest, SavedFileKeepsItsFormat)
{
    const auto snapshot = hardware();
    ASSERT_TRUE(display::DisplayPreferences(path()).save(snapshot, configuration(snapshot)));
    EXPECT_EQ(base::file::read(path()).value_or(""),
              R"({
    "monitors": {
        "edid-a": {
            "mode": {
                "flags": 0,
                "height": 1080,
                "horizontalTotal": 2020,
                "name": "mode",
                "pixelClockKHz": 148500,
                "refreshMilliHz": 60000,
                "verticalTotal": 1130,
                "width": 1920
            },
            "preferredPort": "port-a",
            "rotation": 0,
            "scale120": 150
        }
    },
    "topologies": {
        "edid-a": {
            "enabled": [
                "edid-a"
            ],
            "mirrorGroups": [
            ],
            "placements": [
            ],
            "primary": "edid-a"
        }
    },
    "version": 2
}
)");
}

TEST_F(DisplayPreferencesTest, RestoresWhatItSaved)
{
    const auto snapshot = hardware();
    display::DisplayPreferences preferences(path());
    ASSERT_TRUE(preferences.save(snapshot, configuration(snapshot)));
    const auto restored = preferences.restore(snapshot, 4);
    ASSERT_TRUE(restored);
    ASSERT_EQ(restored->displays.size(), 1u);
    EXPECT_EQ(restored->displays.front().scale.numerator(), 150u);
    EXPECT_EQ(restored->primary, display::DisplayId("output-a"));
    EXPECT_EQ(restored->base.appliedRevision, 4u);
    EXPECT_EQ(preferences.persistentKeys(), std::vector<std::string> {"edid-a"});
}

// A number that is not a whole int reads as missing, as it always has: the
// scale falls back to the panel's recommended one.
TEST_F(DisplayPreferencesTest, FractionalNumbersFallBack)
{
    ASSERT_TRUE(base::file::writeAtomically(path(), R"({"version": 2.0,
        "monitors": {"edid-a": {"scale120": 150.5, "rotation": 1}},
        "topologies": {"edid-a": {"enabled": ["edid-a"], "primary": "edid-a"}}})"));
    const auto restored = display::DisplayPreferences(path()).restore(hardware(), 1);
    ASSERT_TRUE(restored);
    ASSERT_EQ(restored->displays.size(), 1u);
    EXPECT_EQ(restored->displays.front().scale.numerator(), 180u);
    EXPECT_EQ(restored->displays.front().rotation, display::DisplayRotation::Rotate90);
}
