/*
 * libdpkg - Debian packaging suite library routines
 * c-treewalk.c - test tree walk implementation
 *
 * Copyright © 2013-2016 Guillem Jover <guillem@debian.org>
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
#include <stdio.h>

#include <dpkg/ehandle.h>
#include <dpkg/treewalk.h>

static int
treenode_type(struct treenode *node)
{
	switch (treenode_get_mode(node) & S_IFMT) {
	case S_IFREG:
		return 'f';
	case S_IFDIR:
		return 'd';
	case S_IFLNK:
		return 'l';
	case S_IFIFO:
		return 'p';
	case S_IFBLK:
		return 'b';
	case S_IFCHR:
		return 'c';
	case S_IFSOCK:
		return 's';
	default:
		return '?';
	}
}

static int
treenode_visit_meta(struct treenode *node)
{
	printf("T=%c N=%s V=%s R=%s\n",
	       treenode_type(node), treenode_get_name(node),
	       treenode_get_virtname(node), treenode_get_pathname(node));

	return 0;
}

static int
treenode_visit_virt(struct treenode *node)
{
	printf("%s\n", treenode_get_virtname(node));

	return 0;
}

static int
treenode_visit_path(struct treenode *node)
{
	printf("%s\n", treenode_get_pathname(node));

	return 0;
}

static const char *skipname;

static bool
treenode_skip(struct treenode *node)
{
	const char *absname = treenode_get_pathname(node);

	if (strcmp(absname, skipname) == 0)
		return true;

	return false;
}

static void
test_treewalk_list(const char *rootdir)
{
	const char *visitname = getenv("TREEWALK_VISIT");
	struct treewalk_funcs funcs = { NULL };

	if (visitname == NULL || strcmp(visitname, "meta") == 0)
		funcs.visit = treenode_visit_meta;
	else if (strcmp(visitname, "virt") == 0)
		funcs.visit = treenode_visit_virt;
	else if (strcmp(visitname, "path") == 0)
		funcs.visit = treenode_visit_path;
	else
		ohshit("unknown treewalk visit name");

	skipname = getenv("TREEWALK_SKIP");
	if (skipname)
		funcs.skip = treenode_skip;

	treewalk(rootdir, 0, &funcs);
}

int
main(int argc, char **argv)
{
	const char *rootdir = argv[1];

	setvbuf(stdout, NULL, _IOLBF, 0);

	push_error_context();

	if (rootdir == NULL)
		ohshit("missing treewalk root dir");

	test_treewalk_list(rootdir);

	pop_error_context(ehflag_normaltidy);

	return 0;
}
