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


#ifndef __LINGMOFONTINTERFACE_H__
#define __LINGMOFONTINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusFont(void);

//Delete the DBus Communication
bool DeInitDBusFont(void);

/*
 * The resolution used for converting font sizes to pixel sizes,
 * in dots per inch.
 */
void lingmo_personal_fonts_dpi(const double in_arg);

/*
 * The type of antialiasing to use when rendering fonts.
 * Possible values are:
 * @in_arg is equal to 0, "none" for no antialiasing;
 * @in_arg is equal to 1, "grayscale" for standard grayscale antialiasing;
 * @in_arg is equal to 2, "rgba" for subpixel antialiasing (LCD screens only).
 */
void lingmo_personal_fonts_antialias(const int in_arg);

/*
 * The type of hinting to use when rendering fonts.
 * Possible values are:
 * @in_arg is equal to 0, "none" for no hinting;
 * @in_arg is equal to 1, "slight" for basic;
 * @in_arg is equal to 2, "medium" for moderate;
 * @in_arg is equal to 3, "full" for maximum hinting (may cause distortion of letter forms).
 */
void lingmo_personal_fonts_hint(const int in_arg);

 /*
  * The order of subpixel elements on an LCD screen; only used when antialiasing is set to "rgba".
  * Possible values are:
  * @in_arg is equal to 0, "rgb" for red on left (most common);
  * @in_arg is equal to 1, "bgr" for blue on left;
  * @in_arg is equal to 2, "vrgb" for red on top;
  * @in_arg is equal to 3, "vbgr" for red on bottom.
  */
void lingmo_personal_fonts_rgbaorder(const int in_arg);

#ifdef __cplusplus
}
#endif

#endif // FONTRENDERINGINTERFACE_H
