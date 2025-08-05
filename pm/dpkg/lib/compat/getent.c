/*
 * libcompat - system compatibility library
 *
 * Copyright © 2025 Guillem Jover <guillem@debian.org>
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

#include <sys/types.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#include "compat.h"

static char *
parse_field(char **start, int delim)
{
	char *field;
	char *next;

	field = *start;
	if (*field == '\0')
		return NULL;

	next = strchrnul(field, delim);
	if (next[0] == delim) {
		next[0] = '\0';
		next++;
	}
	*start = next;

	return field;
}

static char **
alloc_subfields(size_t len, char ***list, size_t *list_len)
{
	char **elems;

	elems = realloc(*list, sizeof(char *) * (len + 1));
	if (elems == NULL)
		return NULL;
	*list_len = len;
	*list = elems;

	return elems;
}

static char **
parse_subfields(char **start, int delim, char ***list, size_t *list_len)
{
	char *field;
	char **elems;
	size_t subfields = 1;
	size_t i;

	for (field = *start; *field; field++) {
		if (*field == delim)
			subfields++;
	}

	if (*list == NULL || *list_len < subfields) {
		elems = alloc_subfields(subfields, list, list_len);
		if (elems == NULL)
			return NULL;
	} else {
		elems = *list;
	}

	for (i = 0; i < subfields; i++)
		elems[i] = parse_field(start, delim);
	elems[subfields] = NULL;

	return elems;
}

static intmax_t
parse_intmax(char *str)
{
	intmax_t val;
	char *endp;

	errno = 0;
	val = strtoimax(str, &endp, 10);
	if (str == endp || *endp)
		return -1;
	if (val < 0 || errno == ERANGE)
		return -1;

	return val;
}

static intmax_t
parse_field_id(char **start, int delim)
{
	char *field;
	intmax_t id = 0;

	field = parse_field(start, delim);
	if (field == NULL)
		return 0;
	id = parse_intmax(field);
	if (id < 0)
		return 0;

	return id;
}

#define PARSE_FIELD_STR(field) \
	field = parse_field(&cur, ':'); \
	if (field == NULL) \
		continue;

#define PARSE_FIELD_ID(field) \
	field = parse_field_id(&cur, ':');

static char *ent_empty_str;

struct passwd *
fgetpwent(FILE *fp)
{
	static char *line = NULL;
	static size_t line_len = 0;
	static struct passwd pw;
	char *cur;

	while (1) {
		ssize_t len;

		len = getline(&line, &line_len, fp);
		if (len < 0)
			return NULL;
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		cur = line;

		PARSE_FIELD_STR(pw.pw_name);

		/* Special case NIS compat entries. */
		if (pw.pw_name[0] == '+' || pw.pw_name[0] == '-') {
			if (ent_empty_str == NULL) {
				ent_empty_str = strdup("");
				if (ent_empty_str == NULL)
					return NULL;
			}

			pw.pw_passwd = ent_empty_str;
			pw.pw_uid = 0;
			pw.pw_gid = 0;
			pw.pw_gecos = ent_empty_str;
			pw.pw_dir = ent_empty_str;
			pw.pw_shell = ent_empty_str;
		} else {
			PARSE_FIELD_STR(pw.pw_passwd);
			PARSE_FIELD_ID(pw.pw_uid);
			PARSE_FIELD_ID(pw.pw_gid);
			PARSE_FIELD_STR(pw.pw_gecos);
			PARSE_FIELD_STR(pw.pw_dir);
			PARSE_FIELD_STR(pw.pw_shell);
		}

		return &pw;
	}

	return NULL;
}

struct group *
fgetgrent(FILE *fp)
{
	static char *line = NULL;
	static size_t line_len = 0;
	static struct group gr;
	static char **gr_mem = NULL;
	static size_t gr_mem_len = 0;
	char *cur;

	while (1) {
		ssize_t len;

		len = getline(&line, &line_len, fp);
		if (len < 0)
			return NULL;
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		cur = line;

		PARSE_FIELD_STR(gr.gr_name);

		/* Special case NIS compat entries. */
		if (gr.gr_name[0] == '+' || gr.gr_name[0] == '-') {
			if (ent_empty_str == NULL)
				ent_empty_str = strdup("");

			gr.gr_passwd = ent_empty_str;
			gr.gr_gid = 0;
			gr.gr_mem = alloc_subfields(0, &gr_mem, &gr_mem_len);
			if (gr.gr_mem == NULL)
				return NULL;
			gr.gr_mem[0] = NULL;
		} else {
			PARSE_FIELD_STR(gr.gr_passwd);
			PARSE_FIELD_ID(gr.gr_gid);
			gr.gr_mem = parse_subfields(&cur, ',', &gr_mem, &gr_mem_len);
			if (gr.gr_mem == NULL)
				return NULL;
		}

		return &gr;
	}

	return NULL;
}
