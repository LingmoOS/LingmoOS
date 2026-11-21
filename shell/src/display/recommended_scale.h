// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_change_observer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#pragma once

#include "display/display_types.h"

namespace Lingmo::Display {

// The scale a display starts at until someone picks another: from its pixel
// density in its native mode, snapped to the steps Settings offers.
compositor::OutputScale recommendedScale(const DisplaySnapshot &display);

} // namespace Lingmo::Display
