// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Every number and colour the server-side frame is drawn from, in one place.
//
// The values come from the existing Lingmo decoration rather than from a new
// design: the KWin plugin in `kwin-plugins/plugins/decoration` (titlebar
// height, frame radius, caption button size, spacing and hover treatment,
// titlebar and caption colours) and FishUI's window frame (the hairline window
// border, and its active/inactive alpha). Nothing here is derived from the
// KDecoration API, and none of it is linked: only the metrics were carried
// across.

#ifndef LINGMO_DECORATION_DECORATION_THEME_H_
#define LINGMO_DECORATION_DECORATION_THEME_H_

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <string>

#include "decoration/raster.h"
#include "decoration/shadow.h"
#include "decoration/text.h"
#include "decoration/window_visual_style.h"
#include "render/renderer.h"
#include "wm/shadow_types.h"
#include "wm/types.h"

namespace Lingmo::Decoration {

// Frame layout uses logical pixels; the exterior hairline uses physical pixels.
struct Metrics {
    Metrics();

    // Lower than FishUI's 40px header, which also carries toolbar controls;
    // the buttons centre in it.
    int titleBarHeight = 36;
    // The outline overlays client pixels; resize hit areas do not need a visible inset.
    int borderWidth = 0;
    // FishUI's window corner radius, shared by all compositor-owned corners.
    // Initialized in decoration_theme.cpp from the shared visual style.
    int cornerRadius;
    // kwin-plugins: setResizeOnlyBorders(5), which reaches outside the window.
    int resizeMargin = 5;
    // How far along an edge counts as the corner rather than the edge.
    int resizeCorner = 16;
    // FishUI's title bar: 7px between hover circles and to the right edge, so
    // right-aligned windows line their buttons up.
    int buttonSize = 32;
    int buttonSpacing = 1;
    int buttonsRightMargin = 4;
    // The artwork is drawn on a 24x24 grid inside the button.
    int iconSize = 24;
    // kwin-plugins: the caption keeps this much clear of the buttons.
    int captionMargin = 20;
    // Logical pixel size of the caption font: the session's FontSize (points)
    // at a 96 dpi logical density, see Theme::setCaptionPointSize. The
    // default is the Appearance service's 9pt.
    int textPixelSize = 12;

    ShadowStyle shadow(bool active) const
    {
        return shadowFor(active ? wm::ShadowElevation::Active : wm::ShadowElevation::Inactive);
    }

    // Three lobes: a wide ambient one carried well below the window, a tighter
    // directional one under the lower edge, and a near-hard contact lobe that
    // draws the thin dark line hugging the border. Peak opacity stays low and
    // the falloff long, which is what makes the result read as diffuse rather
    // than as a dark halo. `contact` scales the third lobe on its own, so a
    // role can keep the diffuse falloff and drop the drawn edge.
    //
    // Chromium carries a dip elevation and derives blur and offset from it;
    // Lingmo keeps fixed lobes per role and varies only their strength
    // between active and inactive, so the exterior margins - and therefore the
    // window's scene bounds - do not move when a window is activated. Sigma is
    // capped at 32 by ShadowGeometry, and every extra logical pixel of sigma
    // grows the nine-patch source on each side, so 24 is the widest lobe used.
    ShadowStyle shadowFor(wm::ShadowElevation elevation) const
    {
        const auto lobes = [](float wide, float narrow, int offset, float strength,
                              float contact = 1.0f) {
            return ShadowStyle {{{wide, base::Point(0, offset), 0.22f * strength},
                                 {narrow, base::Point(0, (offset + 1) / 2), 0.15f * strength},
                                 {1.5f, base::Point(0, 1), 0.16f * strength * contact}}};
        };
        switch (elevation) {
        case wm::ShadowElevation::None:
            return ShadowStyle {};
        // The dock floats over the wallpaper rather than sitting on it: a wide,
        // very faint falloff and no contact line at all, so the panel's own
        // rounded edge stays the only drawn boundary.
        case wm::ShadowElevation::Dock:
            return lobes(10, 4, 3, 0.25f, 0.0f);
        case wm::ShadowElevation::Popup:
            return lobes(8, 4, 3, 0.7f);
        case wm::ShadowElevation::MenuOrTooltip:
            return lobes(14, 6, 8, 0.85f);
        case wm::ShadowElevation::Inactive:
            return lobes(24, 9, 14, 0.55f);
        case wm::ShadowElevation::Active:
            return lobes(24, 9, 14, 1.0f);
        }
        return ShadowStyle {};
    }

    // What the frame takes out of the outer window bounds.
    wm::Insets insets() const
    {
        return wm::Insets {borderWidth, titleBarHeight, borderWidth, borderWidth};
    }
};

struct Palette {
    render::Color titleBar;
    render::Color captionActive;
    render::Color captionInactive;
    render::Color buttonHover;
    render::Color buttonPressed;
    render::Color buttonIcon;
};

// Told when the palette changes, so a frame already on screen repaints
// instead of keeping the old colours until something else touches it.
class ThemeObserver
{
public:
    virtual ~ThemeObserver() = default;

    virtual void onThemeChanged() = 0;
};

// Shared by every frame: the constants, and the textures rasterized from them.
class Theme
{
public:
    Theme();
    Theme(const Theme &) = delete;
    Theme &operator=(const Theme &) = delete;
    ~Theme();

    void addObserver(ThemeObserver *observer);
    void removeObserver(ThemeObserver *observer);

    const Metrics &metrics() const { return m_metrics; }

    TextRenderer &textRenderer() { return m_textRenderer; }

    bool darkMode() const { return m_darkMode; }

    // Drops the textures rasterized from the old palette and notifies observers.
    void setDarkMode(bool dark);

    // Follows the session's system font size; frames re-raster their caption
    // on the next paint, so this only notifies observers.
    void setCaptionPointSize(double points);

    // Switches every window silhouette between continuous and circular corners
    // (render::setContinuousCorners), drops the shadows rasterized for the old
    // shape and notifies observers.
    void setContinuousCorners(bool continuous);

    // The Appearance service's AccentColor index; only notifies observers.
    void setAccentColor(int index);
    int accentColor() const { return m_accentColor; }

    const Palette &palette() const;

    // The colour the caption and the icons are drawn in.
    render::Color captionColor(bool active) const;
    // The outline pair for the current mode; re-read on ThemeObserver::onThemeChanged.
    WindowVisualStyle visualStyle() const;

    // A texture for `key`, rasterized by `make` the first time it is asked for.
    // Textures belong to the renderer that created them, so the whole cache is
    // dropped if a different one turns up.
    render::Texture *texture(render::Renderer &renderer, const std::string &key,
                             const std::function<Image()> &make);

    // Drops every cached texture. Must run while the renderer's context is
    // still alive.
    void clearTextures();

private:
    void notifyObservers();

    Metrics m_metrics;
    TextRenderer m_textRenderer;
    bool m_darkMode = false;
    int m_accentColor = 0;
    std::vector<ThemeObserver *> m_observers;

    render::Renderer *m_renderer = nullptr;
    std::map<std::string, std::unique_ptr<render::Texture>> m_textures;
};

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_DECORATION_THEME_H_
