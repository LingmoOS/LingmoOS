/*
 * libdpkg - Debian packaging suite library routines
 * mustlib.c - ‘must’ library: routines will succeed or longjmp
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2013, 2015 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>

static inline void *
must_alloc(void *ptr)
{
  if (ptr)
    return ptr;

  onerr_abort++;
  ohshite(_("failed to allocate memory"));
}

void *m_malloc(size_t amount) {
  return must_alloc(malloc(amount));
}

void *
m_calloc(size_t nmemb, size_t size)
{
  return must_alloc(calloc(nmemb, size));
}

void *m_realloc(void *r, size_t amount) {
  return must_alloc(realloc(r, amount));
}

char *
m_strdup(const char *str)
{
  return must_alloc(strdup(str));
}

char *
m_strndup(const char *str, size_t n)
{
  return must_alloc(strndup(str, n));
}

int
m_vasprintf(char **strp, const char *fmt, va_list args)
{
  int n;

  n = vasprintf(strp, fmt, args);
  if (n >= 0)
    return n;

  onerr_abort++;
  ohshite(_("failed to allocate memory"));
}

int
m_asprintf(char **strp, const char *fmt, ...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = m_vasprintf(strp, fmt, args);
  va_end(args);

  return n;
}

int
m_dup(int oldfd)
{
  int newfd;

  newfd = dup(oldfd);
  if (newfd >= 0)
    return newfd;

  onerr_abort++;
  ohshite(_("failed to dup for fd %d"), oldfd);
}

void m_dup2(int oldfd, int newfd) {
  const char *const stdstrings[]= { "in", "out", "err" };

  if (dup2(oldfd,newfd) == newfd) return;

  onerr_abort++;
  if (newfd < 3) ohshite(_("failed to dup for std%s"),stdstrings[newfd]);
  ohshite(_("failed to dup for fd %d"),newfd);
}

void
m_pipe(int fds[2])
{
  if (!pipe(fds)) return;
  onerr_abort++;
  ohshite(_("failed to create pipe"));
}

void
m_output(FILE *f, const char *name)
{
  fflush(f);
  if (ferror(f) && errno != EPIPE)
    ohshite(_("error writing to '%s'"), name);
}

void
setcloexec(int fd, const char *fn)
{
  int f;

  f = fcntl(fd, F_GETFD);
  if (f < 0)
    ohshite(_("unable to read filedescriptor flags for %.250s"),fn);
  if (fcntl(fd, F_SETFD, (f | FD_CLOEXEC)) < 0)
    ohshite(_("unable to set close-on-exec flag for %.250s"),fn);
}
