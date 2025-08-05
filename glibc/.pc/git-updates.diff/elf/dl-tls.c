/* Thread-local storage handling in the ELF dynamic linker.  Generic version.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <libintl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <atomic.h>

#include <tls.h>
#include <dl-tls.h>
#include <ldsodefs.h>
#include <dl-tls_block_align.h>

#if PTHREAD_IN_LIBC
# include <list.h>
#endif

#define TUNABLE_NAMESPACE rtld
#include <dl-tunables.h>

#include <dl-extra_tls.h>

/* Surplus static TLS, GLRO(dl_tls_static_surplus), is used for

   - IE TLS in libc.so for all dlmopen namespaces except in the initial
     one where libc.so is not loaded dynamically but at startup time,
   - IE TLS in other libraries which may be dynamically loaded even in the
     initial namespace,
   - and optionally for optimizing dynamic TLS access.

   The maximum number of namespaces is DL_NNS, but to support that many
   namespaces correctly the static TLS allocation should be significantly
   increased, which may cause problems with small thread stacks due to the
   way static TLS is accounted (bug 11787).

   So there is a rtld.nns tunable limit on the number of supported namespaces
   that affects the size of the static TLS and by default it's small enough
   not to cause problems with existing applications. The limit is not
   enforced or checked: it is the user's responsibility to increase rtld.nns
   if more dlmopen namespaces are used.

   Audit modules use their own namespaces, they are not included in rtld.nns,
   but come on top when computing the number of namespaces.  */

/* Size of initial-exec TLS in libc.so.  This should be the maximum of
   observed PT_GNU_TLS sizes across all architectures.  Some
   architectures have lower values due to differences in type sizes
   and link editor capabilities.  */
#define LIBC_IE_TLS 144

/* Size of initial-exec TLS in libraries other than libc.so.
   This should be large enough to cover runtime libraries of the
   compiler such as libgomp and libraries in libc other than libc.so.  */
#define OTHER_IE_TLS 144

/* Default number of namespaces.  */
#define DEFAULT_NNS 4

/* Default for dl_tls_static_optional.  */
#define OPTIONAL_TLS 512

/* Used to count the number of threads currently executing dynamic TLS
   updates.  Used to avoid recursive malloc calls in __tls_get_addr
   for an interposed malloc that uses global-dynamic TLS (which is not
   recommended); see _dl_tls_allocate_active checks.  This could be a
   per-thread flag, but would need TLS access in the dynamic linker.  */
unsigned int _dl_tls_threads_in_update;

static inline void
_dl_tls_allocate_begin (void)
{
  atomic_fetch_add_relaxed (&_dl_tls_threads_in_update, 1);
}

static inline void
_dl_tls_allocate_end (void)
{
  atomic_fetch_add_relaxed (&_dl_tls_threads_in_update, -1);
}

static inline bool
_dl_tls_allocate_active (void)
{
  return atomic_load_relaxed (&_dl_tls_threads_in_update) > 0;
}

/* Compute the static TLS surplus based on the namespace count and the
   TLS space that can be used for optimizations.  */
static inline int
tls_static_surplus (int nns, int opt_tls)
{
  return (nns - 1) * LIBC_IE_TLS + nns * OTHER_IE_TLS + opt_tls;
}

/* This value is chosen so that with default values for the tunables,
   the computation of dl_tls_static_surplus in
   _dl_tls_static_surplus_init yields the historic value 1664, for
   backwards compatibility.  */
#define LEGACY_TLS (1664 - tls_static_surplus (DEFAULT_NNS, OPTIONAL_TLS))

/* Calculate the size of the static TLS surplus, when the given
   number of audit modules are loaded.  Must be called after the
   number of audit modules is known and before static TLS allocation.  */
void
_dl_tls_static_surplus_init (size_t naudit)
{
  size_t nns, opt_tls;

  nns = TUNABLE_GET (nns, size_t, NULL);
  opt_tls = TUNABLE_GET (optional_static_tls, size_t, NULL);
  if (nns > DL_NNS)
    nns = DL_NNS;
  if (DL_NNS - nns < naudit)
    _dl_fatal_printf ("Failed loading %lu audit modules, %lu are supported.\n",
		      (unsigned long) naudit, (unsigned long) (DL_NNS - nns));
  nns += naudit;

  GL(dl_tls_static_optional) = opt_tls;
  assert (LEGACY_TLS >= 0);
  GLRO(dl_tls_static_surplus) = tls_static_surplus (nns, opt_tls) + LEGACY_TLS;
}

/* Out-of-memory handler.  */
static void
__attribute__ ((__noreturn__))
oom (void)
{
  _dl_fatal_printf ("cannot allocate memory for thread-local data: ABORT\n");
}


