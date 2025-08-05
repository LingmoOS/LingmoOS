/* Copyright (C) 2001-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/socket.h>
#include <sysdep-cancel.h>

static unsigned
contains_uid (unsigned int n, __uid_t uids[n], __uid_t uid)
{
  unsigned i;

  for (i = 0; i < n; i++)
    if (uids[i] == uid)
      return 1;
  return 0;
}

static unsigned
contains_gid (unsigned int n, __gid_t gids[n], __gid_t gid)
{
  unsigned i;

  for (i = 0; i < n; i++)
    if (gids[i] == gid)
      return 1;
  return 0;
}

/* Check the passed credentials.  */
static error_t
check_auth (mach_port_t rendezvous,
		    __pid_t pid,
		    __uid_t uid, __uid_t euid,
		    __gid_t gid,
		    int ngroups, __gid_t groups[ngroups])
{
  error_t err;
  mach_msg_type_number_t neuids = CMGROUP_MAX, nauids = CMGROUP_MAX;
  mach_msg_type_number_t negids = CMGROUP_MAX, nagids = CMGROUP_MAX;
  __uid_t euids_buf[neuids], auids_buf[nauids];
  __gid_t egids_buf[negids], agids_buf[nagids];
  __uid_t *euids = euids_buf, *auids = auids_buf;
  __gid_t *egids = egids_buf, *agids = agids_buf;

  struct procinfo *pi = NULL;
  mach_msg_type_number_t pi_size = 0;
  int flags = PI_FETCH_TASKINFO;
  char *tw = NULL;
  mach_msg_type_number_t tw_size = 0;
  unsigned i;

  err = __mach_port_mod_refs (mach_task_self (), rendezvous,
			    MACH_PORT_RIGHT_SEND, 1);
  if (err)
    goto out;

  do
    err = __USEPORT
      (AUTH, __auth_server_authenticate (port,
					 rendezvous, MACH_MSG_TYPE_COPY_SEND,
					 MACH_PORT_NULL, 0,
					 &euids, &neuids, &auids, &nauids,
					 &egids, &negids, &agids, &nagids));
  while (err == EINTR);
  if (err)
    goto out;

  /* Check whether this process indeed has these IDs */
  if (   !contains_uid (neuids, euids,  uid)
      && !contains_uid (nauids, auids,  uid)
   ||    !contains_uid (neuids, euids, euid)
      && !contains_uid (nauids, auids, euid)
   ||    !contains_gid (negids, egids,  gid)
      && !contains_gid (nagids, agids,  gid)
    )
    {
      err = EIO;
      goto out;
    }

  /* Check groups */
  for (i = 0; i < ngroups; i++)
    if (   !contains_gid (negids, egids, groups[i])
	&& !contains_gid (nagids, agids, groups[i]))
      {
	err = EIO;
	goto out;
      }

  /* Check PID  */
  /* XXX: Using proc_getprocinfo until
     proc_user_authenticate proc_server_authenticate is implemented
  */
  /* Get procinfo to check the owner.  Maybe he faked the pid, but at least we
     check the owner.  */
  err = __USEPORT (PROC, __proc_getprocinfo (port, pid, &flags,
					     (procinfo_t *)&pi,
					     &pi_size, &tw, &tw_size));
  if (err)
    goto out;

  if (   !contains_uid (neuids, euids, pi->owner)
      && !contains_uid (nauids, auids, pi->owner))
    err = EIO;

out:
  __mach_port_deallocate (__mach_task_self (), rendezvous);
  if (euids != euids_buf)
    __vm_deallocate (__mach_task_self(), (vm_address_t) euids, neuids * sizeof(uid_t));
  if (auids != auids_buf)
    __vm_deallocate (__mach_task_self(), (vm_address_t) auids, nauids * sizeof(uid_t));
  if (egids != egids_buf)
    __vm_deallocate (__mach_task_self(), (vm_address_t) egids, negids * sizeof(uid_t));
  if (agids != agids_buf)
    __vm_deallocate (__mach_task_self(), (vm_address_t) agids, nagids * sizeof(uid_t));
  if (tw_size)
    __vm_deallocate (__mach_task_self(), (vm_address_t) tw, tw_size);
  if (pi_size)
    __vm_deallocate (__mach_task_self(), (vm_address_t) pi, pi_size);

  return err;
}

