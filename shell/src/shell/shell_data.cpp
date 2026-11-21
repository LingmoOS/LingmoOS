// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_data.h"

#include <cmath>
#include <string_view>

namespace Lingmo::Shell {

static std::string &dataDirectory()
{
    static std::string directory = LINGMO_SHELL_DATA_DIR;
    return directory;
}

std::string shellDataFile(std::string_view name)
{
    return dataDirectory() + "/" + std::string(name);
}

void setShellDataDirectoryForTesting(std::string directory)
{
    dataDirectory() = std::move(directory);
}

float glassFixed(double value)
{
    // wl_fixed_from_double: 24.8 fixed point, rounded.
    return float(std::round(value * 256.0) / 256.0);
}

// A colour component rounded to 16 bits, then back to a double.
static double colorComponent(double value)
{
    return std::round(value * 65535.0) / 65535.0;
}

render::GlassParams fishuiGlass()
{
    render::GlassParams glass;
    glass.enabled = true;
    glass.blurSigma = glassFixed(1.2);
    glass.bevel = glassFixed(14);
    glass.depth = glassFixed(16);
    glass.ior = glassFixed(1.7);
    glass.profile = glassFixed(2.5);
    glass.displacement = glassFixed(36);
    glass.chroma = glassFixed(0.05);
    glass.lightAngle = glassFixed(50);
    glass.specularIntensity = 0;
    glass.shininess = glassFixed(42);
    glass.sheen = 0;
    glass.ambientOcclusion = 0;
    glass.ambientOcclusionRadius = glassFixed(4);
    glass.saturation = glassFixed(1.0);
    glass.rim = 0;
    glass.tintRed = glassFixed(1);
    glass.tintGreen = glassFixed(1);
    glass.tintBlue = glassFixed(1);
    glass.tintAlpha = glassFixed(colorComponent(0.12));
    return glass;
}

render::GlassParams popupGlass(double red, double green, double blue, double alpha)
{
    render::GlassParams glass = fishuiGlass();
    glass.blurSigma = glassFixed(6.0);
    glass.displacement = glassFixed(20.0);
    glass.chroma = glassFixed(0.03);
    glass.saturation = glassFixed(1.2);
    glass.tintRed = glassFixed(colorComponent(red));
    glass.tintGreen = glassFixed(colorComponent(green));
    glass.tintBlue = glassFixed(colorComponent(blue));
    glass.tintAlpha = glassFixed(colorComponent(alpha));
    return glass;
}

} // namespace Lingmo::Shell