void
_dl_assign_tls_modid (struct link_map *l)
{
  size_t result;

  if (__builtin_expect (GL(dl_tls_dtv_gaps), false))
    {
      size_t disp = 0;
      struct dtv_slotinfo_list *runp = GL(dl_tls_dtv_slotinfo_list);

      /* Note that this branch will never be executed during program
	 start since there are no gaps at that time.  Therefore it
	 does not matter that the dl_tls_dtv_slotinfo is not allocated
	 yet when the function is called for the first times.

	 NB: the offset +1 is due to the fact that DTV[0] is used
	 for something else.  */
      result = GL(dl_tls_static_nelem) + 1;
      if (result <= GL(dl_tls_max_dtv_idx))
	do
	  {
	    while (result - disp < runp->len)
	      {
		if (runp->slotinfo[result - disp].map == NULL)
		  break;

		++result;
		assert (result <= GL(dl_tls_max_dtv_idx) + 1);
	      }

	    if (result - disp < runp->len)
	      {
		/* Mark the entry as used, so any dependency see it.  */
		atomic_store_relaxed (&runp->slotinfo[result - disp].map, l);
		atomic_store_relaxed (&runp->slotinfo[result - disp].gen, 0);
		break;
	      }

	    disp += runp->len;
	  }
	while ((runp = runp->next) != NULL);

      if (result > GL(dl_tls_max_dtv_idx))
	{
	  /* The new index must indeed be exactly one higher than the
	     previous high.  */
	  assert (result == GL(dl_tls_max_dtv_idx) + 1);
	  /* There is no gap anymore.  */
	  GL(dl_tls_dtv_gaps) = false;

	  goto nogaps;
	}
    }
  else
    {
      /* No gaps, allocate a new entry.  */
    nogaps:

      result = GL(dl_tls_max_dtv_idx) + 1;
      /* Can be read concurrently.  */
      atomic_store_relaxed (&GL(dl_tls_max_dtv_idx), result);
    }

  l->l_tls_modid = result;
}


size_t
_dl_count_modids (void)
{
  /* The count is the max unless dlclose or failed dlopen created gaps.  */
  if (__glibc_likely (!GL(dl_tls_dtv_gaps)))
    return GL(dl_tls_max_dtv_idx);

  /* We have gaps and are forced to count the non-NULL entries.  */
  size_t n = 0;
  struct dtv_slotinfo_list *runp = GL(dl_tls_dtv_slotinfo_list);
  while (runp != NULL)
    {
      for (size_t i = 0; i < runp->len; ++i)
	if (runp->slotinfo[i].map != NULL)
	  ++n;

      runp = runp->next;
    }

  return n;
}


