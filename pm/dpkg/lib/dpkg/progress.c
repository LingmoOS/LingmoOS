/*
 * libdpkg - Debian packaging suite library routines
 * progress.c - generic progress reporting
 *
 * Copyright © 2009 Romain Francoise <rfrancoise@debian.org>
 * Copyright © 2009 Guillem Jover <guillem@debian.org>
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

#include <unistd.h>
#include <stdio.h>

#include <dpkg/i18n.h>

#include "progress.h"

void
progress_init(struct progress *progress, const char *text, int max)
{
	progress->text = text;
	progress->max = max;
	progress->cur = 0;
	progress->last_percent = 0;

	progress->on_tty = isatty(1);

	fputs(text, stdout);
	if (progress->on_tty)
		putchar('\r');
}

void
progress_step(struct progress *progress)
{
	int cur_percent;

	if (!progress->on_tty)
		return;

	progress->cur++;

	cur_percent = (progress->cur * 100) / progress->max;
	if (cur_percent <= progress->last_percent)
		return;
	if (cur_percent % 5)
		return;

	progress->last_percent = cur_percent;

	fputs(progress->text, stdout);
	/* TRANSLATORS: This is part of the progress output, it is a decimal
	 * percentage. */
	printf(_("%d%%"), cur_percent);
	putchar('\r');
}

void
progress_done(struct progress *progress)
{
	if (progress->on_tty)
		fputs(progress->text, stdout);
}
