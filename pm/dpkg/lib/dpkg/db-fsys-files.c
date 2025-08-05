/*
 * libdpkg - Debian packaging suite library routines
 * db-fsys-files.c - management of filesystem files database
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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

#ifdef HAVE_LINUX_FIEMAP_H
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/string.h>
#include <dpkg/path.h>
#include <dpkg/dir.h>
#include <dpkg/fdio.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-files.h>
#include <dpkg/progress.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

/*** Generic data structures and routines. ***/

static bool allpackagesdone = false;

void note_must_reread_files_inpackage(struct pkginfo *pkg) {
  allpackagesdone = false;
  pkg->files_list_valid = false;
}

enum pkg_filesdb_load_status {
  PKG_FILESDB_LOAD_NONE = 0,
  PKG_FILESDB_LOAD_INPROGRESS = 1,
  PKG_FILESDB_LOAD_DONE = 2,
};

static enum pkg_filesdb_load_status saidread = PKG_FILESDB_LOAD_NONE;

static void
fsys_list_parse_buffer(struct varbuf *vb, struct pkginfo *pkg)
{
  struct fsys_namenode_list **files_tail;
  char *loaded_list_end, *thisline;

  loaded_list_end = vb->buf + vb->used;

  files_tail = &pkg->files;
  thisline = vb->buf;

  while (thisline < loaded_list_end) {
    struct fsys_namenode *namenode;
    char *nextline, *ptr;

    ptr = memchr(thisline, '\n', loaded_list_end - thisline);
    if (ptr == NULL)
      ohshit(_("files list file for package '%.250s' is missing final newline"),
             pkg_name(pkg, pnaw_nonambig));

    /* Where to start next time around. */
    nextline = ptr + 1;

    /* Strip trailing ‘/’. */
    if (ptr > thisline && ptr[-1] == '/')
      ptr--;

    /* Add the file to the list. */
    if (ptr == thisline)
      ohshit(_("files list file for package '%.250s' contains empty filename"),
             pkg_name(pkg, pnaw_nonambig));
    *ptr = '\0';

    namenode = fsys_hash_find_node(thisline, FHFF_NONE);
    files_tail = pkg_files_add_file(pkg, namenode, files_tail);

    thisline = nextline;
  }
}

/**
 * Load the list of files in this package into memory, or update the
 * list if it is there but stale.
 */
void
ensure_packagefiles_available(struct pkginfo *pkg)
{
  const char *filelistfile;
  struct varbuf buf = VARBUF_INIT;
  struct dpkg_error err = DPKG_ERROR_INIT;

  if (pkg->files_list_valid)
    return;

  /* Throw away any stale data, if there was any. */
  pkg_files_blank(pkg);

  /* Packages which aren't installed don't have a files list. */
  if (pkg->status == PKG_STAT_NOTINSTALLED) {
    pkg->files_list_valid = true;
    return;
  }

  filelistfile = pkg_infodb_get_file(pkg, &pkg->installed, LISTFILE);

  onerr_abort++;

  if (file_slurp(filelistfile, &buf, &err) < 0) {
    if (err.syserrno != ENOENT)
      dpkg_error_print(&err, _("loading files list file for package '%s'"),
                       pkg_name(pkg, pnaw_nonambig));

    onerr_abort--;
    if (pkg->status != PKG_STAT_CONFIGFILES &&
        dpkg_version_is_informative(&pkg->configversion)) {
      warning(_("files list file for package '%.250s' missing; assuming "
                "package has no files currently installed"),
              pkg_name(pkg, pnaw_nonambig));
    }
    pkg->files = NULL;
    pkg->files_list_valid = true;
    return;
  }

  if (buf.used)
    fsys_list_parse_buffer(&buf, pkg);

  varbuf_destroy(&buf);

  onerr_abort--;

  pkg->files_list_valid = true;
}

#if defined(HAVE_LINUX_FIEMAP_H)
static int
pkg_sorter_by_files_list_phys_offs(const void *a, const void *b)
{
  const struct pkginfo *pa = *(const struct pkginfo **)a;
  const struct pkginfo *pb = *(const struct pkginfo **)b;

  /* We can't simply subtract, because the difference may be greater than
   * INT_MAX. */
  if (pa->files_list_phys_offs < pb->files_list_phys_offs)
    return -1;
  else if (pa->files_list_phys_offs > pb->files_list_phys_offs)
    return 1;
  else
    return 0;
}

#define DPKG_FIEMAP_EXTENTS	1
#define DPKG_FIEMAP_BUF_SIZE	\
	(sizeof(struct fiemap) + \
	 sizeof(struct fiemap_extent) * DPKG_FIEMAP_EXTENTS) / sizeof(__u64)

