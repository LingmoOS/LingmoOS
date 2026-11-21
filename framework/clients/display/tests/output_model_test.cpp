// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "lingmo/display/output_model.h"

using namespace Lingmo::Display;

// Displays side by side from x = 0, each its own logical display, display 0 primary.
static DisplayClient::State extendedDisplays(int count)
{
    DisplayClient::State state;
    for (int i = 0; i < count; ++i) {
        PhysicalDisplay physical;
        physical.sessionId = std::to_string(i);
        Mode mode;
        mode.width = 1920;
        mode.height = 1080;
        mode.refreshMilliHz = 60000;
        mode.current = true;
        physical.modes.push_back(mode);
        state.physical.push_back(physical);
        LogicalDisplay logical;
        logical.id = physical.sessionId;
        logical.destinations = {physical.sessionId};
        logical.primary = i == 0;
        logical.scale120 = 120;
        logical.x = i * 1920;
        state.logical.push_back(logical);
    }
    return state;
}

TEST(OutputModelTest, StateRefreshKeepsRowsAndLayout)
{
    DisplayClient::State state;
    for (int i = 0; i < 3; ++i) {
        PhysicalDisplay physical;
        physical.sessionId = std::to_string(i);
        Mode mode;
        mode.width = 1920;
        mode.height = 1080;
        mode.refreshMilliHz = 60000;
        mode.current = true;
        physical.modes.push_back(mode);
        state.physical.push_back(physical);
        LogicalDisplay logical;
        logical.id = physical.sessionId;
        logical.destinations = {physical.sessionId};
        logical.primary = i == 0;
        logical.scale120 = 120;
        logical.x = i == 1 ? -1920 : 0;
        logical.y = i == 2 ? 1080 : 0;
        state.logical.push_back(logical);
    }
    state.placements = {{"1", "0", 3, 0}, {"2", "0", 2, 0}};
    OutputModel model;
    model.setState(state);
    int rowsChanged = 0;
    const auto subscription = model.rowsChanged.connect([&rowsChanged] { ++rowsChanged; });
    ASSERT_TRUE(model.setScale(1, 1.25));
    const auto placements = model.placements();
    ASSERT_EQ(placements.size(), 2u);
    EXPECT_EQ(placements[1].parentId, "0");
    EXPECT_EQ(placements[1].edge, 2u);
    std::reverse(state.physical.begin(), state.physical.end());
    model.setState(state);
    EXPECT_EQ(rowsChanged, 0);
    EXPECT_EQ(model.id(1), "1");
}

TEST(OutputModelTest, OnlyModeChangesNeedConfirmation)
{
    DisplayClient::State state = extendedDisplays(2);
    state.physical[0].connectionType = 1;
    for (PhysicalDisplay& physical : state.physical) {
        Mode mode;
        mode.width = 1280;
        mode.height = 720;
        mode.refreshMilliHz = 60000;
        physical.modes.push_back(mode);
    }
    OutputModel model;
    model.setState(state);
    EXPECT_FALSE(model.needsConfirmation());

    ASSERT_TRUE(model.setScale(1, 1.25));
    EXPECT_TRUE(model.dirty());
    EXPECT_FALSE(model.needsConfirmation());

    // The internal panel is no exception.
    ASSERT_TRUE(model.setResolutionIndex(0, 1));
    EXPECT_TRUE(model.needsConfirmation());
    ASSERT_TRUE(model.setResolutionIndex(0, 0));
    EXPECT_FALSE(model.needsConfirmation());

    ASSERT_TRUE(model.setResolutionIndex(1, 1));
    EXPECT_TRUE(model.needsConfirmation());

    ASSERT_TRUE(model.setEnabled(1, false));
    EXPECT_FALSE(model.needsConfirmation());

    model.discard();
    EXPECT_FALSE(model.needsConfirmation());

    // Switching a display on picks a mode without one to fall back to.
    for (Mode& mode : state.physical[1].modes)
        mode.current = false;
    state.logical.pop_back();
    model.setState(state);
    ASSERT_TRUE(model.setEnabled(1, true));
    EXPECT_TRUE(model.dirty());
    EXPECT_FALSE(model.needsConfirmation());
}