void
_dl_determine_tlsoffset (void)
{
  size_t max_align = TCB_ALIGNMENT;
  size_t freetop = 0;
  size_t freebottom = 0;

  /* Determining the offset of the various parts of the static TLS
     block has several dependencies.  In addition we have to work
     around bugs in some toolchains.

     Each TLS block from the objects available at link time has a size
     and an alignment requirement.  The GNU ld computes the alignment
     requirements for the data at the positions *in the file*, though.
     I.e, it is not simply possible to allocate a block with the size
     of the TLS program header entry.  The data is laid out assuming
     that the first byte of the TLS block fulfills

       p_vaddr mod p_align == &TLS_BLOCK mod p_align

     This means we have to add artificial padding at the beginning of
     the TLS block.  These bytes are never used for the TLS data in
     this module but the first byte allocated must be aligned
     according to mod p_align == 0 so that the first byte of the TLS
     block is aligned according to p_vaddr mod p_align.  This is ugly
     and the linker can help by computing the offsets in the TLS block
     assuming the first byte of the TLS block is aligned according to
     p_align.

     The extra space which might be allocated before the first byte of
     the TLS block need not go unused.  The code below tries to use
     that memory for the next TLS block.  This can work if the total
     memory requirement for the next TLS block is smaller than the
     gap.  */

#if TLS_TCB_AT_TP
  /* We simply start with zero.  */
  size_t offset = 0;

  for (struct link_map *l = GL(dl_ns)[LM_ID_BASE]._ns_loaded; l != NULL;
       l = l->l_next)
    {
      if (l->l_tls_blocksize == 0)
	continue;

      size_t firstbyte = (-l->l_tls_firstbyte_offset
			  & (l->l_tls_align - 1));
      size_t off;
      max_align = MAX (max_align, l->l_tls_align);

      if (freebottom - freetop >= l->l_tls_blocksize)
	{
	  off = roundup (freetop + l->l_tls_blocksize
			 - firstbyte, l->l_tls_align)
		+ firstbyte;
	  if (off <= freebottom)
	    {
	      freetop = off;

	      /* XXX For some architectures we perhaps should store the
		 negative offset.  */
	      l->l_tls_offset = off;
	      continue;
	    }
	}

      off = roundup (offset + l->l_tls_blocksize - firstbyte,
		     l->l_tls_align) + firstbyte;
      if (off > offset + l->l_tls_blocksize
		+ (freebottom - freetop))
	{
	  freetop = offset;
	  freebottom = off - l->l_tls_blocksize;
	}
      offset = off;

      /* XXX For some architectures we perhaps should store the
	 negative offset.  */
      l->l_tls_offset = off;
    }

  /* Insert the extra TLS block after the last TLS block.  */

  /* Extra TLS block for internal usage to append at the end of the TLS blocks
     (in allocation order).  The address at which the block is allocated must
     be aligned to 'extra_tls_align'.  The size of the block as returned by
     '_dl_extra_tls_get_size ()' is always a multiple of the aligment.

     On Linux systems this is where the rseq area will be allocated.  On other
     systems it is currently unused and both values will be '0'.  */
  size_t extra_tls_size = _dl_extra_tls_get_size ();
  size_t extra_tls_align = _dl_extra_tls_get_align ();

  /* Increase the maximum alignment with the extra TLS alignment requirements
     if necessary.  */
  max_align = MAX (max_align, extra_tls_align);

  /* Add the extra TLS block to the global offset.  To ensure proper alignment,
     first align the current global offset to the extra TLS block requirements
     and then add the extra TLS block size.  Both values respect the extra TLS
     alignment requirements and so does the resulting offset.  */
  offset = roundup (offset, extra_tls_align ?: 1) + extra_tls_size;

 /* Record the extra TLS offset.

    With TLS_TCB_AT_TP the TLS blocks are allocated before the thread pointer
    in reverse order.  Our block is added last which results in it being the
    first in the static TLS block, thus record the most negative offset.

    The alignment requirements of the pointer resulting from this offset and
    the thread pointer are enforced by 'max_align' which is used to align the
    tcb_offset.  */
  _dl_extra_tls_set_offset (-offset);

  GL(dl_tls_static_used) = offset;
  GLRO (dl_tls_static_size) = (roundup (offset + GLRO(dl_tls_static_surplus),
					max_align)
			       + TLS_TCB_SIZE);
#elif TLS_DTV_AT_TP
  /* The TLS blocks start right after the TCB.  */
  size_t offset = TLS_TCB_SIZE;

  for (struct link_map *l = GL(dl_ns)[LM_ID_BASE]._ns_loaded; l != NULL;
       l = l->l_next)
    {
      if (l->l_tls_blocksize == 0)
	continue;

      size_t firstbyte = (-l->l_tls_firstbyte_offset
			  & (l->l_tls_align - 1));
      size_t off;
      max_align = MAX (max_align, l->l_tls_align);

      if (l->l_tls_blocksize <= freetop - freebottom)
	{
	  off = roundup (freebottom, l->l_tls_align);
	  if (off - freebottom < firstbyte)
	    off += l->l_tls_align;
	  if (off + l->l_tls_blocksize - firstbyte <= freetop)
	    {
	      l->l_tls_offset = off - firstbyte;
	      freebottom = (off + l->l_tls_blocksize
			    - firstbyte);
	      continue;
	    }
	}

      off = roundup (offset, l->l_tls_align);
      if (off - offset < firstbyte)
	off += l->l_tls_align;

      l->l_tls_offset = off - firstbyte;
      if (off - firstbyte - offset > freetop - freebottom)
	{
	  freebottom = offset;
	  freetop = off - firstbyte;
	}

      offset = off + l->l_tls_blocksize - firstbyte;
    }

  /* Insert the extra TLS block after the last TLS block.  */

  /* Extra TLS block for internal usage to append at the end of the TLS blocks
     (in allocation order).  The address at which the block is allocated must
     be aligned to 'extra_tls_align'.  The size of the block as returned by
     '_dl_extra_tls_get_size ()' is always a multiple of the aligment.

     On Linux systems this is where the rseq area will be allocated.  On other
     systems it is currently unused and both values will be '0'.  */
  size_t extra_tls_size = _dl_extra_tls_get_size ();
  size_t extra_tls_align = _dl_extra_tls_get_align ();

  /* Increase the maximum alignment with the extra TLS alignment requirements
     if necessary.  */
  max_align = MAX (max_align, extra_tls_align);

  /* Align the global offset to the beginning of the extra TLS block.  */
  offset = roundup (offset, extra_tls_align ?: 1);

 /* Record the extra TLS offset.

    With TLS_DTV_AT_TP the TLS blocks are allocated after the thread pointer in
    order.  Our block is added last which results in it being the last in the
    static TLS block, thus record the offset as the size of the static TLS
    block minus the size of our block.

    On some architectures the TLS blocks are offset from the thread pointer,
    include this offset in the extra TLS block offset.

    The alignment requirements of the pointer resulting from this offset and
    the thread pointer are enforced by 'max_align' which is used to align the
    tcb_offset.  */
  _dl_extra_tls_set_offset (offset - TLS_TP_OFFSET);

  /* Add the extra TLS block to the global offset.  */
  offset += extra_tls_size;

  GL(dl_tls_static_used) = offset;
  GLRO (dl_tls_static_size) = roundup (offset + GLRO(dl_tls_static_surplus),
				       TCB_ALIGNMENT);
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif

  /* The alignment requirement for the static TLS block.  */
  GLRO (dl_tls_static_align) = max_align;
}

static void *
allocate_dtv (void *result)
{
  dtv_t *dtv;
  size_t dtv_length;

  /* Relaxed MO, because the dtv size is later rechecked, not relied on.  */
  size_t max_modid = atomic_load_relaxed (&GL(dl_tls_max_dtv_idx));
  /* We allocate a few more elements in the dtv than are needed for the
     initial set of modules.  This should avoid in most cases expansions
     of the dtv.  */
  dtv_length = max_modid + DTV_SURPLUS;
  dtv = calloc (dtv_length + 2, sizeof (dtv_t));
  if (dtv != NULL)
    {
      /* This is the initial length of the dtv.  */
      dtv[0].counter = dtv_length;

      /* The rest of the dtv (including the generation counter) is
	 Initialize with zero to indicate nothing there.  */

      /* Add the dtv to the thread data structures.  */
      INSTALL_DTV (result, dtv);
    }
  else
    result = NULL;

  return result;
}

