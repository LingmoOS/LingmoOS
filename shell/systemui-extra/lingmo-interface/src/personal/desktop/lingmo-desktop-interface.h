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


#ifndef __LINGMODESKTOPINTERFACE_H__
#define __LINGMODESKTOPINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusDesktop(void);

//Delete the DBus Communication
bool DeInitDBusDesktop(void);

/*
 * The font description used for the icons on the desktop.
 */
void lingmo_personal_fonts_font(const char *in_arg);

/*
 * Home icon visible on desktop.
 * If this is set to true, an icon linking to the home folder
 * will be put on the desktop.
 */
void lingmo_personal_fonts_homeiconvisible(const bool in_arg);

/*
 * Computer icon visible on desktop.
 * If this is set to true, an icon linking to the computer location
 * will be put on the desktop.
 */
void lingmo_personal_fonts_computericonvisible(const bool in_arg);

/*
 * Trash icon visible on desktop.
 * If this is set to true, an icon linking to the trash
 * will be put on the desktop.
 */
void lingmo_personal_fonts_trashiconvisible(const bool in_arg);

/*
 * Show mounted volumes on the desktop.
 * If this is set to true, icons linking to mounted volumes
 * will be put on the desktop.
 */
void lingmo_personal_fonts_volumesvisible(const bool in_arg);

/*
 * Network Servers icon visible on the desktop.
 * If this is set to true, an icon linking to the Network Servers view
 * will be put on the desktop.
 */
void lingmo_personal_fonts_networkiconvisible(const bool in_arg);

/*
 * Desktop computer icon name.
 */
void lingmo_personal_fonts_computericonname(const char *in_arg);

/*
 * Desktop home icon name.
 */
void lingmo_personal_fonts_homeiconname(const char *in_arg);

/*
 * Desktop trash icon name.
 */
void lingmo_personal_fonts_trashiconname(const char *in_arg);

/*
 * Network servers icon name.
 */
void lingmo_personal_fonts_networkiconname(const char *in_arg);

/*
 * Text Ellipsis Limit.
 */
void lingmo_personal_fonts_textellipsislimit(const int in_arg);

#ifdef __cplusplus
}
#endif

#endif













