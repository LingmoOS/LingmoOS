/*
 * libdpkg - Debian packaging suite library routines
 * string.c - string handling routines
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2015 Guillem Jover <guillem@debian.org>
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

#include <dpkg/c-ctype.h>
#include <dpkg/string.h>
#include <dpkg/dpkg.h>

char *
str_concat(char *dst, ...)
{
	va_list args;
	const char *src;

	va_start(args, dst);
	while ((src = va_arg(args, const char *))) {
		size_t len;

		len = strlen(src);
		memcpy(dst, src, len);
		dst += len;
	}
	va_end(args);
	*dst = '\0';

	return dst;
}

/**
 * Match the end of a string.
 *
 * @param str The string.
 * @param end The end to match in str.
 *
 * @return Whether the string was matched at the end.
 */
bool
str_match_end(const char *str, const char *end)
{
	size_t str_len = strlen(str);
	size_t end_len = strlen(end);
	const char *str_end = str + str_len - end_len;

	if (str_len >= end_len && strcmp(str_end, end) == 0)
		return true;
	else
		return false;
}

/**
 * Print formatted output to an allocated string.
 *
 * @param fmt The format string.
 * @param ... The format arguments.
 *
 * @return The new allocated formatted output string (never NULL).
 */
char *
str_fmt(const char *fmt, ...)
{
	va_list args;
	char *str;

	va_start(args, fmt);
	m_vasprintf(&str, fmt, args);
	va_end(args);

	return str;
}

/**
 * Escape format characters from a string.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param n The size of the destination buffer.
 *
 * @return The end of the destination string.
 */
char *
str_escape_fmt(char *dst, const char *src, size_t n)
{
	char *d = dst;
	const char *s = src;

	if (n == 0)
		return d;

	while (*s) {
		if (*s == '%') {
			if (n-- <= 2)
				break;
			*d++ = '%';
		}
		if (n-- <= 1)
			break;
		*d++ = *s++;
	}

	*d = '\0';

	return d;
}

/**
 * Quote shell metacharacters in a string.
 *
 * This function allows passing strings to commands without splitting the
 * arguments, like in system(3)
 *
 * @param src The source string to escape.
 *
 * @return The new allocated string (never NULL).
 */
char *
str_quote_meta(const char *src)
{
	char *new_dst, *dst;

	new_dst = dst = m_malloc(strlen(src) * 2);

	while (*src) {
		if (!c_isdigit(*src) && !c_isalpha(*src))
			*dst++ = '\\';

		*dst++ = *src++;
	}

	*dst = '\0';

	return new_dst;
}

/**
 * Check and strip possible surrounding quotes in string.
 *
 * @param str The string to act on.
 *
 * @return A pointer to str or NULL if the quotes were unbalanced.
 */
char *
str_strip_quotes(char *str)
{
	if (str[0] == '"' || str[0] == '\'') {
		size_t str_len = strlen(str);

		if (str[0] != str[str_len - 1])
			return NULL;

		/* Remove surrounding quotes. */
		str[str_len - 1] = '\0';
		str++;
	}

	return str;
}

/**
 * Trim possible ending spaces in string.
 *
 * @param str The string to act on.
 * @param str_end The end of the string to act on.
 *
 * @return A pointer to the end of the trimmed string.
 */
char *
str_rtrim_spaces(const char *str, char *str_end)
{
	while (str_end > str && c_isspace(str_end[-1]))
		str_end--;
	if (str_end >= str)
		*str_end = '\0';
	return str_end;
}