/* Get size and alignment requirements of the static TLS block.  This
   function is no longer used by glibc itself, but the GCC sanitizers
   use it despite the GLIBC_PRIVATE status.  */
void
_dl_get_tls_static_info (size_t *sizep, size_t *alignp)
{
  *sizep = GLRO (dl_tls_static_size);
  *alignp = GLRO (dl_tls_static_align);
}

/* Derive the location of the pointer to the start of the original
   allocation (before alignment) from the pointer to the TCB.  */
static inline void **
tcb_to_pointer_to_free_location (void *tcb)
{
#if TLS_TCB_AT_TP
  /* The TCB follows the TLS blocks, and the pointer to the front
     follows the TCB.  */
  void **original_pointer_location = tcb + TLS_TCB_SIZE;
#elif TLS_DTV_AT_TP
  /* The TCB comes first, preceded by the pre-TCB, and the pointer is
     before that.  */
  void **original_pointer_location = tcb - TLS_PRE_TCB_SIZE - sizeof (void *);
#endif
  return original_pointer_location;
}

void *
_dl_allocate_tls_storage (void)
{
  size_t size = _dl_tls_block_size_with_pre ();

  /* Perform the allocation.  Reserve space for alignment storage of
     the pointer that will have to be freed.  */
  _dl_tls_allocate_begin ();
  void *allocated = malloc (size + GLRO (dl_tls_static_align)
			    + sizeof (void *));
  if (__glibc_unlikely (allocated == NULL))
    {
      _dl_tls_allocate_end ();
      return NULL;
    }

  void *result = _dl_tls_block_align (size, allocated);

  /* Record the value of the original pointer for later
     deallocation.  */
  *tcb_to_pointer_to_free_location (result) = allocated;

  result = allocate_dtv (result);
  if (result == NULL)
    free (allocated);

  _dl_tls_allocate_end ();
  return result;
}


#ifndef SHARED
extern dtv_t _dl_static_dtv[];
# define _dl_initial_dtv (&_dl_static_dtv[1])
#endif

static dtv_t *
_dl_resize_dtv (dtv_t *dtv, size_t max_modid)
{
  /* Resize the dtv.  */
  dtv_t *newp;
  size_t newsize = max_modid + DTV_SURPLUS;
  size_t oldsize = dtv[-1].counter;

  _dl_tls_allocate_begin ();
  if (dtv == GL(dl_initial_dtv))
    {
      /* This is the initial dtv that was either statically allocated in
	 __libc_setup_tls or allocated during rtld startup using the
	 dl-minimal.c malloc instead of the real malloc.  We can't free
	 it, we have to abandon the old storage.  */

      newp = malloc ((2 + newsize) * sizeof (dtv_t));
      if (newp == NULL)
	oom ();
      memcpy (newp, &dtv[-1], (2 + oldsize) * sizeof (dtv_t));
    }
  else
    {
      newp = realloc (&dtv[-1],
		      (2 + newsize) * sizeof (dtv_t));
      if (newp == NULL)
	oom ();
    }
  _dl_tls_allocate_end ();

  newp[0].counter = newsize;

  /* Clear the newly allocated part.  */
  memset (newp + 2 + oldsize, '\0',
	  (newsize - oldsize) * sizeof (dtv_t));

  /* Return the generation counter.  */
  return &newp[1];
}


/* Allocate initial TLS.  RESULT should be a non-NULL pointer to storage
   for the TLS space.  The DTV may be resized, and so this function may
   call malloc to allocate that space.  The loader's GL(dl_load_tls_lock)
   is taken when manipulating global TLS-related data in the loader.

   If MAIN_THREAD, this is the first call during process
   initialization.  In this case, TLS initialization for secondary
   (audit) namespaces is skipped because that has already been handled
   by dlopen.  */
