// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "backend/drm/drm_output.h"
#include "backend/drm/drm_properties.h"

using namespace Lingmo::Backend;

namespace {

// The property set a virtio_gpu primary plane exposes.
DrmPropertyMap makePlaneProperties()
{
    DrmPropertyMap map;
    uint32_t id = 100;
    for (const char* name : {"type", "FB_ID", "CRTC_ID", "SRC_X", "SRC_Y", "SRC_W", "SRC_H",
                             "CRTC_X", "CRTC_Y", "CRTC_W", "CRTC_H"}) {
        map.insert(name, id, 0);
        ++id;
    }
    return map;
}

} // namespace

TEST(DrmPropertyMap, EmptyByDefault)
{
    const DrmPropertyMap map;
    EXPECT_TRUE(map.isEmpty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains("CRTC_ID"));
}

TEST(DrmPropertyMap, LooksUpIdsAndValues)
{
    DrmPropertyMap map;
    map.insert("MODE_ID", 31, 7);
    map.insert("ACTIVE", 32, 1);

    EXPECT_TRUE(map.contains("MODE_ID"));
    EXPECT_EQ(map.id("MODE_ID"), 31u);
    EXPECT_EQ(map.value("MODE_ID"), 7u);
    EXPECT_EQ(map.id("ACTIVE"), 32u);
    EXPECT_EQ(map.size(), 2);

    const auto property = map.find("ACTIVE");
    ASSERT_TRUE(property.has_value());
    EXPECT_EQ(property->id, 32u);
    EXPECT_EQ(property->value, 1u);
}

TEST(DrmPropertyMap, AbsentPropertyHasIdZero)
{
    DrmPropertyMap map;
    map.insert("ACTIVE", 32, 1);

    EXPECT_EQ(map.id("VRR_ENABLED"), 0u);
    EXPECT_EQ(map.value("VRR_ENABLED"), 0u);
    EXPECT_EQ(map.value("VRR_ENABLED", 42), 42u);
    EXPECT_FALSE(map.find("VRR_ENABLED").has_value());
}

TEST(DrmPropertyMap, LookupIsCaseSensitive)
{
    DrmPropertyMap map;
    map.insert("CRTC_ID", 5, 0);
    EXPECT_EQ(map.id("crtc_id"), 0u);
}

TEST(DrmPropertyMap, InsertReplaces)
{
    DrmPropertyMap map;
    map.insert("ACTIVE", 32, 0);
    map.insert("ACTIVE", 32, 1);
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.value("ACTIVE"), 1u);
}

TEST(DrmPropertyMap, ReportsMissingRequiredProperties)
{
    DrmPropertyMap map;
    map.insert("MODE_ID", 31, 0);

    EXPECT_TRUE(map.missing({"MODE_ID"}).empty());
    EXPECT_EQ(map.missing(DrmOutput::requiredCrtcProperties()), std::vector<std::string> {"ACTIVE"});
}

TEST(DrmPropertyMap, AtomicPlaneRequirementsAreSatisfied)
{
    const DrmPropertyMap plane = makePlaneProperties();
    EXPECT_TRUE(plane.missing(DrmOutput::requiredPlaneProperties()).empty());
    EXPECT_EQ(plane.value("type"), 0u);

    DrmPropertyMap connector;
    connector.insert("CRTC_ID", 20, 0);
    EXPECT_TRUE(connector.missing(DrmOutput::requiredConnectorProperties()).empty());
}

TEST(DrmPropertyMap, MissingListsEveryAbsentName)
{
    const DrmPropertyMap empty;
    EXPECT_EQ(empty.missing(DrmOutput::requiredPlaneProperties()).size(),
              DrmOutput::requiredPlaneProperties().size());
}
