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


#ifndef __LINGMOMOUSEINTERFACE_H__
#define __LINGMOMOUSEINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the DBus Communication.
 */
bool InitDBusMouse(void);

/**
 * Delete the DBus Communication.
 */
bool DeInitDBusMouse(void);

/**
 * Swap left and right mouse buttons for left-handed mice.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_lefthanded(const bool in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
bool lingmo_hardware_mouse_get_lefthanded();

/**
 * Set acceleration multiplier for mouse motion.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_motionacceleration(const double in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
double lingmo_hardware_mouse_get_motionacceleration();

/**
 * Set the distance in pixels the pointer must move before accelerated mouse motion is activated.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_motionthreshold(const int in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
int lingmo_hardware_mouse_get_motionthreshold();

/**
 * Set the distance before a drag is started.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_dragthreshold(const int in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
int lingmo_hardware_mouse_get_dragthreshold();

/**
 * Set the length of a double click.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_doubleclick(const int in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
int lingmo_hardware_mouse_get_doubleclick();

/**
 * Enable middle mouse button emulation through simultaneous left and right button click.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_middlebuttonenabled(const bool in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
bool lingmo_hardware_mouse_get_middlebuttonenabled();

/**
 * Highlight the current location of the pointer when the Control key is pressed and released.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_locatepointer(const bool in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
bool lingmo_hardware_mouse_get_locatepointer();

/**
 * Set font name of the cursor. If unset, the default font is used.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_cursorfont(const char *in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
char *lingmo_hardware_mouse_get_cursorfont();

/**
 * Set cursor theme name. Used only by Xservers that support Xcursor,such as XFree86 4.3 and later.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_cursortheme(const char *in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
char *lingmo_hardware_mouse_get_cursortheme();

/**
 * Set the size of the cursor referenced by cursor_theme.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_mouse_set_cursorsize(const int in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
int lingmo_hardware_mouse_get_cursorsize();

#ifdef __cplusplus
}
#endif

#endif