void *
_dl_allocate_tls_init (void *result, bool main_thread)
{
  if (result == NULL)
    /* The memory allocation failed.  */
    return NULL;

  dtv_t *dtv = GET_DTV (result);
  struct dtv_slotinfo_list *listp;
  size_t total = 0;
  size_t maxgen = 0;

  /* Protects global dynamic TLS related state.  */
  __rtld_lock_lock_recursive (GL(dl_load_tls_lock));

  /* Check if the current dtv is big enough.   */
  if (dtv[-1].counter < GL(dl_tls_max_dtv_idx))
    {
      /* Resize the dtv.  */
      dtv = _dl_resize_dtv (dtv, GL(dl_tls_max_dtv_idx));

      /* Install this new dtv in the thread data structures.  */
      INSTALL_DTV (result, &dtv[-1]);
    }

  /* We have to prepare the dtv for all currently loaded modules using
     TLS.  For those which are dynamically loaded we add the values
     indicating deferred allocation.  */
  listp = GL(dl_tls_dtv_slotinfo_list);
  while (1)
    {
      size_t cnt;

      for (cnt = total == 0 ? 1 : 0; cnt < listp->len; ++cnt)
	{
	  struct link_map *map;
	  void *dest;

	  /* Check for the total number of used slots.  */
	  if (total + cnt > GL(dl_tls_max_dtv_idx))
	    break;

	  map = listp->slotinfo[cnt].map;
	  if (map == NULL)
	    /* Unused entry.  */
	    continue;

	  /* Keep track of the maximum generation number.  This might
	     not be the generation counter.  */
	  assert (listp->slotinfo[cnt].gen <= GL(dl_tls_generation));
	  maxgen = MAX (maxgen, listp->slotinfo[cnt].gen);

	  dtv[map->l_tls_modid].pointer.val = TLS_DTV_UNALLOCATED;
	  dtv[map->l_tls_modid].pointer.to_free = NULL;

	  if (map->l_tls_offset == NO_TLS_OFFSET
	      || map->l_tls_offset == FORCED_DYNAMIC_TLS_OFFSET)
	    continue;

	  assert (map->l_tls_modid == total + cnt);
	  assert (map->l_tls_blocksize >= map->l_tls_initimage_size);
#if TLS_TCB_AT_TP
	  assert ((size_t) map->l_tls_offset >= map->l_tls_blocksize);
	  dest = (char *) result - map->l_tls_offset;
#elif TLS_DTV_AT_TP
	  dest = (char *) result + map->l_tls_offset;
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif

	  /* Set up the DTV entry.  The simplified __tls_get_addr that
	     some platforms use in static programs requires it.  */
	  dtv[map->l_tls_modid].pointer.val = dest;

	  /* Copy the initialization image and clear the BSS part.
	     For audit modules or dependencies with initial-exec TLS,
	     we can not set the initial TLS image on default loader
	     initialization because it would already be set by the
	     audit setup, which uses the dlopen code and already
	     clears l_need_tls_init.  Calls with !main_thread from
	     pthread_create need to initialze TLS for the current
	     thread regardless of namespace.  */
	  if (map->l_ns != LM_ID_BASE && main_thread)
	    continue;
	  memset (__mempcpy (dest, map->l_tls_initimage,
			     map->l_tls_initimage_size), '\0',
		  map->l_tls_blocksize - map->l_tls_initimage_size);
	  if (main_thread)
	    map->l_need_tls_init = 0;
	}

      total += cnt;
      if (total > GL(dl_tls_max_dtv_idx))
	break;

      listp = listp->next;
      assert (listp != NULL);
    }
  __rtld_lock_unlock_recursive (GL(dl_load_tls_lock));

  /* The DTV version is up-to-date now.  */
  dtv[0].counter = maxgen;

  return result;
}
rtld_hidden_def (_dl_allocate_tls_init)

void *
_dl_allocate_tls (void *mem)
{
  return _dl_allocate_tls_init (mem == NULL
				? _dl_allocate_tls_storage ()
				: allocate_dtv (mem), false);
}
rtld_hidden_def (_dl_allocate_tls)


void
_dl_deallocate_tls (void *tcb, bool dealloc_tcb)
{
  dtv_t *dtv = GET_DTV (tcb);

  /* We need to free the memory allocated for non-static TLS.  */
  for (size_t cnt = 0; cnt < dtv[-1].counter; ++cnt)
    free (dtv[1 + cnt].pointer.to_free);

  /* The array starts with dtv[-1].  */
  if (dtv != GL(dl_initial_dtv))
    free (dtv - 1);

  if (dealloc_tcb)
    free (*tcb_to_pointer_to_free_location (tcb));
}
rtld_hidden_def (_dl_deallocate_tls)


#ifdef SHARED

/* Allocate one DTV entry.  */
static struct dtv_pointer
allocate_dtv_entry (size_t alignment, size_t size)
{
  if (powerof2 (alignment) && alignment <= _Alignof (max_align_t))
    {
      /* The alignment is supported by malloc.  */
      _dl_tls_allocate_begin ();
      void *ptr = malloc (size);
      _dl_tls_allocate_end ();
      return (struct dtv_pointer) { ptr, ptr };
    }

  /* Emulate memalign to by manually aligning a pointer returned by
     malloc.  First compute the size with an overflow check.  */
  size_t alloc_size = size + alignment;
  if (alloc_size < size)
    return (struct dtv_pointer) {};

  /* Perform the allocation.  This is the pointer we need to free
     later.  */
  _dl_tls_allocate_begin ();
  void *start = malloc (alloc_size);
  _dl_tls_allocate_end ();

  if (start == NULL)
    return (struct dtv_pointer) {};

  /* Find the aligned position within the larger allocation.  */
  void *aligned = (void *) roundup ((uintptr_t) start, alignment);

  return (struct dtv_pointer) { .val = aligned, .to_free = start };
}

