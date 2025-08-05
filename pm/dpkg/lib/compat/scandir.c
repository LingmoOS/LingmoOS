/*
 * libcompat - system compatibility library
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008, 2009 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "compat.h"

static int
cleanup(DIR *dir, struct dirent **dirlist, int used)
{
	if (dir)
		closedir(dir);

	if (dirlist) {
		int i;

		for (i = 0; i < used; i++)
			free(dirlist[i]);
		free(dirlist);
	}

	return -1;
}

int
scandir(const char *dir, struct dirent ***namelist,
        int (*filter)(const struct dirent *),
        int (*cmp)(const void *, const void *))
{
	DIR *d;
	struct dirent *e, *m, **list;
	int used, avail;

	d = opendir(dir);
	if (!d)
		return -1;

	list = NULL;
	used = avail = 0;

	while ((e = readdir(d)) != NULL) {
		if (filter != NULL && !filter(e))
			continue;

		if (used >= avail - 1) {
			struct dirent **newlist;

			if (avail)
				avail *= 2;
			else
				avail = 20;
			newlist = realloc(list, avail * sizeof(*newlist));
			if (!newlist)
				return cleanup(d, list, used);
			list = newlist;
		}

		m = malloc(sizeof(*m) + strlen(e->d_name));
		if (!m)
			return cleanup(d, list, used);
		*m = *e;
		strcpy(m->d_name, e->d_name);

		list[used] = m;
		used++;
	}

	closedir(d);

	if (list != NULL && cmp != NULL)
		qsort(list, used, sizeof(list[0]), cmp);

	*namelist = list;

	return used;
}
