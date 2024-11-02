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


#ifndef LINGMOVERSIONINTERFACE_H
#define LINGMOVERSIONINTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    char *subVersionName;
    int credibility;//可信度0-100；
}subVersionCredibility;

/*
 * Gets the current subversion list.
 * return a struct pointer.
 */
subVersionCredibility *lingmo_os_sysinfo_getchildrroup();

/*
 * Verify whether it is a given subversion.
 * @version is the name of subversion.
 * return the credibility of subversion.
 */
int lingmo_os_sysinfo_verifysubversion(const char *version);

#ifdef __cplusplus
}
#endif

#endif // LINGMOVERSIONINTERFACE_H