static struct dtv_pointer
allocate_and_init (struct link_map *map)
{
  struct dtv_pointer result = allocate_dtv_entry
    (map->l_tls_align, map->l_tls_blocksize);
  if (result.val == NULL)
    oom ();

  /* Initialize the memory.  */
  memset (__mempcpy (result.val, map->l_tls_initimage,
		     map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);

  return result;
}


struct link_map *
_dl_update_slotinfo (unsigned long int req_modid, size_t new_gen)
{
  struct link_map *the_map = NULL;
  dtv_t *dtv = THREAD_DTV ();

  /* CONCURRENCY NOTES:

     The global dl_tls_dtv_slotinfo_list array contains for each module
     index the generation counter current when that entry was updated.
     This array never shrinks so that all module indices which were
     valid at some time can be used to access it.  Concurrent loading
     and unloading of modules can update slotinfo entries or extend
     the array.  The updates happen under the GL(dl_load_tls_lock) and
     finish with the release store of the generation counter to
     GL(dl_tls_generation) which is synchronized with the load of
     new_gen in the caller.  So updates up to new_gen are synchronized
     but updates for later generations may not be.

     Here we update the thread dtv from old_gen (== dtv[0].counter) to
     new_gen generation.  For this, each dtv[i] entry is either set to
     an unallocated state (set), or left unmodified (nop).  Where (set)
     may resize the dtv first if modid i >= dtv[-1].counter. The rules
     for the decision between (set) and (nop) are

     (1) If slotinfo entry i is concurrently updated then either (set)
         or (nop) is valid: TLS access cannot use dtv[i] unless it is
         synchronized with a generation > new_gen.

     Otherwise, if the generation of slotinfo entry i is gen and the
     loaded module for this entry is map then

     (2) If gen <= old_gen then do (nop).

     (3) If old_gen < gen <= new_gen then
         (3.1) if map != 0 then (set)
         (3.2) if map == 0 then either (set) or (nop).

     Note that (1) cannot be reliably detected, but since both actions
     are valid it does not have to be.  Only (2) and (3.1) cases need
     to be distinguished for which relaxed mo access of gen and map is
     enough: their value is synchronized when it matters.

     Note that a relaxed mo load may give an out-of-thin-air value since
     it is used in decisions that can affect concurrent stores.  But this
     should only happen if the OOTA value causes UB that justifies the
     concurrent store of the value.  This is not expected to be an issue
     in practice.  */
  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

  if (dtv[0].counter < new_gen)
    {
      size_t total = 0;
      size_t max_modid  = atomic_load_relaxed (&GL(dl_tls_max_dtv_idx));
      assert (max_modid >= req_modid);

      /* We have to look through the entire dtv slotinfo list.  */
      listp =  GL(dl_tls_dtv_slotinfo_list);
      do
	{
	  for (size_t cnt = total == 0 ? 1 : 0; cnt < listp->len; ++cnt)
	    {
	      size_t modid = total + cnt;

	      /* Case (1) for all later modids.  */
	      if (modid > max_modid)
		break;

	      size_t gen = atomic_load_relaxed (&listp->slotinfo[cnt].gen);

	      /* Case (1).  */
	      if (gen > new_gen)
		continue;

	      /* Case (2) or (1).  */
	      if (gen <= dtv[0].counter)
		continue;

	      /* Case (3) or (1).  */

	      /* If there is no map this means the entry is empty.  */
	      struct link_map *map
		= atomic_load_relaxed (&listp->slotinfo[cnt].map);
	      /* Check whether the current dtv array is large enough.  */
	      if (dtv[-1].counter < modid)
		{
		  /* Case (3.2) or (1).  */
		  if (map == NULL)
		    continue;

		  /* Resizing the dtv aborts on failure: bug 16134.  */
		  dtv = _dl_resize_dtv (dtv, max_modid);

		  assert (modid <= dtv[-1].counter);

		  /* Install this new dtv in the thread data
		     structures.  */
		  INSTALL_NEW_DTV (dtv);
		}

	      /* If there is currently memory allocate for this
		 dtv entry free it.  Note: this is not AS-safe.  */
	      /* XXX Ideally we will at some point create a memory
		 pool.  */
	      /* Avoid calling free on a null pointer.  Some mallocs
		 incorrectly use dynamic TLS, and depending on how the
		 free function was compiled, it could call
		 __tls_get_addr before the null pointer check in the
		 free implementation.  Checking here papers over at
		 least some dynamic TLS usage by interposed mallocs.  */
	      if (dtv[modid].pointer.to_free != NULL)
		{
		  _dl_tls_allocate_begin ();
		  free (dtv[modid].pointer.to_free);
		  _dl_tls_allocate_end ();
		}
	      dtv[modid].pointer.val = TLS_DTV_UNALLOCATED;
	      dtv[modid].pointer.to_free = NULL;

	      if (modid == req_modid)
		the_map = map;
	    }

	  total += listp->len;
	  if (total > max_modid)
	    break;

	  /* Synchronize with _dl_add_to_slotinfo.  Ideally this would
	     be consume MO since we only need to order the accesses to
	     the next node after the read of the address and on most
	     hardware (other than alpha) a normal load would do that
	     because of the address dependency.  */
	  listp = atomic_load_acquire (&listp->next);
	}
      while (listp != NULL);

      /* This will be the new maximum generation counter.  */
      dtv[0].counter = new_gen;
    }

  return the_map;
}

/* Adjust the TLS variable pointer using the TLS descriptor offset and
   the ABI-specific offset.  */
static inline void *
tls_get_addr_adjust (void *from_dtv, tls_index *ti)
{
  /* Perform arithmetic in uintptr_t to avoid pointer wraparound
     issues.  The outer cast to uintptr_t suppresses a warning about
     pointer/integer size mismatch on ILP32 targets with 64-bit
     ti_offset.  */
  return (void *) (uintptr_t) ((uintptr_t) from_dtv + ti->ti_offset
			       + TLS_DTV_OFFSET);
}

static void *
__attribute_noinline__
tls_get_addr_tail (tls_index *ti, dtv_t *dtv, struct link_map *the_map)
{
  /* The allocation was deferred.  Do it now.  */
  if (the_map == NULL)
    {
      /* Find the link map for this module.  */
      size_t idx = ti->ti_module;
      struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

      while (idx >= listp->len)
	{
	  idx -= listp->len;
	  listp = listp->next;
	}

      the_map = listp->slotinfo[idx].map;
    }

  /* Make sure that, if a dlopen running in parallel forces the
     variable into static storage, we'll wait until the address in the
     static TLS block is set up, and use that.  If we're undecided
     yet, make sure we make the decision holding the lock as well.  */
  if (__glibc_unlikely (the_map->l_tls_offset
			!= FORCED_DYNAMIC_TLS_OFFSET))
    {
      __rtld_lock_lock_recursive (GL(dl_load_tls_lock));
      if (__glibc_likely (the_map->l_tls_offset == NO_TLS_OFFSET))
	{
	  the_map->l_tls_offset = FORCED_DYNAMIC_TLS_OFFSET;
	  __rtld_lock_unlock_recursive (GL(dl_load_tls_lock));
	}
      else if (__glibc_likely (the_map->l_tls_offset
			       != FORCED_DYNAMIC_TLS_OFFSET))
	{
#if TLS_TCB_AT_TP
	  void *p = (char *) THREAD_SELF - the_map->l_tls_offset;
#elif TLS_DTV_AT_TP
	  void *p = (char *) THREAD_SELF + the_map->l_tls_offset + TLS_PRE_TCB_SIZE;
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
	  __rtld_lock_unlock_recursive (GL(dl_load_tls_lock));

	  dtv[ti->ti_module].pointer.to_free = NULL;
	  dtv[ti->ti_module].pointer.val = p;

	  return tls_get_addr_adjust (p, ti);
	}
      else
	__rtld_lock_unlock_recursive (GL(dl_load_tls_lock));
    }
  struct dtv_pointer result = allocate_and_init (the_map);
  dtv[ti->ti_module].pointer = result;
  assert (result.to_free != NULL);

  return tls_get_addr_adjust (result.val, ti);
}


static struct link_map *
__attribute_noinline__
update_get_addr (tls_index *ti, size_t gen)
{
  struct link_map *the_map = _dl_update_slotinfo (ti->ti_module, gen);
  dtv_t *dtv = THREAD_DTV ();

  void *p = dtv[ti->ti_module].pointer.val;

  if (__glibc_unlikely (p == TLS_DTV_UNALLOCATED))
    return tls_get_addr_tail (ti, dtv, the_map);

  return tls_get_addr_adjust (p, ti);
}

/* For all machines that have a non-macro version of __tls_get_addr, we
   want to use rtld_hidden_proto/rtld_hidden_def in order to call the
   internal alias for __tls_get_addr from ld.so. This avoids a PLT entry
   in ld.so for __tls_get_addr.  */

#ifndef __tls_get_addr
extern void * __tls_get_addr (tls_index *ti);
rtld_hidden_proto (__tls_get_addr)
rtld_hidden_def (__tls_get_addr)
#endif

/* The generic dynamic and local dynamic model cannot be used in
   statically linked applications.  */
void *
__tls_get_addr (tls_index *ti)
{
  dtv_t *dtv = THREAD_DTV ();

  /* Update is needed if dtv[0].counter < the generation of the accessed
     module, but the global generation counter is easier to check (which
     must be synchronized up to the generation of the accessed module by
     user code doing the TLS access so relaxed mo read is enough).  */
  size_t gen = atomic_load_relaxed (&GL(dl_tls_generation));
  if (__glibc_unlikely (dtv[0].counter != gen))
    {
      if (_dl_tls_allocate_active ()
	  && ti->ti_module < _dl_tls_initial_modid_limit)
	  /* This is a reentrant __tls_get_addr call, but we can
	     satisfy it because it's an initially-loaded module ID.
	     These TLS slotinfo slots do not change, so the
	     out-of-date generation counter does not matter.  However,
	     if not in a TLS update, still update_get_addr below, to
	     get off the slow path eventually.  */
	;
      else
	{
	  /* Update DTV up to the global generation, see CONCURRENCY NOTES
	     in _dl_update_slotinfo.  */
	  gen = atomic_load_acquire (&GL(dl_tls_generation));
	  return update_get_addr (ti, gen);
	}
    }

  void *p = dtv[ti->ti_module].pointer.val;

  if (__glibc_unlikely (p == TLS_DTV_UNALLOCATED))
    return tls_get_addr_tail (ti, dtv, NULL);

  return tls_get_addr_adjust (p, ti);
}
#endif /* SHARED */


/* Look up the module's TLS block as for __tls_get_addr,
   but never touch anything.  Return null if it's not allocated yet.  */
void *
_dl_tls_get_addr_soft (struct link_map *l)
{
  if (__glibc_unlikely (l->l_tls_modid == 0))
    /* This module has no TLS segment.  */
    return NULL;

  dtv_t *dtv = THREAD_DTV ();
  /* This may be called without holding the GL(dl_load_tls_lock).  Reading
     arbitrary gen value is fine since this is best effort code.  */
  size_t gen = atomic_load_relaxed (&GL(dl_tls_generation));
  if (__glibc_unlikely (dtv[0].counter != gen))
    {
      /* This thread's DTV is not completely current,
	 but it might already cover this module.  */

      if (l->l_tls_modid >= dtv[-1].counter)
	/* Nope.  */
	return NULL;

      size_t idx = l->l_tls_modid;
      struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);
      while (idx >= listp->len)
	{
	  idx -= listp->len;
	  listp = listp->next;
	}

      /* We've reached the slot for this module.
	 If its generation counter is higher than the DTV's,
	 this thread does not know about this module yet.  */
      if (dtv[0].counter < listp->slotinfo[idx].gen)
	return NULL;
    }

  void *data = dtv[l->l_tls_modid].pointer.val;
  if (__glibc_unlikely (data == TLS_DTV_UNALLOCATED))
    /* The DTV is current, but this thread has not yet needed
       to allocate this module's segment.  */
    data = NULL;

  return data;
}

