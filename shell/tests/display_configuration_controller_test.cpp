// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/resolution_notification_controller_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests cover full-layout preview confirmation, timeout,
// owner loss, and confirmed-only JSON persistence.

#include <gtest/gtest.h>

#include <deque>
#include <filesystem>


#include "base/file.h"
#include "display/display_configuration_controller.h"
#include "fake_native_display_delegate.h"
#include "loop_test_util.h"

using namespace Lingmo;

namespace {

display::DisplayId id(const char* value)
{
    return display::DisplayId(std::string(value));
}

display::DisplayMode mode(base::Size size)
{
    return {size, 60000, 148500, size.width() + 100, size.height() + 50, 0,
            "mode", true};
}

display::DisplaySnapshot output(const char* name, base::Size size)
{
    display::DisplaySnapshot value;
    value.sessionId = id(name);
    value.persistentKey = "edid-" + std::string(name);
    value.portKey = "port-" + std::string(name);
    value.connectorName = std::string(name);
    value.devicePath = "/dev/dri/card0";
    value.physicalSizeMm = {600, 340};
    value.modes = {mode(size)};
    value.currentMode = value.modes.front();
    value.capabilities = display::DisplayCapability::Scale |
        display::DisplayCapability::Rotation | display::DisplayCapability::Power;
    return value;
}

display::HardwareSnapshot hardware()
{
    return {"instance", "seat0", 3,
            {output("internal", {1920, 1080}), output("external", {2560, 1440})}};
}

display::AppliedConfiguration applied(const display::HardwareSnapshot& state)
{
    return {state.compositorInstanceId,
            state.topologyGeneration,
            7,
            state.displays,
            {{id("internal"), {id("internal")}, "eDP-1",
              {0, 0, 1920, 1080}, {0, 0, 1920, 1080}, {},
              display::DisplayRotation::Normal, true},
             {id("external"), {id("external")}, "DP-1",
              {1920, 0, 2560, 1440}, {1920, 0, 2560, 1440}, {},
              display::DisplayRotation::Normal, false}},
            {{id("external"), id("internal"), display::LayoutEdge::Right, 0}}};
}

display::DisplayConfiguration request(const display::HardwareSnapshot& state)
{
    display::DisplayConfiguration value;
    value.base = {state.compositorInstanceId, state.topologyGeneration, 7};
    value.displays = {{id("internal"), true, state.displays[0].modes[0], {},
                       display::DisplayRotation::Normal},
                      {id("external"), true, state.displays[1].modes[0],
                       *compositor::OutputScale::fromNumerator(150),
                       display::DisplayRotation::Normal}};
    value.primary = id("external");
    value.placements = {{id("internal"), id("external"), display::LayoutEdge::Left, 0}};
    return value;
}

struct Fixture {
    base::file::TemporaryDirectory directory {std::filesystem::temp_directory_path().string()};

    std::string path(const std::string &name) const { return directory.path() + '/' + name; }
    display::HardwareSnapshot snapshot = hardware();
    display::DisplayManager manager;
    test::FakeNativeDisplayDelegate delegate;
    std::unique_ptr<display::DisplayConfigurationController> controller;

    Fixture()
    {
        EXPECT_TRUE(directory.isValid());
        delegate.setSnapshot(snapshot);
        EXPECT_TRUE(manager.installHardware(snapshot));
        EXPECT_TRUE(manager.installApplied(applied(snapshot)));
        controller = std::make_unique<display::DisplayConfigurationController>(
            &manager, &delegate,
            display::DisplayPreferences(path("display.json")));
    }
};

class DeferredDelegate : public test::FakeNativeDisplayDelegate
{
public:
    std::deque<ConfigureCallback> pending;
    int calls = 0;
    void configure(const display::DisplayConfiguration&, ConfigureMode,
                   ConfigureCallback callback) override
    {
        ++calls;
        pending.push_back(std::move(callback));
    }
    void complete()
    {
        auto callback = std::move(pending.front());
        pending.pop_front();
        callback({true, false, {}, std::nullopt});
    }
};

} // namespace

