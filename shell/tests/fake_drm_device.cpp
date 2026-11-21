// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/fake_drm_device.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "fake_drm_device.h"

#include <xf86drmMode.h>

namespace Lingmo::Testing {

size_t FakeDrmDevice::countCommits(bool testOnly) const
{
    size_t count = 0;
    for (const Commit& commit : commits) {
        if (static_cast<bool>(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY) == testOnly)
            ++count;
    }
    return count;
}

} // namespace Lingmo::Testing
