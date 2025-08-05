/*
 * libdpkg - Debian packaging suite library routines
 * nfmalloc.c - non-freeing malloc, used for in-core database
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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
#include <obstack.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#define obstack_chunk_alloc m_malloc
#define obstack_chunk_free free

static struct obstack db_obs;
static bool dbobs_init = false;

/* We use lots of mem, so use a large chunk. */
#define CHUNK_SIZE 8192

#define OBSTACK_INIT do { if (!dbobs_init) { nfobstack_init(); } } while (0)

static void nfobstack_init(void) {
  obstack_init(&db_obs);
  dbobs_init = true;
  obstack_chunk_size(&db_obs) = CHUNK_SIZE;
}

void *
nfmalloc(size_t size)
{
  OBSTACK_INIT;
  /* cppcheck-suppress[nullPointerArithmetic]:
   * False positive, imported module. */
  return obstack_alloc(&db_obs, size);
}

char *nfstrsave(const char *string) {
  OBSTACK_INIT;
  /* cppcheck-suppress[nullPointerArithmetic]:
   * False positive, imported module. */
  return obstack_copy0 (&db_obs, string, strlen(string));
}

char *
nfstrnsave(const char *string, size_t size)
{
  OBSTACK_INIT;
  /* cppcheck-suppress[nullPointerArithmetic]:
   * False positive, imported module. */
  return obstack_copy0(&db_obs, string, size);
}

void nffreeall(void) {
  if (dbobs_init) {
    /* cppcheck-suppress[nullPointerArithmetic,pointerLessThanZero]:
     * False positive, imported module. */
    obstack_free(&db_obs, NULL);
    dbobs_init = false;
  }
}
