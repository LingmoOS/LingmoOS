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


#ifndef __LINGMOINTERFACEINTERFACE_H__
#define __LINGMOINTERFACEINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the DBus Communication.
 */
bool InitDBusInterface(void);

/**
 * Delete the DBus Communication.
 */
bool DeInitDBusInterface(void);

/**
 * Set cursor blink.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_keyboard_set_cursorblink(const bool in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
bool lingmo_hardware_keyboard_get_cursorblink();

/**
 * Set cursor blink time.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_hardware_keyboard_set_cursorblinktime(const int in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
int lingmo_hardware_keyboard_get_cursorblinktime();

/**
 * Set the name of the default font used by gtk+.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_personal_fonts_set_fontname(const char *in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
char *lingmo_personal_fonts_get_fontname();

/**
 * Set the name of the default font used for reading documents.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_personal_fonts_set_documentfont(const char *in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
char *lingmo_personal_fonts_get_documentfont();

/**
 * Set the name of a monospaced (fixed-width) font for use in locations like terminals.
 * @param in_arg is the value you want to set.
 * @return true if setting successfully.
 */
bool lingmo_personal_fonts_set_monospacefont(const char *in_arg);

/**
 * Get the value of key.
 * @return the value of key.
 */
char *lingmo_personal_fonts_get_monospacefont();
#ifdef __cplusplus
}
#endif

#endif













