/*
 * dpkg - main program for package management
 * security-mac.h - MAC-based security support
 *
 * Copyright © 2015 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DPKG_SECURITY_MAC_H
#define DPKG_SECURITY_MAC_H

#include <sys/stat.h>

void dpkg_selabel_load(void);
void dpkg_selabel_set_context(const char *matchpath, const char *path, mode_t mode);
void dpkg_selabel_close(void);

#endif /* DPKG_SECURITY_MAC_H */
