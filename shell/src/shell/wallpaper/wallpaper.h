// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "shell/shell.h"
#include "shell/wallpaper/bar_backdrop.h"
#include "shell/wallpaper/wallpaper_cache.h"
#include "ui/bitmap.h"
#include "ui/geometry.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::appearance {
class Appearance;
}

namespace Lingmo::Compositor {
class CaptureSource;
}

namespace Lingmo::Scene {
class Node;
class WallpaperNode;
}

namespace Lingmo::WM {
class Window;
}

namespace Lingmo::Shell {

// The desktop background of every output, under the desktop surfaces: the
// picture or the plain colour
// org.lingmo.Services.Appearance names, one WallpaperNode per output at the
// bottom of the desktop layer, and on the primary output the gradient under
// the status bar. A new picture replaces the old one only once it is decoded
// and, on the primary output, judged for the bar; the picture, the gradient
// and the bar's contents then change in the same frame.
class Wallpaper final : public Component
{
public:
    // What the desktop shows, as Appearance publishes it.
    struct Background {
        // BackgroundType: 0 a picture, otherwise the colour.
        int type = 0;
        // A local file.
        std::string path;
        ui::Color color {0x2b, 0x8a, 0xda, 255};

        bool operator==(const Background &) const = default;
    };

    // `cacheDirectory` holds the disk cache; empty is the user's.
    // `servicesSettings` is read while the Appearance service is not up;
    // empty reads the user's services.conf.
    explicit Wallpaper(Shell *shell, std::string cacheDirectory = {}, std::string servicesSettings = {});
    ~Wallpaper() override;

    // Follows org.lingmo.Services.Appearance. Until it answers, services.conf
    // names the background, as the service will.
    void followAppearance(appearance::Appearance *appearance);
    void setBackground(const Background &background);
    const Background &background() const { return m_background; }
    base::Signal<> backgroundChanged;

    WallpaperCache &cache() { return m_cache; }

    // What is behind the status bar, judged from what the primary output
    // shows; invalid while nothing is judged.
    bool isJudged() const { return m_judged; }
    const BarBackdrop::Style &style() const { return m_style; }
    // The judgement changed, in the same loop turn as the picture it is for.
    base::Signal<> judgementChanged;
    // Average wallpaper colour behind a global logical rectangle, including
    // the readability gradient actually drawn under the status bar.
    BarBackdrop::Sample sampleBehind(const ui::RectF& globalRect) const;

    // The primary output's picture as shown, null for a colour or nothing.
    ui::Bitmap primaryPicture() const;

    // Every output shows what it is going to: a picture, the colour, or
    // nothing because the background is off or could not be read.
    bool isReady() const;

    scene::WallpaperNode *node(const std::string &output) const;
    std::vector<const scene::Node *> nodes() const;

    // The output's desktop as window capture picks it: the wallpaper with
    // the desktop window's icons over it.
    std::shared_ptr<compositor::CaptureSource> desktopCaptureSource(wm::Window *desktop, const std::string &output);

private:
    struct Screen;
    class CaptureNode;

    void updateOutputs();
    void reload(Screen &screen);
    void tryCommit(Screen &screen);
    void decoded(Screen &screen, const ui::Bitmap &image);
    void present(const Screen &screen);
    void publish(bool judged, const BarBackdrop::Style &style);
    void updateGradient();
    void readServicesSettings();
    void reloadAppearance();
    static std::string sampleKey(const std::string &path, const base::Size &size);

    Shell *m_shell;
    WallpaperCache m_cache;
    std::string m_servicesSettings;
    Background m_background;
    bool m_hasBackground = false;
    appearance::Appearance *m_appearance = nullptr;
    std::unique_ptr<appearance::Appearance> m_ownAppearance;
    base::Subscription m_appearanceChanged;
    base::Job m_settingsJob;
    std::vector<std::unique_ptr<Screen>> m_screens;
    std::map<std::string, BarBackdrop::Sample> m_samples;
    bool m_judged = false;
    BarBackdrop::Style m_style;
    base::Subscription m_outputsChanged;
};

} // namespace Lingmo::Shell

