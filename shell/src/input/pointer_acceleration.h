// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_INPUT_POINTER_ACCELERATION_H_
#define LINGMO_INPUT_POINTER_ACCELERATION_H_

#include <vector>

namespace Lingmo::Input {

// Mouse acceleration as a gain in physical units: logical pixels per inch of
// mouse travel at a given hand speed. The curve is the parametric family of
// IOHIDFamily's IOHIDParametricAcceleration (IOHIDEventSystemPlugIns/
// IOHIDAccelerationAlgorithm.cpp) with the curve table of its generic HID
// pointer personality (IOHIDFamily/Info-IOHIDEventDriver.plist): a polynomial
// that is nearly linear at a crawl and bends upwards through the middle
// speeds, continued along its tangent, then growing as a square root, so a
// flick still reaches far without running away. ChromeOS keeps its curves in
// libgestures, which the Chromium tree does not carry.
//
// `speed` is the settings slider, -1 to 1, with a step per table entry: -1 is
// the first (linear, no acceleration), 0 the fourth and default (0.6875), 1
// the last (3), each step below 0 a third and each above it a sixth. Between
// steps the curve is interpolated as the table is.
double pointerGain(double inchesPerSecond, double speed);

// The curve sampled for libinput's custom profile, whose axes are the raw
// device speed in counts per millisecond and the pointer speed in pixels per
// millisecond: resolution is the caller's business there, so the mouse's is
// folded in here.
struct AccelerationPoints {
    double step = 0;
    std::vector<double> points;
};

AccelerationPoints accelerationPoints(int dotsPerInch, double speed);

// The default resolution from udev's MOUSE_DPI ("1000@125", or a list with
// the default starred, "400 *800 1600@125"); libinput's 1000 when absent.
int mouseDpi(const char *property);

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_POINTER_ACCELERATION_H_