TEST(OutputModelTest, MirroringThePrimaryHandsItToTheSource)
{
    OutputModel model;
    model.setState(extendedDisplays(3));
    // Screen applies on dirtyChanged, so the handoff has to come before it.
    std::string applied;
    const auto subscription = model.dirtyChanged.connect([&] { applied = model.primary(); });
    ASSERT_TRUE(model.setReplicationSourceIndex(0, 1));
    EXPECT_EQ(applied, "1");
    EXPECT_EQ(model.primary(), "1");
    EXPECT_FALSE(model.isPrimary(0));
    EXPECT_TRUE(model.isPrimary(1));
    const auto placements = model.placements();
    ASSERT_EQ(placements.size(), 1u);
    EXPECT_EQ(placements[0].displayId, "2");
    EXPECT_EQ(placements[0].parentId, "1");
}

TEST(OutputModelTest, MirroringThePrimaryNeedsAnExtendedSource)
{
    // Display 1 is off in the first state and mirrors display 2 in the second.
    DisplayClient::State off = extendedDisplays(3);
    off.logical.erase(off.logical.begin() + 1);
    DisplayClient::State mirroring = off;
    mirroring.logical[1].destinations.push_back("1");
    for (const DisplayClient::State& state : {off, mirroring}) {
        OutputModel model;
        model.setState(state);
        ASSERT_TRUE(model.setReplicationSourceIndex(0, 1));
        EXPECT_EQ(model.primary(), "0");
        EXPECT_TRUE(model.isPrimary(0));
    }
}

TEST(OutputModelTest, SwitchingOffAMirrorSourceKeepsItsMirrorsOn)
{
    // Display 1 mirrors the primary display 0.
    DisplayClient::State state = extendedDisplays(2);
    state.logical.pop_back();
    state.logical[0].destinations.push_back("1");
    OutputModel model;
    model.setState(state);
    ASSERT_EQ(model.replicationSourceIndex(1), 1);

    ASSERT_TRUE(model.setEnabled(0, false));
    EXPECT_TRUE(model.isEnabled(1));
    EXPECT_EQ(model.replicationSourceIndex(1), 0);
    EXPECT_EQ(model.primary(), "1");
    EXPECT_TRUE(model.mirrorGroups().empty());
}

TEST(OutputModelTest, MirroringAMirrorSourceMovesItsMirrors)
{
    // Display 1 mirrors the primary display 0; display 2 extends.
    DisplayClient::State state = extendedDisplays(3);
    state.logical.erase(state.logical.begin() + 1);
    state.logical[0].destinations.push_back("1");
    {
        OutputModel model;
        model.setState(state);
        ASSERT_TRUE(model.setReplicationSourceIndex(0, 2));
        EXPECT_EQ(model.mirrorGroups(), (MirrorGroups{{"2", "0", "1"}}));
        EXPECT_EQ(model.primary(), "2");
    }
    {
        // Display 0 mirrors display 1 instead, which then mirrors nothing.
        OutputModel model;
        model.setState(state);
        ASSERT_TRUE(model.setReplicationSourceIndex(0, 1));
        EXPECT_EQ(model.replicationSourceIndex(1), 0);
        EXPECT_EQ(model.mirrorGroups(), (MirrorGroups{{"1", "0"}}));
        EXPECT_EQ(model.primary(), "1");
    }
}