size_t _dl_tls_initial_modid_limit;

void
_dl_tls_initial_modid_limit_setup (void)
{
  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);
  size_t idx;
  /* Start with 1 because TLS module ID zero is unused.  */
  for (idx = 1; idx < listp->len; ++idx)
    {
      struct link_map *l = listp->slotinfo[idx].map;
      if (l == NULL
	  /* The object can be unloaded, so its modid can be
	     reassociated.  */
	  || !(l->l_type == lt_executable || l->l_type == lt_library))
	break;
    }
  _dl_tls_initial_modid_limit = idx;
}


/* Add module to slot information data.  If DO_ADD is false, only the
   required memory is allocated.  Must be called with
   GL (dl_load_tls_lock) acquired.  If the function has already been
   called for the link map L with !DO_ADD, then this function will not
   raise an exception, otherwise it is possible that it encounters a
   memory allocation failure.

   Return false if L has already been added to the slotinfo data, or
   if L has no TLS data.  If the returned value is true, L has been
   added with this call (DO_ADD), or has been added in a previous call
   (!DO_ADD).

   The expected usage is as follows: Call _dl_add_to_slotinfo for
   several link maps with DO_ADD set to false, and record if any calls
   result in a true result.  If there was a true result, call
   _dl_add_to_slotinfo again, this time with DO_ADD set to true.  (For
   simplicity, it's possible to call the function for link maps where
   the previous result was false.)  The return value from the second
   round of calls can be ignored.  If there was true result initially,
   call _dl_update_slotinfo to update the TLS generation counter.  */
