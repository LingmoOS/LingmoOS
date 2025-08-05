/*
 * libdpkg - Debian packaging suite library routines
 * log.c - logging related functions
 *
 * Copyright © 2005 Scott James Remnant <scott@netsplit.com>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/fdio.h>

const char *log_file = NULL;

void
log_message(const char *fmt, ...)
{
	static struct varbuf log;
	static int logfd = -1;
	char time_str[20];
	time_t now;
	struct tm tm;
	va_list args;

	if (!log_file)
		return;

	if (logfd < 0) {
		logfd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (logfd < 0) {
			notice(_("could not open log '%s': %s"),
			       log_file, strerror(errno));
			log_file = NULL;
			return;
		}
		setcloexec(logfd, log_file);
	}

	time(&now);
	if (localtime_r(&now, &tm) == NULL) {
		notice(_("cannot get local time to log into '%s': %s"),
		       log_file, strerror(errno));
		return;
	}
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm);

	va_start(args, fmt);
	varbuf_set_str(&log, time_str);
	varbuf_add_char(&log, ' ');
	varbuf_add_vfmt(&log, fmt, args);
	varbuf_add_char(&log, '\n');
	va_end(args);

	if (fd_write(logfd, log.buf, log.used) < 0)
		notice(_("cannot write to log file '%s': %s"),
		       log_file, strerror(errno));
}

struct pipef {
	struct pipef *next;
	int fd;
};

static struct pipef *status_pipes = NULL;

void
statusfd_add(int fd)
{
	struct pipef *pipe_new;

	setcloexec(fd, _("<package status and progress file descriptor>"));

	pipe_new = nfmalloc(sizeof(*pipe_new));
	pipe_new->fd = fd;
	pipe_new->next = status_pipes;
	status_pipes = pipe_new;
}

void
statusfd_send(const char *fmt, ...)
{
	static struct varbuf vb;
	struct pipef *pipef;
	va_list args;

	if (!status_pipes)
		return;

	va_start(args, fmt);
	varbuf_set_vfmt(&vb, fmt, args);
	/* Sanitize string to not include new lines, as front-ends should be
	 * doing their own word-wrapping. */
	varbuf_map_char(&vb, '\n', ' ');
	varbuf_add_char(&vb, '\n');
	va_end(args);

	for (pipef = status_pipes; pipef; pipef = pipef->next) {
		if (fd_write(pipef->fd, vb.buf, vb.used) < 0)
			ohshite(_("unable to write to status fd %d"),
			        pipef->fd);
	}
}
