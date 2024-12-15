// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* configured by cmake, do not edit */

//#define ALBUM_VERSION ""
//#define VERSION        ""
/* #undef USE_DXCB */
/* #undef DMR_DEBUG */

/* only defined when build with flatpak */
/* #undef DTK_DMAN_PORTAL */

/*#define RADIUS 0*/
/*#define RADIUS_MV 18*/

#define USE_DTK
/* #undef NO_DTK */


#define TRANSFILEPATH "/usr/local/share/drawboard/translations"
#define TOOLPLUGINPATH "/usr/local/lib/x86_64-linux-gnu/drawboard/plugins/tools"
#define TOOLPLUGINTRANSPATH "/usr/local/share/drawboard/translations/plugins/tools"
#endif  /* __CONFIG_H__ */
