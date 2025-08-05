/*
 * libdpkg - Debian packaging suite library routines
 * color.c - color support
 *
 * Copyright © 2015-2016 Guillem Jover <guillem@debian.org>
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

#include <config.h>
#include <compat.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dpkg/macros.h>
#include <dpkg/color.h>

static enum color_mode color_mode = COLOR_MODE_UNKNOWN;
static bool use_color = false;

bool
color_set_mode(const char *mode)
{
	if (strcmp(mode, "auto") == 0) {
		color_mode = COLOR_MODE_AUTO;
		use_color = isatty(STDOUT_FILENO);
	} else if (strcmp(mode, "always") == 0) {
		color_mode = COLOR_MODE_ALWAYS;
		use_color = true;
	} else {
		color_mode = COLOR_MODE_NEVER;
		use_color = false;
	}

	return use_color;
}

static bool
color_enabled(void)
{
	const char *mode;

	if (color_mode != COLOR_MODE_UNKNOWN)
		return use_color;

	mode = getenv("DPKG_COLORS");
	if (mode == NULL)
		mode = "auto";

	return color_set_mode(mode);
}

const char *
color_get(const char *color)
{
	if (!color_enabled())
		return "";

	return color;
}