static void
pkg_files_optimize_load(struct pkg_array *array)
{
  __u64 fiemap_buf[DPKG_FIEMAP_BUF_SIZE];
  struct fiemap *fm = (struct fiemap *)fiemap_buf;
  struct statfs fs;
  int i;

  /* Get the filesystem block size. */
  if (statfs(pkg_infodb_get_dir(), &fs) < 0)
    return;

  /* Sort packages by the physical location of their list files, so that
   * scanning them later will minimize disk drive head movements. */
  for (i = 0; i < array->n_pkgs; i++) {
    struct pkginfo *pkg = array->pkgs[i];
    const char *listfile;
    int fd;

    if (pkg->status == PKG_STAT_NOTINSTALLED ||
        pkg->files_list_phys_offs != 0)
      continue;

    pkg->files_list_phys_offs = -1;

    listfile = pkg_infodb_get_file(pkg, &pkg->installed, LISTFILE);

    fd = open(listfile, O_RDONLY);
    if (fd < 0)
      continue;

    memset(fiemap_buf, 0, sizeof(fiemap_buf));
    fm->fm_start = 0;
    fm->fm_length = fs.f_bsize;
    fm->fm_flags = 0;
    fm->fm_extent_count = DPKG_FIEMAP_EXTENTS;

    if (ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fm) == 0)
      pkg->files_list_phys_offs = fm->fm_extents[0].fe_physical;

    close(fd);
  }

  pkg_array_sort(array, pkg_sorter_by_files_list_phys_offs);
}
#elif defined(HAVE_POSIX_FADVISE)
static void
pkg_files_optimize_load(struct pkg_array *array)
{
  int i;

  /* Ask the kernel to start preloading the list files, so as to get a
   * boost when later we actually load them. */
  for (i = 0; i < array->n_pkgs; i++) {
    struct pkginfo *pkg = array->pkgs[i];
    const char *listfile;
    int fd;

    listfile = pkg_infodb_get_file(pkg, &pkg->installed, LISTFILE);

    fd = open(listfile, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
      posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
      close(fd);
    }
  }
}
#else
static void
pkg_files_optimize_load(struct pkg_array *array)
{
}
#endif

void ensure_allinstfiles_available(void) {
  struct pkg_array array;
  struct progress progress;
  int i;

  if (allpackagesdone) return;
  if (saidread < PKG_FILESDB_LOAD_DONE) {
    int max = pkg_hash_count_pkg();

    saidread = PKG_FILESDB_LOAD_INPROGRESS;
    progress_init(&progress, _("(Reading database ... "), max);
  }

  pkg_array_init_from_hash(&array);

  pkg_files_optimize_load(&array);

  for (i = 0; i < array.n_pkgs; i++) {
    struct pkginfo *pkg = array.pkgs[i];

    ensure_packagefiles_available(pkg);

    if (saidread == PKG_FILESDB_LOAD_INPROGRESS)
      progress_step(&progress);
  }

  pkg_array_destroy(&array);

  allpackagesdone = true;

  if (saidread == PKG_FILESDB_LOAD_INPROGRESS) {
    progress_done(&progress);
    printf(P_("%d file or directory currently installed.)\n",
              "%d files and directories currently installed.)\n",
              fsys_hash_entries()),
           fsys_hash_entries());
    saidread = PKG_FILESDB_LOAD_DONE;
  }
}

void ensure_allinstfiles_available_quiet(void) {
  saidread = PKG_FILESDB_LOAD_DONE;
  ensure_allinstfiles_available();
}

/*
 * If mask is nonzero, will not write any file whose fsys_namenode
 * has any flag bits set in mask.
 */
void
write_filelist_except(struct pkginfo *pkg, struct pkgbin *pkgbin,
                      struct fsys_namenode_list *list, enum fsys_namenode_flags mask)
{
  struct atomic_file *file;
  struct fsys_namenode_list *node;
  const char *listfile;

  listfile = pkg_infodb_get_file(pkg, pkgbin, LISTFILE);

  file = atomic_file_new(listfile, 0);
  atomic_file_open(file);

  for (node = list; node; node = node->next) {
    if (!(mask && (node->namenode->flags & mask))) {
      fputs(node->namenode->name, file->fp);
      putc('\n', file->fp);
    }
  }

  atomic_file_sync(file);
  atomic_file_close(file);
  atomic_file_commit(file);
  atomic_file_free(file);

  dir_sync_path(pkg_infodb_get_dir());

  note_must_reread_files_inpackage(pkg);
}
