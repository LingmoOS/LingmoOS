/* SysV shmat for Hurd.
   Copyright (C) 2005-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <utime.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sysvshm.h"

/* Attach the shared memory segment associated with SHMID to the data
   segment of the calling process.  SHMADDR and SHMFLG determine how
   and where the segment is attached.  */
void *
__shmat (int shmid, const void *shmaddr, int shmflg)
{
  error_t err;
  char filename[sizeof (SHM_DIR) - 1 + SHM_NAMEMAX];
  int fd;
  void *addr;
  struct stat statbuf;
  int res;

  sprintf (filename, SHM_DIR SHM_NAMEPRI, shmid);
  fd = __open (filename, (shmflg & SHM_RDONLY) ? O_RDONLY : O_RDWR);
  if (fd < 0)
    {
      if (errno == ENOENT)
	errno = EINVAL;
      return (void *) -1;
    }

  res = __fstat (fd, &statbuf);
  if (res < 0)
    {
      err = errno;
      __close (fd);
      errno = err;
      return (void *) -1;
    }

  addr = __mmap ((void *) shmaddr, statbuf.st_size,
		 PROT_READ | ((shmflg & SHM_RDONLY) ? 0 : PROT_WRITE),
		 MAP_SHARED, fd, 0);
  __close (fd);
  if (addr == MAP_FAILED)
    return (void *) -1;

  err = __sysvshm_add (addr, statbuf.st_size);
  if (err)
    {
      __munmap (addr, statbuf.st_size);
      errno = err;
      return (void *) -1;
    }

  return addr;
}

weak_alias(__shmat, shmat)
