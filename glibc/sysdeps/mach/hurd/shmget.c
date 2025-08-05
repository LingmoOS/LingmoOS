/* Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <hurd/fd.h>

#include "sysvshm.h"

/* Create a new shared memory segment file without linking it into the
   filesystem.  Return the directory and file ports in R_DIR and R_FILE.  */
static error_t
create_shm_file (size_t size, int flags, file_t *r_dir, file_t *r_file)
{
  error_t err;
  file_t dir;
  file_t file;

  flags &= 0777;

  /* Get a port to the directory that will contain the file.  */
  dir = __file_name_lookup (SHM_DIR, 0, 0);
  if (dir == MACH_PORT_NULL)
    return errno;

  /* Create an unnamed file in the directory.  */
  err = __dir_mkfile (dir, O_RDWR, flags, &file);
  if (err)
    {
      __mach_port_deallocate (__mach_task_self (), dir);
      return err;
    }

  err = __file_set_size (file, size);
  if (err)
    {
      __mach_port_deallocate (__mach_task_self (), file);
      __mach_port_deallocate (__mach_task_self (), dir);

      return err;
    }

  *r_dir = dir;
  *r_file = file;

  return 0;
}


/* Open the shared memory segment *R_KEY and return a file descriptor
   to it in R_FD.  If KEY is IPC_PRIVATE, use a private key and return
   it in R_KEY.  */
static error_t
get_exclusive (int shmflags, size_t size, key_t *r_key, int *r_fd)
{
  error_t err;
  file_t dir;
  file_t file;
  char filename[SHM_NAMEMAX];
  key_t key = *r_key;
  bool is_private;

  /* Create the shared memory segment.  */
  err = create_shm_file (size, shmflags, &dir, &file);
  if (err)
    return err;

  if (key == IPC_PRIVATE)
    {
      is_private = true;
      key = SHM_PRIV_KEY_START;

      /* Try to link the shared memory segment into the filesystem
	 (exclusively).  Private segments have negative keys.  */
      do
	{
	  sprintf (filename, SHM_NAMEPRI, key);
	  err = __dir_link (dir, file, filename, 1);
	  if (!err)
	    {
	      /* We are done.  */
	      *r_key = key;
	      break;
	    }
	  else if (err == EEXIST)
	    {
	      /* Check if we ran out of keys.  If not, try again with new
		 key.  */
	      if (key == SHM_PRIV_KEY_END)
		err = ENOSPC;
	      else
		err = 0;

	      key--;
	    }
	}
      while (!err);
    }
  else
    {
      /* Try to link the shared memory segment into the filesystem
	 (exclusively) under the given key.  */
      sprintf (filename, SHM_NAMEPRI, key);
      err = __dir_link (dir, file, filename, 1);
    }

  __mach_port_deallocate (__mach_task_self (), dir);

  if (!err)
    {
      int fd;

      /* Get a file descriptor for that port.  */
      fd = _hurd_intern_fd (file, O_RDWR, 1); /* dealloc on error */
      if (fd < 0)
	err = errno;
      else
	*r_fd = fd;
    }

  return err;
}


/* Open the shared memory segment KEY (creating it if it doesn't yet
   exist) and return a file descriptor to it in R_FD.  */
static error_t
get_shared (int shmflags, size_t size, key_t key, int *r_fd)
{
  error_t err = 0;
  char filename[sizeof (SHM_DIR) - 1 + SHM_NAMEMAX];
  int fd = -1;
  sprintf (filename, SHM_DIR SHM_NAMEPRI, key);

  do
    {
      fd = __open (filename, O_NORW, shmflags & 0777);

      if (fd < 0 && errno != ENOENT)
	/* We give up.  */
	return errno;
      else if (fd >= 0)
	{
	  int res;
	  struct stat statbuf;

	  /* Check the size (we only need to do this if we did not
	     create the shared memory segment file ourselves).  */
	  res = __fstat (fd, &statbuf);
	  if (res < 0)
	    {
	      err = errno;
	      __close (fd);
	      return err;
	    }

	  if (statbuf.st_size < size)
	    {
	      __close (fd);
	      return EINVAL;
	    }
	}
      else
	{
	  /* The memory segment doesn't exist.  */
	  if (shmflags & IPC_CREAT)
	    {
	      /* Try to create it exclusively.  */
	      err = get_exclusive (shmflags, size, &key, &fd);
	      if (err == EEXIST)
		/* If somebody created it in the meanwhile, just try again.  */
		err = 0;
	    }
	  else
	    err = ENOENT;
	}
    }
  while (fd < 0 && !err);

  if (!err)
    *r_fd = fd;
  else
    *r_fd = -1;

  return err;
}

/* Return an identifier for an shared memory segment of at least size
   SIZE which is associated with KEY.  */
int
__shmget (key_t key, size_t size, int shmflags)
{
  error_t err;
  int fd;

  if (key == IPC_PRIVATE || shmflags & IPC_EXCL)
    /* An exclusive shared memory segment must be created.  */
    err = get_exclusive (shmflags, size, &key, &fd);
  else
    err = get_shared (shmflags, size, key, &fd);

  if (err)
    {
      errno = err;
      return -1;
    }

  /* From here, we can't fail.  That's important, as otherwise we
     would need to unlink the file if we created it (in that case, the
     code above would have to be changed to pass a "created" flag down
     to the caller).  */

  __close (fd);

  return key;
}

weak_alias(__shmget, shmget)