TEST(DisplayConfigurationController, AppliesOnlyAfterTestAndPersistsOnlyOnConfirm)
{
    Fixture fixture;
    const std::string operation = fixture.controller->apply(request(fixture.snapshot),
                                                        ":1.20", 1000);
    EXPECT_TRUE(operation.starts_with("apply-"));
    EXPECT_EQ(fixture.delegate.configureCount(), 2);
    EXPECT_EQ(fixture.manager.applied()->revision, 8u);
    ASSERT_FALSE(fixture.controller->previewToken().empty());
    EXPECT_FALSE(std::filesystem::exists(fixture.path("display.json")));

    fixture.controller->confirm(fixture.controller->previewToken(), ":1.20");
    EXPECT_TRUE(std::filesystem::exists(fixture.path("display.json")));
    display::DisplayError error;
    const auto restored = display::DisplayPreferences(
                              fixture.path("display.json"))
                              .restore(fixture.snapshot, 8, &error);
    ASSERT_TRUE(restored) << error.message;
    EXPECT_EQ(restored->primary, id("external"));
}

TEST(DisplayConfigurationController, RejectedConcurrentRequestsDoNotReleaseTheActiveTransaction)
{
    Fixture fixture;
    DeferredDelegate delegate;
    display::DisplayConfigurationController controller(&fixture.manager, &delegate);
    controller.apply(request(fixture.snapshot), "owner", 1000);
    ASSERT_TRUE(controller.busy());
    controller.test(request(fixture.snapshot));
    EXPECT_TRUE(controller.busy());
    controller.setPower({}, display::NativeDisplayDelegate::PowerState::Off);
    EXPECT_TRUE(controller.busy());
    controller.confirm("foreign", "other");
    EXPECT_TRUE(controller.busy());
    delegate.complete();
    ASSERT_EQ(delegate.pending.size(), 1);
    delegate.complete();
    EXPECT_FALSE(controller.busy());
    EXPECT_EQ(fixture.manager.applied()->revision, 8u);
}

TEST(DisplayConfigurationController, ConfirmingAPreviewBeingRevertedKeepsTheRevert)
{
    Fixture fixture;
    DeferredDelegate delegate;
    display::DisplayConfigurationController controller(
        &fixture.manager, &delegate, display::DisplayPreferences(fixture.path("display.json")));
    controller.apply(request(fixture.snapshot), "owner", 1000);
    delegate.complete();
    delegate.complete();
    const std::string preview = controller.previewToken();
    ASSERT_FALSE(preview.empty());
    ASSERT_FALSE(controller.busy());

    controller.revert(preview, "owner");
    ASSERT_TRUE(controller.busy());
    bool confirmed = true;
    const auto finished = controller.operationFinished.connect(
            [&](const std::string& token, const display::ConfigurationResult& result) {
                if (token.starts_with("confirm-"))
                    confirmed = result.succeeded;
            });
    controller.confirm(preview, "owner");
    EXPECT_FALSE(confirmed);
    EXPECT_TRUE(controller.busy());

    delegate.complete();
    delegate.complete();
    EXPECT_FALSE(controller.busy());
    EXPECT_TRUE(controller.previewToken().empty());
    EXPECT_EQ(fixture.manager.primaryDisplay()->id, id("internal"));
    EXPECT_FALSE(std::filesystem::exists(fixture.path("display.json")));
}

