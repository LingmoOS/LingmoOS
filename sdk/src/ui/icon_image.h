// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "ui/view.h"

namespace Lingmo::ui {

// An icon as an image provider's pixmap: the theme's icon by name
// (Theme::iconTheme, then hicolor, then /usr/share/pixmaps), or the file at an
// absolute path, made `pixels` square at a device pixel ratio of 1 and drawn
// smoothly stretched into the view's rect, unsnapped.
class IconImage : public View
{
public:
    explicit IconImage(std::string name = {}, int pixels = 0);

    void setName(std::string name);
    const std::string& name() const { return m_name; }
    // The pixmap's size in pixels, whatever size the view is drawn at.
    void setPixels(int pixels);
    // Shown when nothing has the name.
    void setFallbackName(std::string name);

    // Makes the pixmap now, into the cache the views draw from, so an idle
    // turn of the host can pay for it; false when nothing has the name.
    static bool preload(const std::string& name, int pixels);

protected:
    void paint(Painter& painter) override;

private:
    std::string m_name;
    std::string m_fallback;
    int m_pixels;
};

} // namespace Lingmo::ui
