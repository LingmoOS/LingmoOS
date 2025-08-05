/*
 * dpkg - main program for package management
 * filters.c - filtering routines for excluding bits of packages
 *
 * Copyright © 2007, 2008 Tollef Fog Heen <tfheen@err.no>
 * Copyright © 2008, 2010, 2012-2014 Guillem Jover <guillem@debian.org>
 * Copyright © 2010 Canonical Ltd.
 *   written by Martin Pitt <martin.pitt@canonical.com>
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

#include <fnmatch.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-fsys.h>

#include "main.h"
#include "filters.h"

struct filter_node {
	struct filter_node *next;
	char *pattern;
	bool include;
};

static struct filter_node *filter_head = NULL;
static struct filter_node **filter_tail = &filter_head;

void
filter_add(const char *pattern, bool include)
{
	struct filter_node *filter;

	debug(dbg_general, "adding %s filter for '%s'",
	      include ? "include" : "exclude", pattern);

	filter = m_malloc(sizeof(*filter));
	filter->pattern = m_strdup(pattern);
	filter->include = include;
	filter->next = NULL;

	*filter_tail = filter;
	filter_tail = &filter->next;
}

bool
filter_should_skip(struct tar_entry *ti)
{
	struct filter_node *f;
	bool skip = false;

	if (!filter_head)
		return false;

	/* Last match wins. */
	for (f = filter_head; f != NULL; f = f->next) {
		debug(dbg_eachfile, "filter comparing '%s' and '%s'",
		      &ti->name[1], f->pattern);

		if (fnmatch(f->pattern, &ti->name[1], 0) == 0) {
			if (f->include) {
				skip = false;
				debug(dbg_eachfile, "filter including %s",
				      ti->name);
			} else {
				skip = true;
				debug(dbg_eachfile, "filter removing %s",
				      ti->name);
			}
		}
	}

	/* We need to keep directories (or symlinks to directories) if a
	 * glob excludes them, but a more specific include glob brings back
	 * files; XXX the current implementation will probably include more
	 * directories than necessary, but better err on the side of caution
	 * than failing with “no such file or directory” (which would leave
	 * the package in a very bad state). */
	if (skip && (ti->type == TAR_FILETYPE_DIR ||
	             ti->type == TAR_FILETYPE_SYMLINK)) {
		debug(dbg_eachfile,
		      "filter seeing if '%s' needs to be reincluded",
		      &ti->name[1]);

		for (f = filter_head; f != NULL; f = f->next) {
			const char *wildcard;
			int path_len;

			if (!f->include)
				continue;

			/* Calculate the offset of the first wildcard
			 * character in the pattern. */
			wildcard = strpbrk(f->pattern, "*?[\\");
			if (wildcard)
				path_len = wildcard - f->pattern;
			else
				path_len = strlen(f->pattern);

			/* Ignore any trailing slash for the comparison. */
			while (path_len && f->pattern[path_len - 1] == '/')
				path_len--;

			debug(dbg_eachfiledetail,
			      "filter subpattern '%.*s'", path_len, f->pattern);

			if (strncmp(&ti->name[1], f->pattern, path_len) == 0) {
				debug(dbg_eachfile, "filter reincluding %s",
				      ti->name);
				return false;
			}
		}
	}

	return skip;
}