TEST(DisplayConfigurationController, SnapshotInvalidationDuringTestPreventsCommit)
{
    Fixture fixture;
    DeferredDelegate delegate;
    display::DisplayConfigurationController controller(&fixture.manager, &delegate);
    display::DisplayErrorCode error = display::DisplayErrorCode::None;
    const auto finished1 = controller.operationFinished.connect(
            [&](const std::string&, const display::ConfigurationResult& result) {
                error = result.error.code;
            });
    controller.apply(request(fixture.snapshot), "owner", 1000);
    delegate.invalidate(fixture.snapshot.topologyGeneration + 1);
    delegate.complete();
    EXPECT_FALSE(controller.busy());
    EXPECT_EQ(error, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_EQ(delegate.calls, 1);
    EXPECT_EQ(fixture.manager.applied()->revision, 7u);
}

TEST(DisplayConfigurationController, TimeoutAndOwnerLossRestorePreviousConfiguration)
{
    Fixture fixture;
    fixture.controller->apply(request(fixture.snapshot), ":1.20", 10);
    ASSERT_TRUE(test::runUntil([&] { return fixture.controller->previewToken().empty(); },
                               std::chrono::seconds(1)));
    EXPECT_EQ(fixture.manager.applied()->revision, 9u);
    EXPECT_EQ(fixture.manager.primaryDisplay()->id, id("internal"));
    EXPECT_EQ(fixture.delegate.configureCount(), 4);

    auto next = request(fixture.snapshot);
    next.base.appliedRevision = 9;
    fixture.controller->apply(next, ":1.21", 1000);
    ASSERT_FALSE(fixture.controller->previewToken().empty());
    fixture.controller->ownerDisconnected(":1.21");
    EXPECT_TRUE(fixture.controller->previewToken().empty());
    EXPECT_EQ(fixture.manager.primaryDisplay()->id, id("internal"));
}

TEST(DisplayConfigurationController, RejectsStaleAndForeignPreviewTokens)
{
    Fixture fixture;
    int failures = 0;
    const auto finished2 = fixture.controller->operationFinished.connect(
            [&failures](const std::string&, const display::ConfigurationResult& result) {
                if (!result.succeeded)
                    ++failures;
            });
    auto stale = request(fixture.snapshot);
    stale.base.appliedRevision = 6;
    fixture.controller->apply(stale, ":1.20", 1000);
    EXPECT_EQ(fixture.delegate.configureCount(), 0);
    EXPECT_EQ(failures, 1);

    fixture.controller->apply(request(fixture.snapshot), ":1.20", 1000);
    fixture.controller->confirm(fixture.controller->previewToken(), ":1.99");
    EXPECT_FALSE(fixture.controller->previewToken().empty());
    EXPECT_EQ(failures, 2);
}

TEST(DisplayConfigurationController, ImmediateChangePersistsWithoutPreviewOrOwnerLifetime)
{
    Fixture fixture;
    fixture.controller->apply(request(fixture.snapshot), ":1.20", 0);
    EXPECT_TRUE(fixture.controller->previewToken().empty());
    EXPECT_EQ(fixture.manager.applied()->revision, 8u);
    EXPECT_TRUE(std::filesystem::exists(fixture.path("display.json")));
    fixture.controller->ownerDisconnected(":1.20");
    EXPECT_EQ(fixture.manager.applied()->revision, 8u);
}

TEST(DisplayConfigurationController, ModeChangesPublishSelectedModeAndRevertToPreviousSelection)
{
    Fixture fixture;
    auto hardware = fixture.snapshot;
    hardware.topologyGeneration++;
    hardware.displays[1].modes.push_back(mode({1920, 1080}));
    fixture.snapshot = hardware;
    fixture.delegate.setSnapshot(hardware);
    ASSERT_TRUE(fixture.manager.installHardware(hardware));
    ASSERT_TRUE(fixture.manager.installApplied(applied(hardware)));
    auto first = request(hardware);
    first.displays[1].mode = hardware.displays[1].modes[1];
    fixture.controller->apply(first, ":1.20", 0);
    ASSERT_EQ(fixture.manager.applied()->revision, 8u);
    EXPECT_EQ(fixture.manager.applied()->physicalDisplays[1].currentMode->pixelSize, base::Size(1920, 1080));
    auto second = request(hardware);
    second.base.appliedRevision = 8;
    fixture.controller->apply(second, ":1.20", 1000);
    EXPECT_EQ(fixture.manager.applied()->physicalDisplays[1].currentMode->pixelSize, base::Size(2560, 1440));
    fixture.controller->revert(fixture.controller->previewToken(), ":1.20");
    EXPECT_EQ(fixture.manager.applied()->physicalDisplays[1].currentMode->pixelSize, base::Size(1920, 1080));
    ASSERT_TRUE(fixture.delegate.lastConfiguration());
    EXPECT_EQ(fixture.delegate.lastConfiguration()->displays[1].mode.pixelSize, base::Size(1920, 1080));
}

TEST(DisplayConfigurationController, ImmediatePersistenceFailureRestoresPreviousConfiguration)
{
    Fixture fixture;
    fixture.controller = std::make_unique<display::DisplayConfigurationController>(
        &fixture.manager, &fixture.delegate, display::DisplayPreferences(fixture.directory.path()));
    bool persistenceFailed = false;
    const auto finished3 = fixture.controller->operationFinished.connect(
            [&](const std::string&, const display::ConfigurationResult& result) {
                persistenceFailed |= !result.succeeded && result.error.code == display::DisplayErrorCode::PersistenceFailed;
            });
    fixture.controller->apply(request(fixture.snapshot), ":1.20", 0);
    EXPECT_TRUE(persistenceFailed);
    EXPECT_TRUE(fixture.controller->previewToken().empty());
    EXPECT_EQ(fixture.manager.primaryDisplay()->id, id("internal"));
    EXPECT_EQ(fixture.delegate.configureCount(), 4);
}

TEST(DisplayConfigurationController, PersistenceFailureCompletesOnlyAfterAsynchronousRollback)
{
    Fixture fixture;
    DeferredDelegate delegate;
    display::DisplayConfigurationController controller(
        &fixture.manager, &delegate, display::DisplayPreferences(fixture.directory.path()));
    std::string completedToken;
    display::ConfigurationResult completion;
    const auto finished4 = controller.operationFinished.connect(
            [&](const std::string& token, const display::ConfigurationResult& result) {
                completedToken = token;
                completion = result;
            });
    const std::string token = controller.apply(request(fixture.snapshot), ":1.20", 0);
    delegate.complete();
    delegate.complete();
    EXPECT_TRUE(controller.busy());
    EXPECT_TRUE(completedToken.empty());
    delegate.complete();
    EXPECT_TRUE(controller.busy());
    delegate.complete();
    EXPECT_FALSE(controller.busy());
    EXPECT_EQ(completedToken, token);
    EXPECT_FALSE(completion.succeeded);
    EXPECT_EQ(completion.error.code, display::DisplayErrorCode::PersistenceFailed);
    EXPECT_EQ(fixture.manager.primaryDisplay()->id, id("internal"));
}

TEST(DisplayConfigurationController, MirrorRollbackKeepsTheSourceAsPrimary)
{
    auto snapshot = hardware();
    for (auto& physical : snapshot.displays)
        physical.capabilities |= display::DisplayCapability::SoftwareMirror;
    auto mirrored = applied(snapshot);
    mirrored.logicalDisplays.pop_back();
    mirrored.logicalDisplays[0].destinations.push_back(id("external"));
    mirrored.placements.clear();
    display::DisplayManager manager;
    ASSERT_TRUE(manager.installHardware(snapshot));
    ASSERT_TRUE(manager.installApplied(mirrored));
    test::FakeNativeDisplayDelegate delegate;
    display::DisplayConfigurationController controller(&manager, &delegate);
    controller.apply(request(snapshot), ":1.20", 1000);
    ASSERT_FALSE(controller.previewToken().empty());
    controller.revert(controller.previewToken(), ":1.20");
    ASSERT_TRUE(delegate.lastConfiguration());
    EXPECT_EQ(delegate.lastConfiguration()->primary, id("internal"));
    EXPECT_EQ(manager.applied()->logicalDisplays.size(), 1);
}
