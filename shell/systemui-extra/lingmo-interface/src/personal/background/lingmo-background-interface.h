/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#ifndef __LINGMOBACKGROUNDINTERFACE_H__
#define __LINGMOBACKGROUNDINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusBackground(void);

//Delete the DBus Communication
bool DeInitDBusBackground(void);

/*
 * Draw Desktop Background.
 */
void lingmo_personal_personal_drawbackground(const bool in_arg);

/*
 * Show Desktop Icons.
 */
void lingmo_personal_personal_showdesktopicons(const bool in_arg);

/*
 * Fade the background on change.
 * If set to true, then MATE will change the desktop background with a fading effect.
 */
void lingmo_personal_personal_backgroundfade(const bool in_arg);

/*
 * Determines how the image set by wallpaper_filename is rendered.
 * Possible values are:
 * @in_arg is equal to 0, "wallpaper";
 * @in_arg is equal to 1, "zoom";
 * @in_arg is equal to 2, "centered";
 * @in_arg is equal to 3, "scaled";
 * @in_arg is equal to 4, "stretched";
 * @in_arg is equal to 5, "spanned".
 */
void lingmo_personal_personal_pictureoptions(const int in_arg);

/*
 * File to use for the background image.
 */
void lingmo_personal_personal_picturefilename(const char *in_arg);

/*
 * Opacity with which to draw the background picture.
 */
void lingmo_personal_personal_pictureopacity(const int in_arg);

/*
 * Left or Top color when drawing gradients, or the solid color.
 */
void lingmo_personal_personal_primarycolor(const char *in_arg);

/*
 * Right or Bottom color when drawing gradients, not used for solid color.
 */
void lingmo_personal_personal_secondarycolor(const char *in_arg);

/*
 * How to shade the background color.
 * Possible values are:
 * @in_arg is equal to 0, "solid";
 * @in_arg is equal to 1, "horizontal-gradient";
 * @in_arg is equal to 2, "vertical-gradient".
 */
void lingmo_personal_personal_colorshadingtype(const int in_arg);

#ifdef __cplusplus
}
#endif

#endif