/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.  */
ssize_t
__libc_recvmsg (int fd, struct msghdr *message, int flags)
{
  error_t err;
  addr_port_t aport;
  char *data = NULL;
  mach_msg_type_number_t len = 0;
  mach_port_t *ports, *newports = NULL;
  mach_msg_type_number_t nports = 0;
  struct cmsghdr *cmsg;
  char *cdata = NULL;
  mach_msg_type_number_t clen = 0;
  size_t amount;
  char *buf;
  int nfds, *opened_fds = NULL;
  int i, ii, j;
  int newfds;
  int cancel_oldtype;

  error_t reauthenticate (mach_port_t port, mach_port_t *result)
    {
      error_t err;
      mach_port_t ref;
      ref = __mach_reply_port ();
      int cancel_oldtype;

      cancel_oldtype = LIBC_CANCEL_ASYNC();
      do
	err = __io_reauthenticate (port, ref, MACH_MSG_TYPE_MAKE_SEND);
      while (err == EINTR);
      if (!err)
	do
	  err = __USEPORT_CANCEL (AUTH, __auth_user_authenticate (port,
					  ref, MACH_MSG_TYPE_MAKE_SEND,
					  result));
	while (err == EINTR);
      LIBC_CANCEL_RESET (cancel_oldtype);

      __mach_port_destroy (__mach_task_self (), ref);
      return err;
    }

  /* Find the total number of bytes to be read.  */
  amount = 0;
  for (i = 0; i < message->msg_iovlen; i++)
    {
      amount += message->msg_iov[i].iov_len;

      /* As an optimization, we set the initial values of DATA and LEN
         from the first non-empty iovec.  This kicks-in in the case
         where the whole packet fits into that iovec buffer.  */
      if (data == NULL && message->msg_iov[i].iov_len > 0)
	{
	  data = message->msg_iov[i].iov_base;
	  len = message->msg_iov[i].iov_len;
	}
    }

  buf = data;
  cancel_oldtype = LIBC_CANCEL_ASYNC();
  err = HURD_DPORT_USE_CANCEL (fd, __socket_recv (port, &aport,
						  flags, &data, &len,
						  &ports, &nports,
						  &cdata, &clen,
						  &message->msg_flags, amount));
  LIBC_CANCEL_RESET (cancel_oldtype);
  if (err)
    return __hurd_sockfail (fd, flags, err);

  if (message->msg_name != NULL && aport != MACH_PORT_NULL)
    {
      char *buf = message->msg_name;
      mach_msg_type_number_t buflen = message->msg_namelen;
      int type;

      cancel_oldtype = LIBC_CANCEL_ASYNC();
      err = __socket_whatis_address (aport, &type, &buf, &buflen);
      LIBC_CANCEL_RESET (cancel_oldtype);

      if (err == EOPNOTSUPP)
	/* If the protocol server can't tell us the address, just return a
	   zero-length one.  */
	{
	  buf = message->msg_name;
	  buflen = 0;
	  err = 0;
	}

      if (err)
	{
	  __mach_port_deallocate (__mach_task_self (), aport);
	  return __hurd_sockfail (fd, flags, err);
	}

      if (message->msg_namelen > buflen)
	message->msg_namelen = buflen;

      if (buf != message->msg_name)
	{
	  memcpy (message->msg_name, buf, message->msg_namelen);
	  __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
	}

      if (buflen > 0)
	((struct sockaddr *) message->msg_name)->sa_family = type;
    }
  else if (message->msg_name != NULL)
    message->msg_namelen = 0;

  if (MACH_PORT_VALID (aport))
    __mach_port_deallocate (__mach_task_self (), aport);

  if (buf == data)
    buf += len;
  else
    {
      /* Copy the data into MSG.  */
      if (len > amount)
	message->msg_flags |= MSG_TRUNC;
      else
	amount = len;

      buf = data;
      for (i = 0; i < message->msg_iovlen; i++)
	{
#define min(a, b)	((a) > (b) ? (b) : (a))
	  size_t copy = min (message->msg_iov[i].iov_len, amount);

	  memcpy (message->msg_iov[i].iov_base, buf, copy);

	  buf += copy;
	  amount -= copy;
	  if (len == 0)
	    break;
	}

      __vm_deallocate (__mach_task_self (), (vm_address_t) data, len);
    }

  /* Copy the control message into MSG.  */
  if (clen > message->msg_controllen)
    message->msg_flags |= MSG_CTRUNC;
  else
    message->msg_controllen = clen;
  memcpy (message->msg_control, cdata, message->msg_controllen);

  if (nports > 0)
    {
      newports = __alloca (nports * sizeof (mach_port_t));
      opened_fds = __alloca (nports * sizeof (int));
    }

  /* This counts how many ports we processed completely.  */
  i = 0;
  /* This counts how many new fds we create.  */
  newfds = 0;

  for (cmsg = CMSG_FIRSTHDR (message);
       cmsg;
       cmsg = CMSG_NXTHDR (message, cmsg))
  {
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
      {
	/* SCM_RIGHTS support.  */
	/* The fd's flags are passed in the control data.  */
	int *fds = (int *) CMSG_DATA (cmsg);
	nfds = (cmsg->cmsg_len - CMSG_ALIGN (sizeof (struct cmsghdr)))
	       / sizeof (int);

	for (j = 0; j < nfds; j++)
	  {
	    int fd_flags = (flags & MSG_CMSG_CLOEXEC) ? O_CLOEXEC : 0;
	    err = reauthenticate (ports[i], &newports[newfds]);
	    if (err)
	      goto cleanup;
	    /* We do not currently take any flag from the sender.  */
	    fds[j] = opened_fds[newfds] = _hurd_intern_fd (newports[newfds],
							   (fds[j] & 0)
							   | fd_flags,
							   0);
	    if (fds[j] == -1)
	      {
		err = errno;
		__mach_port_deallocate (__mach_task_self (), newports[newfds]);
		goto cleanup;
	      }
	    i++;
	    newfds++;
	  }
      }
    else if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDS)
      {
	/* SCM_CREDS support.  */
	/* Check received credentials */
	struct cmsgcred *ucredp = (struct cmsgcred *) CMSG_DATA(cmsg);

	err = check_auth (ports[i],
			  ucredp->cmcred_pid,
			  ucredp->cmcred_uid, ucredp->cmcred_euid,
			  ucredp->cmcred_gid,
			  ucredp->cmcred_ngroups, ucredp->cmcred_groups);
	if (err)
	  goto cleanup;
	i++;
      }
  }

  for (i = 0; i < nports; i++)
    __mach_port_deallocate (mach_task_self (), ports[i]);

  __vm_deallocate (__mach_task_self (), (vm_address_t) cdata, clen);

  return (buf - data);

cleanup:
  /* Clean up all the file descriptors from port 0 to i-1.  */
  if (nports > 0)
    {
      ii = 0;
      newfds = 0;
      for (cmsg = CMSG_FIRSTHDR (message);
	   cmsg;
	   cmsg = CMSG_NXTHDR (message, cmsg))
	{
	  if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
	    {
	      nfds = (cmsg->cmsg_len - CMSG_ALIGN (sizeof (struct cmsghdr)))
		     / sizeof (int);
	      for (j = 0; j < nfds && ii < i; j++, ii++, newfds++)
	      {
		_hurd_fd_close (_hurd_fd_get (opened_fds[newfds]));
		__mach_port_deallocate (__mach_task_self (), newports[newfds]);
		__mach_port_deallocate (__mach_task_self (), ports[ii]);
	      }
	    }
	  else if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDS)
	    {
	      __mach_port_deallocate (__mach_task_self (), ports[ii]);
	      ii++;
	    }
	}
    }

  __vm_deallocate (__mach_task_self (), (vm_address_t) cdata, clen);
  return __hurd_fail (err);
}

weak_alias (__libc_recvmsg, recvmsg)
weak_alias (__libc_recvmsg, __recvmsg)
