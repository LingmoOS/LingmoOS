// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once


#include <string>

#include "base/signal.h"
#include "base/thread_pool.h"

namespace Lingmo::Shell {

// The status bar's [Statusbar] group of shell.conf: the Gaussian blur behind
// the bar and how opaque that blur is. Missing keys are written with their
// defaults. Read on the file thread; the defaults stand until isReady().
class StatusBarSettings
{
public:
    // Logical pixels of Gaussian sigma: enough to read as frosted glass over
    // the wallpaper, small enough to keep what is behind recognisable.
    static constexpr double kDefaultBlurSigma = 20.0;
    static constexpr double kDefaultOpacity = 0.55;
    // Past this the bar is only an expensive sheet of frost.
    static constexpr double kMaxBlurSigma = 64.0;

    // `path` is shell.conf.
    explicit StatusBarSettings(std::string path);

    bool isReady() const { return m_ready; }

    double blurSigma() const { return m_blurSigma; }
    // Clamped to 0..kMaxBlurSigma; not a number is the default.
    void setBlurSigma(double sigma);

    double opacity() const { return m_opacity; }
    // Clamped to 0..1; 0 leaves the bar clear.
    void setOpacity(double opacity);

    base::Signal<> ready;
    base::Signal<> blurSigmaChanged;
    base::Signal<> opacityChanged;

private:
    struct Values {
        double blurSigma = kDefaultBlurSigma;
        double opacity = kDefaultOpacity;
    };

    static Values load(const std::string &path);
    void write(const char *key, double value);

    std::string m_path;
    bool m_ready = false;
    double m_blurSigma = kDefaultBlurSigma;
    double m_opacity = kDefaultOpacity;
    base::Job m_load;
};

} // namespace Lingmo::Shell
