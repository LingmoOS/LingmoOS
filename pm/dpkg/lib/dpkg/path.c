/*
 * libdpkg - Debian packaging suite library routines
 * path.c - path handling functions
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <dpkg/dpkg.h>
#include <dpkg/string.h>
#include <dpkg/strvec.h>
#include <dpkg/path.h>

/**
 * Perform lexical canonicalization of a pathname.
 *
 * @param path The pathname to canonicalize.
 *
 * @return The canonicalized pathname.
 */
char *
path_canonicalize(const char *path)
{
	struct strvec *osv, *nsv;
	size_t o;
	char *newpath;
	bool is_absolute = path[0] == '/';

	osv = strvec_split(path, '/', STRVEC_SPLIT_SKIP_DUP_SEP);
	nsv = strvec_new(0);

	for (o = 0; o < osv->used; o++) {
		if (strcmp(osv->vec[o], "..") == 0) {
			/* Go up. */
			if (is_absolute && nsv->used == 1)
				continue;

			strvec_drop(nsv);
		} else if ((strcmp(osv->vec[o], "") == 0 && o > 0) ||
		           strcmp(osv->vec[o], ".") == 0) {
			/* Skip part. */
			continue;
		} else {
			/* Move the component over. */
			strvec_push(nsv, osv->vec[o]);
			osv->vec[o] = NULL;
		}
	}

	/* Make sure we have something to join on. */
	if (is_absolute && nsv->used == 1)
		strvec_push(nsv, m_strdup(""));

	newpath = strvec_join(nsv, '/');
	strvec_free(nsv);
	strvec_free(osv);

	return newpath;
}

/**
 * Trim ‘/’ and ‘/.’ from the end of a pathname.
 *
 * The given string will get NUL-terminatd.
 *
 * @param path The pathname to trim.
 *
 * @return The size of the trimmed pathname.
 */
size_t
path_trim_slash_slashdot(char *path)
{
	char *end;

	if (str_is_unset(path))
		return 0;

	for (end = path + strlen(path) - 1; end - path >= 1; end--) {
		if (*end == '/' || (*(end - 1) == '/' && *end == '.'))
			*end = '\0';
		else
			break;
	}

	return end - path + 1;
}

/**
 * Skip ‘/’ and ‘./’ from the beginning of a pathname.
 *
 * @param path The pathname to skip.
 *
 * @return The new beginning of the pathname.
 */
const char *
path_skip_slash_dotslash(const char *path)
{
	while (path[0] == '/' || (path[0] == '.' && path[1] == '/'))
		path++;

	return path;
}

/**
 * Return the last component of a pathname.
 *
 * @param path The pathname to get the base name from.
 *
 * @return A pointer to the last component inside pathname.
 */
const char *
path_basename(const char *path)
{
	const char *last_slash;

	last_slash = strrchr(path, '/');
	if (last_slash == NULL)
		return path;
	else
		return last_slash + 1;
}

/**
 * Create a template for a temporary pathname.
 *
 * @param suffix The suffix to use for the template string.
 *
 * @return An allocated string with the created template.
 */
char *
path_make_temp_template(const char *suffix)
{
	const char *tmpdir;

	tmpdir = getenv("TMPDIR");
	if (!tmpdir)
		tmpdir = P_tmpdir;

	return str_fmt("%s/%s.XXXXXX", tmpdir, suffix);
}

/**
 * Escape characters in a pathname for safe locale printing.
 *
 * We need to quote paths so that they do not cause problems when printing
 * them, for example with snprintf(3) which does not work if the format
 * string contains %s and an argument has invalid characters for the
 * current locale, it will then return -1.
 *
 * To simplify things, we just escape all 8 bit characters, instead of
 * just invalid characters.
 *
 * @param dst The escaped destination string.
 * @param src The source string to escape.
 * @param n The size of the destination buffer.
 *
 * @return The destination string.
 */
char *
path_quote_filename(char *dst, const char *src, size_t n)
{
	char *ret = dst;
	ssize_t size = (ssize_t)n;

	if (size == 0)
		return ret;

	while (*src) {
		if (*src == '\\') {
			size -= 2;
			if (size <= 0)
				break;

			*dst++ = '\\';
			*dst++ = '\\';
			src++;
		} else if (((*src) & 0x80) == '\0') {
			size--;
			if (size <= 0)
				break;

			*dst++ = *src++;
		} else {
			size -= 4;
			if (size <= 0)
				break;

			snprintf(dst, 5, "\\%03o",
			        *(const unsigned char *)src);
			dst += 4;
			src++;
		}
	}

	*dst = '\0';

	return ret;
}
