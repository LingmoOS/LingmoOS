/*
 * libdpkg - Debian packaging suite library routines
 * strwide.c - wide character string handling routines
 *
 * Copyright © 2004 Changwoo Ryu <cwryu@debian.org>
 * Copyright © 2012-2013 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <stdlib.h>
#ifdef ENABLE_NLS
#include <wchar.h>
#endif

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/string.h>

/**
 * Compute the screen width of a string.
 *
 * @param str The multibyte string.
 *
 * @return The width of the string.
 */
int
str_width(const char *str)
{
#ifdef ENABLE_NLS
	mbstate_t state;
	wchar_t *wcs;
	const char *mbs = str;
	size_t len, res;
	int width;

	len = strlen(str) + 1;
	wcs = m_malloc(sizeof(wcs[0]) * len);

	memset(&state, 0, sizeof(state));

	res = mbsrtowcs(wcs, &mbs, len, &state);
	if (res == (size_t)-1) {
#ifdef DPKG_UNIFORM_ENCODING
		ohshit(_("cannot convert multibyte string '%s' "
		         "to a wide-character string"), str);
#else
		/* Cannot convert, fallback to ASCII method. */
		free(wcs);
		return strlen(str);
#endif
	}

	width = wcswidth(wcs, res);

	free(wcs);

	return width;
#else
	return strlen(str);
#endif
}

/**
 * Generate the crop values for a string given a maximum screen width.
 *
 * This function analyzes the string passed and computes the correct point
 * where to crop the string, returning the amount of string and maximum
 * bytes to use for padding for example.
 *
 * On NLS enabled builds, in addition the string will be cropped on any
 * newline.
 *
 * @param str        The string to crop.
 * @param max_width  The max screen width to use.
 * @param[out] crop  The generated crop values for the string.
 */
void
str_gen_crop(const char *str, int max_width, struct str_crop_info *crop)
{
#ifdef ENABLE_NLS
	mbstate_t state;
	size_t str_bytes;
	int mbs_bytes = 0;
	int mbs_width = 0;

	str_bytes = strlen(str) + 1;
	memset(&state, 0, sizeof(state));

	for (;;) {
		wchar_t wc;
		int wc_width;
		size_t mb_bytes;

		mb_bytes = mbrtowc(&wc, str, str_bytes, &state);
		if (mb_bytes == (size_t)-1 || mb_bytes == (size_t)-2) {
#ifdef DPKG_UNIFORM_ENCODING
			ohshit(_("cannot convert multibyte sequence '%s' "
			         "to a wide character"), str);
#else
			/* Cannot convert, fallback to ASCII method. */
			crop->str_bytes = crop->max_bytes = max_width;
			return;
#endif
		}
		if (mb_bytes == 0)
			break;

		wc_width = wcwidth(wc);
		if (wc_width < 0)
			break;
		if (mbs_width + wc_width > max_width)
			break;

		mbs_width += wc_width;
		mbs_bytes += mb_bytes;
		str_bytes -= mb_bytes;
		str += mb_bytes;
	}

	crop->str_bytes = mbs_bytes;
	crop->max_bytes = mbs_bytes + max_width - mbs_width;
#else
	crop->str_bytes = crop->max_bytes = max_width;
#endif
}
