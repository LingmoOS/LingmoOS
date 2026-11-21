
#include <string>

#pragma once
struct GtkSettings
{
    std::string fontFamily;
    double fontPointSize = 9;
    // fontconfig's values, as written by writeFontConfig().
    bool fontAntialias = true;
    std::string fontHintStyle = "hintfull";
    std::string fontRgba = "rgb";
    bool darkMode = false;
    std::string iconTheme;
    std::string cursorTheme;
    int cursorSize = 24;
};

// Mirrors the system appearance into GTK's config: settings.ini for Xwayland clients and the
// GSettings keyfile for Wayland clients.
void writeGtkSettings(const GtkSettings &settings);

// Puts the Lingmo stylesheet import at the top of the user's gtk.css for GTK 3 and 4, keeping
// the rest of the file. It is re-added at every login, even after the user removed it. The
// stylesheet carries the palette, so it follows the appearance's light or dark mode.
void ensureGtkCssImports(bool darkMode);

