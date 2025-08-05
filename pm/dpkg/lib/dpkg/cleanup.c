/*
 * libdpkg - Debian packaging suite library routines
 * cleanup.c - cleanup functions, used when we need to unwind
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

#include <dpkg/dpkg.h>

void
cu_closepipe(int argc, void **argv)
{
	int *p1 = (int *)argv[0];

	close(p1[0]);
	close(p1[1]);
}

void
cu_closestream(int argc, void **argv)
{
	FILE *f = (FILE *)(argv[0]);

	fclose(f);
}

void
cu_closedir(int argc, void **argv)
{
	DIR *d = (DIR *)(argv[0]);

	closedir(d);
}

void
cu_closefd(int argc, void **argv)
{
	int ip = *(int *)argv[0];

	close(ip);
}

void
cu_filename(int argc, void **argv)
{
	const char *filename = argv[0];

	(void)unlink(filename);
}
