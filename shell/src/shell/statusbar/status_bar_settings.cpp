// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_bar_settings.h"

#include <algorithm>
#include <cmath>

#include "base/settings.h"
#include "shell/common/file_thread.h"

namespace Lingmo::Shell {

StatusBarSettings::StatusBarSettings(std::string path)
    : m_path(std::move(path))
{
    m_load = runOnFileThread([path = m_path] { return load(path); },
                             [this](Values values) {
                                 const bool blur = values.blurSigma != m_blurSigma;
                                 const bool opacity = values.opacity != m_opacity;
                                 m_blurSigma = values.blurSigma;
                                 m_opacity = values.opacity;
                                 m_ready = true;
                                 ready();
                                 if (blur)
                                     blurSigmaChanged();
                                 if (opacity)
                                     opacityChanged();
                             });
}

StatusBarSettings::Values StatusBarSettings::load(const std::string &path)
{
    base::Settings settings(path);
    // Missing keys are written with their defaults.
    if (!settings.contains("Statusbar/BlurSigma"))
        settings.setValue("Statusbar/BlurSigma", kDefaultBlurSigma);
    if (!settings.contains("Statusbar/Opacity"))
        settings.setValue("Statusbar/Opacity", kDefaultOpacity);
    settings.sync();

    Values values;
    const double blur = settings.doubleValue("Statusbar/BlurSigma", kDefaultBlurSigma);
    values.blurSigma = std::isfinite(blur) ? std::clamp(blur, 0.0, kMaxBlurSigma) : kDefaultBlurSigma;
    const double opacity = settings.doubleValue("Statusbar/Opacity", kDefaultOpacity);
    values.opacity = std::isfinite(opacity) ? std::clamp(opacity, 0.0, 1.0) : kDefaultOpacity;
    return values;
}

void StatusBarSettings::write(const char *key, double value)
{
    const std::string name = key;
    writeSettings(m_path, [name, value](base::Settings &settings) { settings.setValue(name, value); });
}

void StatusBarSettings::setBlurSigma(double sigma)
{
    sigma = std::isfinite(sigma) ? std::clamp(sigma, 0.0, kMaxBlurSigma) : kDefaultBlurSigma;
    if (std::abs(sigma - m_blurSigma) < 1e-12)
        return;
    m_blurSigma = sigma;
    write("Statusbar/BlurSigma", sigma);
    blurSigmaChanged();
}

void StatusBarSettings::setOpacity(double opacity)
{
    opacity = std::isfinite(opacity) ? std::clamp(opacity, 0.0, 1.0) : kDefaultOpacity;
    if (std::abs(opacity - m_opacity) < 1e-12)
        return;
    m_opacity = opacity;
    write("Statusbar/Opacity", opacity);
    opacityChanged();
}

} // namespace Lingmo::Shell
