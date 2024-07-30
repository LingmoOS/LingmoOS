/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2002 Michael v.Ostheim <MvOstheim@web.de>
 */

#ifndef XVIDEXTWRAP_H
#define XVIDEXTWRAP_H

#include <X11/Xutil.h>

/**A wrapper for XF86VidMode Extension
 *@author Michael v.Ostheim
 */

class XVidExtWrap
{
public:
    enum GammaChannel { Value = 0, Red = 1, Green = 2, Blue = 3 };
    explicit XVidExtWrap(bool *OK, const char *displayname = nullptr);
    ~XVidExtWrap();

    /** Returns the default screen */
    int _DefaultScreen();
    /** Returns the number of screens (extracted from XF86Config) */
    int _ScreenCount();
    /** Returns the displayname */
    const char *DisplayName();
    /** Sets the screen actions are take effect */
    void setScreen(int scrn)
    {
        screen = scrn;
    }
    /** Returns the current screen */
    int getScreen()
    {
        return screen;
    }
    /** Sets the gamma value on the current screen */
    void setGamma(int channel, float gam, bool *OK = nullptr);
    /** Gets the gamma value of the current screen */
    float getGamma(int channel, bool *OK = nullptr);
    /** Limits the possible gamma values (default 0.1-10.0) */
    void setGammaLimits(float min, float max);

private:
    float mingamma, maxgamma;
    int screen;
    Display *dpy;
};

#endif
