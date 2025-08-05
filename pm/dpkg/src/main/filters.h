/*
 * dpkg - main program for package management
 * filters.h - external definitions for filter handling
 *
 * Copyright © 2007, 2008 Tollef Fog Heen <tfheen@err.no>
 * Copyright © 2008, 2010 Guillem Jover <guillem@debian.org>
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

#ifndef DPKG_FILTERS_H
#define DPKG_FILTERS_H

#include <stdbool.h>

#include <dpkg/macros.h>
#include <dpkg/tarfn.h>

DPKG_BEGIN_DECLS

void filter_add(const char *glob, bool include);
bool filter_should_skip(struct tar_entry *ti);

DPKG_END_DECLS

#endif
