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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/shm.h>


/* Description of an shm attachment.  */
struct sysvshm_attach
{
  /* Linked list.  */
  struct sysvshm_attach *next;

  /* Map address.  */
  void *addr;

  /* Map size.  */
  size_t size;
};

/* List of attachments.  */
static struct sysvshm_attach *attach_list;

/* A lock to protect the linked list of shared memory attachments.  */
static unsigned int sysvshm_lock = LLL_LOCK_INITIALIZER;


/* Adds a segment attachment.  */
error_t
__sysvshm_add (void *addr, size_t size)
{
  struct sysvshm_attach *shm;

  shm = malloc (sizeof (*shm));
  if (!shm)
    return errno;

  __mutex_lock (&sysvshm_lock);
  shm->addr = addr;
  shm->size = size;
  shm->next = attach_list;
  attach_list = shm;
  __mutex_unlock (&sysvshm_lock);

  return 0;
}

/* Removes a segment attachment.  Returns its size if found, or EINVAL
   otherwise.  */
error_t
__sysvshm_remove (void *addr, size_t *size)
{
  struct sysvshm_attach *shm;
  struct sysvshm_attach **pshm = &attach_list;

  __mutex_lock (&sysvshm_lock);
  shm = attach_list;
  while (shm)
    {
      shm = *pshm;
      if (shm->addr == addr)
	{
	  *pshm = shm->next;
	  *size = shm->size;
	  __mutex_unlock (&sysvshm_lock);
	  free (shm);
	  return 0;
	}
      pshm = &shm->next;
      shm = shm->next;
    }
  __mutex_unlock (&sysvshm_lock);
  return EINVAL;
}
