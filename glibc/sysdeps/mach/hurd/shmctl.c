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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sysvshm.h"

/* Provide operations to control over shared memory segments.  */
int
__shmctl (int id, int cmd, struct shmid_ds *buf)
{
  error_t err = 0;
  int fd;
  int res;
  char filename[sizeof (SHM_DIR) - 1 + SHM_NAMEMAX];
  struct stat statbuf;

  sprintf (filename, SHM_DIR SHM_NAMEPRI, id);
  /* SysV requires read access for IPC_STAT.  */
  fd = __open (filename, O_NORW);
  if (fd < 0)
    {
      if (errno == ENOENT)
	errno = EINVAL;
      return -1;
    }

  res = __fstat (fd, &statbuf);
  if (res < 0)
    {
      err = errno;
      __close (fd);
      errno = err;
      return -1;
    }

  switch (cmd)
    {
    case IPC_STAT:

      buf->shm_perm.__key = id;
      buf->shm_perm.uid = statbuf.st_uid;
      buf->shm_perm.gid = statbuf.st_gid;

      /* We do not support the creator.  */
      buf->shm_perm.cuid = statbuf.st_uid;
      buf->shm_perm.cgid = statbuf.st_gid;

      /* We just want the protection bits.  */
      buf->shm_perm.mode = statbuf.st_mode & 0777;
      /* Hopeless.  We do not support a sequence number.  */
      buf->shm_perm.__seq = statbuf.st_ino;
      buf->shm_segsz = statbuf.st_size;

      /* Hopeless.  We do not support any of these.  */
      buf->shm_atime = statbuf.st_atime;
      buf->shm_dtime = statbuf.st_mtime;
      /* Well, this comes at least close.  */
      buf->shm_ctime = statbuf.st_ctime;

      /* We do not support the PID.  */
      buf->shm_cpid = 0;
      buf->shm_lpid = 0;

      if (statbuf.st_mode & S_IMMAP0)
        buf->shm_nattch = 0;
      else
        /* 42 is the answer.  Of course this is bogus, but for most
	   applications, this should be fine.  */
        buf->shm_nattch = 42;

      break;

    case IPC_SET:
      if (statbuf.st_uid != buf->shm_perm.uid
	  || statbuf.st_gid != buf->shm_perm.gid)
	{
	  res = __fchown (fd,
			  (statbuf.st_uid != buf->shm_perm.uid)
			  ? buf->shm_perm.uid : -1,
			  (statbuf.st_gid != buf->shm_perm.gid)
			  ? buf->shm_perm.gid : -1);
	  if (res < 0)
	    err = errno;
	}

      if (!err && statbuf.st_mode & 0777 != buf->shm_perm.mode & 0777)
	{
	  res = __fchmod (fd, (statbuf.st_mode & ~0777)
			  | (buf->shm_perm.mode & 0777));
	  if (res < 0)
	    err = errno;
	}
      break;

    case IPC_RMID:
      res = __unlink (filename);
      /* FIXME: Check error (mapping ENOENT to EINVAL).  */
      break;

    default:
      err = EINVAL;
    }

  __close (fd);
  errno = err;
  return err ? -1 : 0;
}

weak_alias(__shmctl, shmctl)