TEST(OutputModelTest, SelectedResolutionAndRefreshStayConsistent)
{
    DisplayClient::State state;
    PhysicalDisplay physical;
    physical.sessionId = "display-1";
    Mode mode;
    mode.width = 1920;
    mode.height = 1080;
    mode.refreshMilliHz = 60000;
    mode.current = true;
    physical.modes.push_back(mode);
    mode.current = false;
    mode.refreshMilliHz = 144000;
    physical.modes.push_back(mode);
    mode.width = 1280;
    mode.height = 720;
    mode.refreshMilliHz = 60000;
    physical.modes.push_back(mode);
    state.physical.push_back(physical);
    OutputModel model;
    model.setState(state);
    int changes = 0;
    const auto subscription = model.changed.connect([&changes] { ++changes; });
    ASSERT_TRUE(model.setRefreshRateIndex(0, 1));
    EXPECT_EQ(model.requests().front().mode.refreshMilliHz, 144000);
    EXPECT_EQ(model.refreshRates(0), (std::vector<std::string>{"60.00 Hz", "144.00 Hz"}));
    ASSERT_TRUE(model.setResolutionIndex(0, 1));
    EXPECT_EQ(model.requests().front().mode.width, 1280);
    EXPECT_EQ(model.refreshRateIndex(0), 0);
    EXPECT_EQ(model.refreshRates(0).size(), 1u);
    EXPECT_EQ(model.resolutions(0), (std::vector<std::string>{"1920 × 1080", "1280 × 720"}));
    EXPECT_EQ(changes, 2);
}

TEST(OutputModelTest, ExternalStateDoesNotBecomeAnEdit)
{
    OutputModel model;
    bool edit = false;
    const auto subscription = model.dirtyChanged.connect([&] { edit |= model.dirty(); });
    model.setState({});
    EXPECT_FALSE(edit);
}

TEST(OutputModelTest, RecommendationsFollowTheCompositor)
{
    DisplayClient::State state;
    PhysicalDisplay physical;
    physical.sessionId = "0";
    physical.recommendedScale120 = 180;
    for (const auto& [width, height] : {std::pair{1920, 1080}, std::pair{3840, 2160}, std::pair{2560, 1440}}) {
        Mode mode;
        mode.width = width;
        mode.height = height;
        mode.refreshMilliHz = 60000;
        mode.preferred = width == 3840;
        mode.current = width == 1920;
        physical.modes.push_back(mode);
    }
    state.physical.push_back(physical);
    LogicalDisplay logical;
    logical.id = physical.sessionId;
    logical.destinations = {physical.sessionId};
    state.logical.push_back(logical);

    OutputModel model;
    model.setState(state);
    // Largest first: 3840 × 2160 heads the list.
    EXPECT_EQ(model.recommendedResolutionIndex(0), 0);
    EXPECT_EQ(model.recommendedScale(0), 1.5);
}

TEST(OutputModelTest, ScreensAreNamedForPeople)
{
    DisplayClient::State state;
    const auto add = [&state](const std::string& connector, const std::string& monitor, uint32_t type) {
        PhysicalDisplay physical;
        physical.sessionId = connector;
        physical.connectorName = connector;
        physical.model = monitor;
        physical.connectionType = type;
        state.physical.push_back(physical);
        LogicalDisplay logical;
        logical.id = connector;
        logical.destinations = {connector};
        state.logical.push_back(logical);
    };
    add("eDP-1", "eDP-1", 1);
    add("DP-1", "DELL U2720Q", 2);
    add("DP-2", "DELL U2720Q", 2);
    add("HDMI-A-1", "HDMI-A-1", 2);

    OutputModel model;
    model.setState(state);
    std::vector<std::string> names;
    for (int row = 0; row < model.count(); ++row)
        names.push_back(model.name(row));
    EXPECT_EQ(names, (std::vector<std::string>{"Built-in Display", "DELL U2720Q (DP-1)", "DELL U2720Q (DP-2)",
                                               "HDMI-A-1"}));
    EXPECT_EQ(model.replicationSources(3), (std::vector<std::string>{"Extend", "Built-in Display",
                                                                      "DELL U2720Q (DP-1)", "DELL U2720Q (DP-2)"}));
}

TEST(OutputModelTest, RotationsMapBothWays)
{
    OutputModel model;
    model.setState(extendedDisplays(1));
    EXPECT_EQ(model.rotation(0), OutputModel::None);
    ASSERT_TRUE(model.setRotation(0, OutputModel::Right));
    EXPECT_EQ(model.rotation(0), OutputModel::Right);
    EXPECT_EQ(model.requests().front().rotation, 3u);
    EXPECT_FALSE(model.setRotation(0, 3));
    EXPECT_FALSE(model.setScale(0, 0.5));
}