bool
_dl_add_to_slotinfo (struct link_map *l, bool do_add)
{
  if (l->l_tls_blocksize == 0 || l->l_tls_in_slotinfo)
    return false;

  /* Now that we know the object is loaded successfully add
     modules containing TLS data to the dtv info table.  We
     might have to increase its size.  */
  struct dtv_slotinfo_list *listp;
  struct dtv_slotinfo_list *prevp;
  size_t idx = l->l_tls_modid;

  /* Find the place in the dtv slotinfo list.  */
  listp = GL(dl_tls_dtv_slotinfo_list);
  prevp = NULL;		/* Needed to shut up gcc.  */
  do
    {
      /* Does it fit in the array of this list element?  */
      if (idx < listp->len)
	break;
      idx -= listp->len;
      prevp = listp;
      listp = listp->next;
    }
  while (listp != NULL);

  if (listp == NULL)
    {
      /* When we come here it means we have to add a new element
	 to the slotinfo list.  And the new module must be in
	 the first slot.  */
      assert (idx == 0);

      _dl_tls_allocate_begin ();
      listp = (struct dtv_slotinfo_list *)
	malloc (sizeof (struct dtv_slotinfo_list)
		+ TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
      _dl_tls_allocate_end ();
      if (listp == NULL)
	{
	  /* We ran out of memory while resizing the dtv slotinfo list.  */
	  _dl_signal_error (ENOMEM, "dlopen", NULL, N_("\
cannot create TLS data structures"));
	}

      listp->len = TLS_SLOTINFO_SURPLUS;
      listp->next = NULL;
      memset (listp->slotinfo, '\0',
	      TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
      /* Synchronize with _dl_update_slotinfo.  */
      atomic_store_release (&prevp->next, listp);
    }

  /* Add the information into the slotinfo data structure.  */
  if (do_add)
    {
      /* Can be read concurrently.  See _dl_update_slotinfo.  */
      atomic_store_relaxed (&listp->slotinfo[idx].map, l);
      atomic_store_relaxed (&listp->slotinfo[idx].gen,
			    GL(dl_tls_generation) + 1);
      l->l_tls_in_slotinfo = true;
    }

  return true;
}

#if PTHREAD_IN_LIBC
static inline void __attribute__((always_inline))
init_one_static_tls (struct pthread *curp, struct link_map *map)
{
# if TLS_TCB_AT_TP
  void *dest = (char *) curp - map->l_tls_offset;
# elif TLS_DTV_AT_TP
  void *dest = (char *) curp + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Initialize the memory.  */
  memset (__mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

void
_dl_init_static_tls (struct link_map *map)
{
  lll_lock (GL (dl_stack_cache_lock), LLL_PRIVATE);

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &GL (dl_stack_used))
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &GL (dl_stack_user))
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  lll_unlock (GL (dl_stack_cache_lock), LLL_PRIVATE);
}
#endif /* PTHREAD_IN_LIBC */
