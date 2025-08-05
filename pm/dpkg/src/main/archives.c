/*
 * dpkg - main program for package management
 * archives.c - actions that process archive files, mainly unpack
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2007-2015 Guillem Jover <guillem@debian.org>
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
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
#include <sys/time.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <obstack.h>
#define obstack_chunk_alloc m_malloc
#define obstack_chunk_free free

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/path.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/file.h>
#include <dpkg/treewalk.h>
#include <dpkg/tarfn.h>
#include <dpkg/options.h>
#include <dpkg/triglib.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"
#include "archives.h"
#include "filters.h"

static inline void
fd_writeback_init(int fd)
{
  /* Ignore the return code as it should be considered equivalent to an
   * asynchronous hint for the kernel, we are doing an fsync() later on
   * anyway. */
#if defined(SYNC_FILE_RANGE_WRITE)
  sync_file_range(fd, 0, 0, SYNC_FILE_RANGE_WRITE);
#elif defined(HAVE_POSIX_FADVISE)
  posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
#endif
}

static struct obstack tar_pool;
static bool tar_pool_init = false;

/**
 * Allocate memory from the tar memory pool.
 */
static void *
tar_pool_alloc(size_t size)
{
  if (!tar_pool_init) {
    obstack_init(&tar_pool);
    tar_pool_init = true;
  }

  /* cppcheck-suppress[nullPointerArithmetic]:
   * False positive, imported module. */
  return obstack_alloc(&tar_pool, size);
}

/**
 * Free memory from the tar memory pool.
 */
static void
tar_pool_free(void *ptr)
{
  obstack_free(&tar_pool, ptr);
}

/**
 * Release the tar memory pool.
 */
static void
tar_pool_release(void)
{
  if (tar_pool_init) {
    /* cppcheck-suppress[nullPointerArithmetic,pointerLessThanZero]:
     * False positive, imported module. */
    obstack_free(&tar_pool, NULL);
    tar_pool_init = false;
  }
}

struct fsys_namenode_list *
tar_fsys_namenode_queue_push(struct fsys_namenode_queue *queue,
                            struct fsys_namenode *namenode)
{
  struct fsys_namenode_list *node;

  node = tar_pool_alloc(sizeof(*node));
  node->namenode = namenode;
  node->next = NULL;

  *queue->tail = node;
  queue->tail = &node->next;

  return node;
}

static void
tar_fsys_namenode_queue_pop(struct fsys_namenode_queue *queue,
                           struct fsys_namenode_list **tail_prev,
                           struct fsys_namenode_list *node)
{
  tar_pool_free(node);
  queue->tail = tail_prev;
  *tail_prev = NULL;
}

/**
 * Check if a file or directory will save a package from disappearance.
 *
 * A package can only be saved by a file or directory which is part
 * only of itself - it must be neither part of the new package being
 * installed nor part of any 3rd package (this is important so that
 * shared directories don't stop packages from disappearing).
 */
bool
filesavespackage(struct fsys_namenode_list *file,
                 struct pkginfo *pkgtobesaved,
                 struct pkginfo *pkgbeinginstalled)
{
  struct fsys_node_pkgs_iter *iter;
  struct pkginfo *thirdpkg;

  debug(dbg_eachfiledetail, "filesavespackage file '%s' package %s",
        file->namenode->name, pkg_name(pkgtobesaved, pnaw_always));

  /* If the file is a contended one and it's overridden by either
   * the package we're considering disappearing or the package
   * we're installing then they're not actually the same file, so
   * we can't disappear the package - it is saved by this file. */
  if (file->namenode->divert && file->namenode->divert->useinstead) {
    struct pkgset *divpkgset;

    divpkgset = file->namenode->divert->pkgset;
    if (divpkgset == pkgtobesaved->set || divpkgset == pkgbeinginstalled->set) {
      debug(dbg_eachfiledetail,"filesavespackage ... diverted -- save!");
      return true;
    }
  }
  /* Is the file in the package being installed? If so then it can't save. */
  if (file->namenode->flags & FNNF_NEW_INARCHIVE) {
    debug(dbg_eachfiledetail,"filesavespackage ... in new archive -- no save");
    return false;
  }
  /* Look for a 3rd package which can take over the file (in case
   * it's a directory which is shared by many packages. */
  iter = fsys_node_pkgs_iter_new(file->namenode);
  while ((thirdpkg = fsys_node_pkgs_iter_next(iter))) {
    debug(dbg_eachfiledetail, "filesavespackage ... also in %s",
          pkg_name(thirdpkg, pnaw_always));

    /* Is this not the package being installed or the one being
     * checked for disappearance? */
    if (thirdpkg == pkgbeinginstalled || thirdpkg == pkgtobesaved)
      continue;

    /* A Multi-Arch: same package can share files and their presence in a
     * third package of the same set is not a sign that we can get rid of
     * it. */
    if (pkgtobesaved->installed.multiarch == PKG_MULTIARCH_SAME &&
        thirdpkg->set == pkgtobesaved->set)
      continue;

    debug(dbg_eachfiledetail,"filesavespackage ...  is 3rd package");

    /* If !files_list_valid then we have already disappeared this one,
     * so we should not try to make it take over this shared directory. */
    if (!thirdpkg->files_list_valid) {
      debug(dbg_eachfiledetail, "process_archive ... already disappeared!");
      continue;
    }

    /* We've found a package that can take this file. */
    debug(dbg_eachfiledetail, "filesavespackage ...  taken -- no save");
    fsys_node_pkgs_iter_free(iter);
    return false;
  }
  fsys_node_pkgs_iter_free(iter);

  debug(dbg_eachfiledetail, "filesavespackage ... not taken -- save !");
  return true;
}

static void
md5hash_prev_conffile(struct pkginfo *pkg, char *oldhash, const char *oldname,
                      struct fsys_namenode *namenode)
{
  struct pkginfo *otherpkg;
  struct conffile *conff;

  debug(dbg_conffdetail, "tarobject looking for shared conffile %s",
        namenode->name);

  for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
    if (otherpkg == pkg)
      continue;
    /* If we are reinstalling, even if the other package is only unpacked,
     * we can always make use of the Conffiles hash value from an initial
     * installation, if that happened at all. */
    if (otherpkg->status <= PKG_STAT_UNPACKED &&
        dpkg_version_compare(&otherpkg->installed.version,
                             &otherpkg->configversion) != 0)
      continue;
    for (conff = otherpkg->installed.conffiles; conff; conff = conff->next) {
      if (conffile_is_disappearing(conff))
        continue;
      if (strcmp(conff->name, namenode->name) == 0)
        break;
    }
    if (conff) {
      strcpy(oldhash, conff->hash);
      debug(dbg_conffdetail,
            "tarobject found shared conffile, from pkg %s (%s); digest=%s",
            pkg_name(otherpkg, pnaw_always),
            pkg_status_name(otherpkg), oldhash);
      break;
    }
  }

  /* If no package was found with a valid Conffiles field, we make the
   * risky assumption that the hash of the current .dpkg-new file is
   * the one of the previously unpacked package. */
  if (otherpkg == NULL) {
    md5hash(pkg, oldhash, oldname);
    debug(dbg_conffdetail,
          "tarobject found shared conffile, from disk; digest=%s", oldhash);
  }
}

void cu_pathname(int argc, void **argv) {
  path_remove_tree((char*)(argv[0]));
}

int
tarfileread(struct tar_archive *tar, char *buf, int len)
{
  struct tarcontext *tc = (struct tarcontext *)tar->ctx;
  int n;

  n = fd_read(tc->backendpipe, buf, len);
  if (n < 0)
    ohshite(_("error reading from dpkg-deb pipe"));
  return n;
}

static void
tarobject_skip_padding(struct tarcontext *tc, struct tar_entry *te)
{
  struct dpkg_error err;
  size_t remainder;

  remainder = te->size % TARBLKSZ;
  if (remainder == 0)
    return;

  if (fd_skip(tc->backendpipe, TARBLKSZ - remainder, &err) < 0)
    ohshit(_("cannot skip padding for file '%.255s': %s"), te->name, err.str);
}

static void
tarobject_skip_entry(struct tarcontext *tc, struct tar_entry *ti)
{
  /* We need to advance the tar file to the next object, so read the
   * file data and set it to oblivion. */
  if (ti->type == TAR_FILETYPE_FILE) {
    struct dpkg_error err;

    if (fd_skip(tc->backendpipe, ti->size, &err) < 0)
      ohshit(_("cannot skip file '%.255s' (replaced or excluded?) from pipe: %s"),
             ti->name, err.str);
    tarobject_skip_padding(tc, ti);
  }
}

struct varbuf_state fname_state;
struct varbuf_state fnametmp_state;
struct varbuf_state fnamenew_state;
struct varbuf fnamevb;
struct varbuf fnametmpvb;
struct varbuf fnamenewvb;
struct pkg_deconf_list *deconfigure = NULL;

static time_t currenttime;

static int
does_replace(struct pkginfo *new_pkg, struct pkgbin *new_pkgbin,
             struct pkginfo *old_pkg, struct pkgbin *old_pkgbin)
{
  struct dependency *dep;

  debug(dbg_depcon,"does_replace new=%s old=%s (%s)",
        pkgbin_name(new_pkg, new_pkgbin, pnaw_always),
        pkgbin_name(old_pkg, old_pkgbin, pnaw_always),
        versiondescribe_c(&old_pkgbin->version, vdew_always));
  for (dep = new_pkgbin->depends; dep; dep = dep->next) {
    if (dep->type != dep_replaces || dep->list->ed != old_pkg->set)
      continue;
    debug(dbg_depcondetail,"does_replace ... found old, version %s",
          versiondescribe_c(&dep->list->version,vdew_always));
    if (!versionsatisfied(old_pkgbin, dep->list))
      continue;
    /* The test below can only trigger if dep_replaces start having
     * arch qualifiers different from “any”. */
    if (!archsatisfied(old_pkgbin, dep->list))
      continue;
    debug(dbg_depcon,"does_replace ... yes");
    return true;
  }
  debug(dbg_depcon,"does_replace ... no");
  return false;
}

static void
tarobject_extract(struct tarcontext *tc, struct tar_entry *te,
                  const char *path, struct file_stat *st,
                  struct fsys_namenode *namenode)
{
  static struct varbuf hardlinkfn;
  static int fd;

  struct dpkg_error err;
  struct fsys_namenode *linknode;
  char *newhash;
  int rc;

  switch (te->type) {
  case TAR_FILETYPE_FILE:
    /* We create the file with mode 0 to make sure nobody can do anything with
     * it until we apply the proper mode, which might be a statoverride. */
    fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0);
    if (fd < 0)
      ohshite(_("unable to create '%.255s' (while processing '%.255s')"),
              path, te->name);
    push_cleanup(cu_closefd, ehflag_bombout, 1, &fd);
    debug(dbg_eachfiledetail, "tarobject file open size=%jd",
          (intmax_t)te->size);

    /* We try to tell the filesystem how much disk space we are going to
     * need to let it reduce fragmentation and possibly improve performance,
     * as we do know the size beforehand. */
    fd_allocate_size(fd, 0, te->size);

    newhash = nfmalloc(MD5HASHLEN + 1);
    if (fd_fd_copy_and_md5(tc->backendpipe, fd, newhash, te->size, &err) < 0)
      ohshit(_("cannot copy extracted data for '%.255s' to '%.255s': %s"),
             te->name, fnamenewvb.buf, err.str);
    namenode->newhash = newhash;
    debug(dbg_eachfiledetail, "tarobject file digest=%s", namenode->newhash);

    tarobject_skip_padding(tc, te);

    fd_writeback_init(fd);

    if (namenode->statoverride)
      debug(dbg_eachfile, "tarobject ... stat override, uid=%d, gid=%d, mode=%04o",
            namenode->statoverride->uid,
            namenode->statoverride->gid,
            namenode->statoverride->mode);
    rc = fchown(fd, st->uid, st->gid);
    if (forcible_nonroot_error(rc))
      ohshite(_("error setting ownership of '%.255s'"), te->name);
    rc = fchmod(fd, st->mode & ~S_IFMT);
    if (forcible_nonroot_error(rc))
      ohshite(_("error setting permissions of '%.255s'"), te->name);

    /* Postpone the fsync, to try to avoid massive I/O degradation. */
    if (!in_force(FORCE_UNSAFE_IO))
      namenode->flags |= FNNF_DEFERRED_FSYNC;

    pop_cleanup(ehflag_normaltidy); /* fd = open(path) */
    if (close(fd))
      ohshite(_("error closing/writing '%.255s'"), te->name);
    break;
  case TAR_FILETYPE_FIFO:
    if (mkfifo(path, 0))
      ohshite(_("error creating pipe '%.255s'"), te->name);
    debug(dbg_eachfiledetail, "tarobject fifo");
    break;
  case TAR_FILETYPE_CHARDEV:
    if (mknod(path, S_IFCHR, te->dev))
      ohshite(_("error creating device '%.255s'"), te->name);
    debug(dbg_eachfiledetail, "tarobject chardev");
    break;
  case TAR_FILETYPE_BLOCKDEV:
    if (mknod(path, S_IFBLK, te->dev))
      ohshite(_("error creating device '%.255s'"), te->name);
    debug(dbg_eachfiledetail, "tarobject blockdev");
    break;
  case TAR_FILETYPE_HARDLINK:
    varbuf_set_str(&hardlinkfn, dpkg_fsys_get_dir());
    linknode = fsys_hash_find_node(te->linkname, FHFF_NONE);
    varbuf_add_str(&hardlinkfn,
                   namenodetouse(linknode, tc->pkg, &tc->pkg->available)->name);
    if (linknode->flags & (FNNF_DEFERRED_RENAME | FNNF_NEW_CONFF))
      varbuf_add_str(&hardlinkfn, DPKGNEWEXT);
    if (link(hardlinkfn.buf, path))
      ohshite(_("error creating hard link '%.255s'"), te->name);
    namenode->newhash = linknode->newhash;
    debug(dbg_eachfiledetail, "tarobject hardlink digest=%s", namenode->newhash);
    break;
  case TAR_FILETYPE_SYMLINK:
    /* We've already checked for an existing directory. */
    if (symlink(te->linkname, path))
      ohshite(_("error creating symbolic link '%.255s'"), te->name);
    debug(dbg_eachfiledetail, "tarobject symlink creating");
    break;
  case TAR_FILETYPE_DIR:
    /* We've already checked for an existing directory. */
    if (mkdir(path, 0))
      ohshite(_("error creating directory '%.255s'"), te->name);
    debug(dbg_eachfiledetail, "tarobject directory creating");
    break;
  default:
    internerr("unknown tar type '%d', but already checked", te->type);
  }
}

static void
tarobject_hash(struct tarcontext *tc, struct tar_entry *te,
               struct fsys_namenode *namenode)
{
  if (te->type == TAR_FILETYPE_FILE) {
    struct dpkg_error err;
    char *newhash;

    newhash = nfmalloc(MD5HASHLEN + 1);
    if (fd_md5(tc->backendpipe, newhash, te->size, &err) < 0)
      ohshit(_("cannot compute MD5 digest for file '%.255s' in tar archive: %s"),
             te->name, err.str);
    tarobject_skip_padding(tc, te);

    namenode->newhash = newhash;
    debug(dbg_eachfiledetail, "tarobject file digest=%s", namenode->newhash);
  } else if (te->type == TAR_FILETYPE_HARDLINK) {
    struct fsys_namenode *linknode;

    linknode = fsys_hash_find_node(te->linkname, FHFF_NONE);
    namenode->newhash = linknode->newhash;
    debug(dbg_eachfiledetail, "tarobject hardlink digest=%s", namenode->newhash);
  }
}

static void
tarobject_set_mtime(struct tar_entry *te, const char *path)
{
  struct timeval tv[2];

  tv[0].tv_sec = currenttime;
  tv[0].tv_usec = 0;
  tv[1].tv_sec = te->mtime;
  tv[1].tv_usec = 0;

  if (te->type == TAR_FILETYPE_SYMLINK) {
#ifdef HAVE_LUTIMES
    if (lutimes(path, tv) && errno != ENOSYS)
      ohshite(_("error setting timestamps of '%.255s'"), path);
#endif
  } else {
    if (utimes(path, tv))
      ohshite(_("error setting timestamps of '%.255s'"), path);
  }
}

static void
tarobject_set_perms(struct tar_entry *te, const char *path, struct file_stat *st)
{
  int rc;

  if (te->type == TAR_FILETYPE_FILE)
    return; /* Already handled using the file descriptor. */

  if (te->type == TAR_FILETYPE_SYMLINK) {
    rc = lchown(path, st->uid, st->gid);
    if (forcible_nonroot_error(rc))
      ohshite(_("error setting ownership of symlink '%.255s'"), path);
  } else {
    rc = chown(path, st->uid, st->gid);
    if (forcible_nonroot_error(rc))
      ohshite(_("error setting ownership of '%.255s'"), path);
    rc = chmod(path, st->mode & ~S_IFMT);
    if (forcible_nonroot_error(rc))
      ohshite(_("error setting permissions of '%.255s'"), path);
  }
}

static void
tarobject_set_se_context(const char *matchpath, const char *path, mode_t mode)
{
  dpkg_selabel_set_context(matchpath, path, mode);
}

static void
tarobject_matches(struct tarcontext *tc,
                  const char *fn_old, struct stat *stab, char *oldhash,
                  const char *fn_new, struct tar_entry *te,
                  struct fsys_namenode *namenode)
{
  struct varbuf linkname = VARBUF_INIT;
  ssize_t linksize;
  bool linkmatch;

  debug(dbg_eachfiledetail, "tarobject matches on-disk object?");

  switch (te->type) {
  case TAR_FILETYPE_DIR:
    /* Nothing to check for a new directory. */
    return;
  case TAR_FILETYPE_SYMLINK:
    /* Symlinks to existing dirs have already been dealt with, only
     * remain real symlinks where we can compare the target. */
    if (!S_ISLNK(stab->st_mode))
      break;
    linksize = file_readlink(fn_old, &linkname, stab->st_size);
    if (linksize < 0)
      ohshite(_("unable to read link '%.255s'"), fn_old);
    else if (linksize > stab->st_size)
      ohshit(_("symbolic link '%.250s' size has changed from %jd to %zd"),
             fn_old, (intmax_t)stab->st_size, linksize);
    else if (linksize < stab->st_size)
      warning(_("symbolic link '%.250s' size has changed from %jd to %zd"),
             fn_old, (intmax_t)stab->st_size, linksize);
    linkmatch = strcmp(linkname.buf, te->linkname) == 0;
    varbuf_destroy(&linkname);
    if (linkmatch)
      return;
    break;
  case TAR_FILETYPE_CHARDEV:
    if (S_ISCHR(stab->st_mode) && stab->st_rdev == te->dev)
      return;
    break;
  case TAR_FILETYPE_BLOCKDEV:
    if (S_ISBLK(stab->st_mode) && stab->st_rdev == te->dev)
      return;
    break;
  case TAR_FILETYPE_FIFO:
    if (S_ISFIFO(stab->st_mode))
      return;
    break;
  case TAR_FILETYPE_HARDLINK:
    /* Fall through. */
  case TAR_FILETYPE_FILE:
    /* Only check metadata for non-conffiles. */
    if (!(namenode->flags & FNNF_NEW_CONFF) &&
        !(S_ISREG(stab->st_mode) && te->size == stab->st_size))
      break;
    if (strcmp(oldhash, namenode->newhash) == 0)
      return;
    break;
  default:
    internerr("unknown tar type '%d', but already checked", te->type);
  }

  forcibleerr(FORCE_OVERWRITE,
              _("trying to overwrite shared '%.250s', which is different "
                "from other instances of package %.250s"),
              namenode->name, pkg_name(tc->pkg, pnaw_nonambig));
}

void setupfnamevbs(const char *filename) {
  varbuf_rollback(&fname_state);
  varbuf_add_str(&fnamevb, filename);

  varbuf_rollback(&fnametmp_state);
  varbuf_add_str(&fnametmpvb, filename);
  varbuf_add_str(&fnametmpvb, DPKGTEMPEXT);

  varbuf_rollback(&fnamenew_state);
  varbuf_add_str(&fnamenewvb, filename);
  varbuf_add_str(&fnamenewvb, DPKGNEWEXT);

  debug(dbg_eachfiledetail, "setupvnamevbs main='%s' tmp='%s' new='%s'",
        fnamevb.buf, fnametmpvb.buf, fnamenewvb.buf);
}

static bool
linktosameexistingdir(const struct tar_entry *ti, const char *fname,
                      struct varbuf *symlinkfn)
{
  struct stat oldstab, newstab;
  int statr;

  statr= stat(fname, &oldstab);
  if (statr) {
    if (!(errno == ENOENT || errno == ELOOP || errno == ENOTDIR))
      ohshite(_("failed to stat (dereference) existing symlink '%.250s'"),
              fname);
    return false;
  }
  if (!S_ISDIR(oldstab.st_mode))
    return false;

  /* But is it to the same dir? */
  if (ti->linkname[0] == '/') {
    varbuf_set_str(symlinkfn, dpkg_fsys_get_dir());
  } else {
    const char *lastslash;

    lastslash= strrchr(fname, '/');
    if (lastslash == NULL)
      internerr("tar entry filename '%s' does not contain '/'", fname);
    varbuf_set_buf(symlinkfn, fname, (lastslash - fname) + 1);
  }
  varbuf_add_str(symlinkfn, ti->linkname);

  statr= stat(symlinkfn->buf, &newstab);
  if (statr) {
    if (!(errno == ENOENT || errno == ELOOP || errno == ENOTDIR))
      ohshite(_("failed to stat (dereference) proposed new symlink target"
                " '%.250s' for symlink '%.250s'"), symlinkfn->buf, fname);
    return false;
  }
  if (!S_ISDIR(newstab.st_mode))
    return false;
  if (newstab.st_dev != oldstab.st_dev ||
      newstab.st_ino != oldstab.st_ino)
    return false;
  return true;
}

int
tarobject(struct tar_archive *tar, struct tar_entry *ti)
{
  static struct varbuf conffderefn, symlinkfn;
  const char *usename;
  struct fsys_namenode *namenode, *usenode;

  struct conffile *conff;
  struct tarcontext *tc = tar->ctx;
  bool existingdir, keepexisting;
  bool refcounting;
  char oldhash[MD5HASHLEN + 1];
  int statr;
  struct stat stab, stabtmp;
  struct file_stat nodestat;
  struct fsys_namenode_list *nifd, **oldnifd;
  struct pkgset *divpkgset;
  struct pkginfo *otherpkg;

  tar_entry_update_from_system(ti);

  /* Perform some sanity checks on the tar entry. */
  if (strchr(ti->name, '\n'))
    ohshit(_("newline not allowed in archive object name '%.255s'"), ti->name);

  namenode = fsys_hash_find_node(ti->name, FHFF_NONE);

  if (namenode->flags & FNNF_RM_CONFF_ON_UPGRADE)
    ohshit(_("conffile '%s' marked for removal on upgrade, shipped in package"),
           ti->name);

  /* Append to list of files.
   * The trailing ‘/’ put on the end of names in tarfiles has already
   * been stripped by tar_extractor(). */
  oldnifd = tc->newfiles_queue->tail;
  nifd = tar_fsys_namenode_queue_push(tc->newfiles_queue, namenode);
  nifd->namenode->flags |= FNNF_NEW_INARCHIVE;

  debug(dbg_eachfile,
        "tarobject ti->name='%s' mode=%lo owner=%u:%u type=%d(%c)"
        " ti->linkname='%s' namenode='%s' flags=%o instead='%s'",
        ti->name, (long)ti->stat.mode,
        (unsigned)ti->stat.uid, (unsigned)ti->stat.gid,
        ti->type,
        ti->type >= '0' && ti->type <= '6' ? "-hlcbdp"[ti->type - '0'] : '?',
        ti->linkname,
        nifd->namenode->name, nifd->namenode->flags,
        nifd->namenode->divert && nifd->namenode->divert->useinstead
        ? nifd->namenode->divert->useinstead->name : "<none>");

  if (nifd->namenode->divert && nifd->namenode->divert->camefrom) {
    divpkgset = nifd->namenode->divert->pkgset;

    if (divpkgset) {
      forcibleerr(FORCE_OVERWRITE_DIVERTED,
                  _("trying to overwrite '%.250s', which is the "
                    "diverted version of '%.250s' (package: %.100s)"),
                  nifd->namenode->name, nifd->namenode->divert->camefrom->name,
                  divpkgset->name);
    } else {
      forcibleerr(FORCE_OVERWRITE_DIVERTED,
                  _("trying to overwrite '%.250s', which is the "
                    "diverted version of '%.250s'"),
                  nifd->namenode->name, nifd->namenode->divert->camefrom->name);
    }
  }

  if (nifd->namenode->statoverride) {
    nodestat = *nifd->namenode->statoverride;
    nodestat.mode |= ti->stat.mode & S_IFMT;
  } else {
    nodestat = ti->stat;
  }

  usenode = namenodetouse(nifd->namenode, tc->pkg, &tc->pkg->available);
  usename = usenode->name;

  trig_file_activate(usenode, tc->pkg);

  if (nifd->namenode->flags & FNNF_NEW_CONFF) {
    /* If it's a conffile we have to extract it next to the installed
     * version (i.e. we do the usual link-following). */
    if (conffderef(tc->pkg, &conffderefn, usename))
      usename= conffderefn.buf;
    debug(dbg_conff, "tarobject FNNF_NEW_CONFF deref='%s'", usename);
  }

  setupfnamevbs(usename);

  statr= lstat(fnamevb.buf,&stab);
  if (statr) {
    /* The lstat failed. */
    if (errno != ENOENT && errno != ENOTDIR)
      ohshite(_("unable to stat '%.255s' (which was about to be installed)"),
              ti->name);
    /* OK, so it doesn't exist.
     * However, it's possible that we were in the middle of some other
     * backup/restore operation and were rudely interrupted.
     * So, we see if we have .dpkg-tmp, and if so we restore it. */
    if (rename(fnametmpvb.buf,fnamevb.buf)) {
      /* Trying to remove a directory or a file on a read-only filesystem,
       * even if non-existent, always returns EROFS. */
      if (errno == EROFS) {
        /* If the file does not exist the access() function will remap the
         * EROFS into an ENOENT, otherwise restore EROFS to fail with that. */
        if (access(fnametmpvb.buf, F_OK) == 0)
          errno = EROFS;
      }

      if (errno != ENOENT && errno != ENOTDIR)
        ohshite(_("unable to clean up mess surrounding '%.255s' before "
                  "installing another version"), ti->name);
      debug(dbg_eachfiledetail,"tarobject nonexistent");
    } else {
      debug(dbg_eachfiledetail,"tarobject restored tmp to main");
      statr= lstat(fnamevb.buf,&stab);
      if (statr)
        ohshite(_("unable to stat restored '%.255s' before installing"
                  " another version"), ti->name);
    }
  } else {
    debug(dbg_eachfiledetail,"tarobject already exists");
  }

  /* Check to see if it's a directory or link to one and we don't need to
   * do anything. This has to be done now so that we don't die due to
   * a file overwriting conflict. */
  existingdir = false;
  switch (ti->type) {
  case TAR_FILETYPE_SYMLINK:
    /* If it's already an existing directory, do nothing. */
    if (!statr && S_ISDIR(stab.st_mode)) {
      debug(dbg_eachfiledetail, "tarobject symlink exists as directory");
      existingdir = true;
    } else if (!statr && S_ISLNK(stab.st_mode)) {
      if (linktosameexistingdir(ti, fnamevb.buf, &symlinkfn))
        existingdir = true;
    }
    break;
  case TAR_FILETYPE_DIR:
    /* If it's already an existing directory, do nothing. */
    if (!stat(fnamevb.buf,&stabtmp) && S_ISDIR(stabtmp.st_mode)) {
      debug(dbg_eachfiledetail, "tarobject directory exists");
      existingdir = true;
    }
    break;
  case TAR_FILETYPE_FILE:
  case TAR_FILETYPE_CHARDEV:
  case TAR_FILETYPE_BLOCKDEV:
  case TAR_FILETYPE_FIFO:
  case TAR_FILETYPE_HARDLINK:
    break;
  default:
    ohshit(_("archive contained object '%.255s' of unknown type 0x%x"),
           ti->name, ti->type);
  }

  keepexisting = false;
  refcounting = false;
  if (!existingdir) {
    struct fsys_node_pkgs_iter *iter;

    iter = fsys_node_pkgs_iter_new(nifd->namenode);
    while ((otherpkg = fsys_node_pkgs_iter_next(iter))) {
      if (otherpkg == tc->pkg)
        continue;
      debug(dbg_eachfile, "tarobject ... found in %s",
            pkg_name(otherpkg, pnaw_always));

      /* A pkgset can share files between its instances. Overwriting
       * is allowed when they are not getting in sync, otherwise the
       * file content must match the installed file. */
      if (otherpkg->set == tc->pkg->set &&
          otherpkg->installed.multiarch == PKG_MULTIARCH_SAME &&
          tc->pkg->available.multiarch == PKG_MULTIARCH_SAME) {
        if (statr == 0 && tc->pkgset_getting_in_sync)
          refcounting = true;
        debug(dbg_eachfiledetail, "tarobject ... shared with %s %s (syncing=%d)",
              pkg_name(otherpkg, pnaw_always),
              versiondescribe_c(&otherpkg->installed.version, vdew_nonambig),
              tc->pkgset_getting_in_sync);
        continue;
      }

      if (nifd->namenode->divert && nifd->namenode->divert->useinstead) {
        /* Right, so we may be diverting this file. This makes the conflict
         * OK iff one of us is the diverting package (we don't need to
         * check for both being the diverting package, obviously). */
        divpkgset = nifd->namenode->divert->pkgset;
        debug(dbg_eachfile, "tarobject ... diverted, divpkgset=%s",
              divpkgset ? divpkgset->name : "<none>");
        if (otherpkg->set == divpkgset || tc->pkg->set == divpkgset)
          continue;
      }

      /* If the new object is a directory and the previous object does
       * not exist assume it's also a directory and skip further checks.
       * XXX: Ideally with more information about the installed files we
       * could perform more clever checks. */
      if (statr != 0 && ti->type == TAR_FILETYPE_DIR) {
        debug(dbg_eachfile, "tarobject ... assuming shared directory");
        continue;
      }

      ensure_package_clientdata(otherpkg);

      /* Nope? Hmm, file conflict, perhaps. Check Replaces. */
      switch (otherpkg->clientdata->replacingfilesandsaid) {
      case 2:
        keepexisting = true;
        /* Fall through. */
      case 1:
        continue;
      }

      /* Is the package with the conflicting file in the “config files only”
       * state? If so it must be a config file and we can silently take it
       * over. */
      if (otherpkg->status == PKG_STAT_CONFIGFILES)
        continue;

      /* Perhaps we're removing a conflicting package? */
      if (otherpkg->clientdata->istobe == PKG_ISTOBE_REMOVE)
        continue;

      /* Is the file an obsolete conffile in the other package
       * and a conffile in the new package? */
      if ((nifd->namenode->flags & FNNF_NEW_CONFF) &&
          !statr && S_ISREG(stab.st_mode)) {
        for (conff = otherpkg->installed.conffiles;
             conff;
             conff = conff->next) {
          if (!(conff->flags & CONFFILE_OBSOLETE))
            continue;
          if (strcmp(conff->name, nifd->namenode->name) == 0)
            break;
        }
        if (conff) {
          debug(dbg_eachfiledetail, "tarobject other's obsolete conffile");
          /* process_archive() will have copied its hash already. */
          continue;
        }
      }

      if (does_replace(tc->pkg, &tc->pkg->available,
                       otherpkg, &otherpkg->installed)) {
        printf(_("Replacing files in old package %s (%s) ...\n"),
               pkg_name(otherpkg, pnaw_nonambig),
               versiondescribe(&otherpkg->installed.version, vdew_nonambig));
        otherpkg->clientdata->replacingfilesandsaid = 1;
      } else if (does_replace(otherpkg, &otherpkg->installed,
                              tc->pkg, &tc->pkg->available)) {
        printf(_("Replaced by files in installed package %s (%s) ...\n"),
               pkg_name(otherpkg, pnaw_nonambig),
               versiondescribe(&otherpkg->installed.version, vdew_nonambig));
        otherpkg->clientdata->replacingfilesandsaid = 2;
        nifd->namenode->flags &= ~FNNF_NEW_INARCHIVE;
        keepexisting = true;
      } else {
        /* At this point we are replacing something without a Replaces. */
        if (!statr && S_ISDIR(stab.st_mode)) {
          forcibleerr(FORCE_OVERWRITE_DIR,
                      _("trying to overwrite directory '%.250s' "
                        "in package %.250s (%.250s) with nondirectory"),
                      nifd->namenode->name, pkg_name(otherpkg, pnaw_nonambig),
                      versiondescribe(&otherpkg->installed.version,
                                      vdew_nonambig));
        } else {
          forcibleerr(FORCE_OVERWRITE,
                      _("trying to overwrite '%.250s', "
                        "which is also in package %.250s (%.250s)"),
                      nifd->namenode->name, pkg_name(otherpkg, pnaw_nonambig),
                      versiondescribe(&otherpkg->installed.version,
                                      vdew_nonambig));
        }
      }
    }
    fsys_node_pkgs_iter_free(iter);
  }

  if (keepexisting) {
    if (nifd->namenode->flags & FNNF_NEW_CONFF)
      nifd->namenode->flags |= FNNF_OBS_CONFF;
    tar_fsys_namenode_queue_pop(tc->newfiles_queue, oldnifd, nifd);
    tarobject_skip_entry(tc, ti);
    return 0;
  }

  if (filter_should_skip(ti)) {
    nifd->namenode->flags &= ~FNNF_NEW_INARCHIVE;
    nifd->namenode->flags |= FNNF_FILTERED;
    tarobject_skip_entry(tc, ti);

    return 0;
  }

  if (existingdir)
    return 0;

  /* Compute the hash of the previous object, before we might replace it
   * with the new version on forced overwrites. */
  if (refcounting) {
    debug(dbg_eachfiledetail, "tarobject computing on-disk file '%s' digest, refcounting",
          fnamevb.buf);
    if (nifd->namenode->flags & FNNF_NEW_CONFF) {
      md5hash_prev_conffile(tc->pkg, oldhash, fnamenewvb.buf, nifd->namenode);
    } else if (S_ISREG(stab.st_mode)) {
      md5hash(tc->pkg, oldhash, fnamevb.buf);
    } else {
      strcpy(oldhash, EMPTYHASHFLAG);
    }
  }

  if (refcounting && !in_force(FORCE_OVERWRITE)) {
    /* If we are not forced to overwrite the path and are refcounting,
     * just compute the hash w/o extracting the object. */
    tarobject_hash(tc, ti, nifd->namenode);
  } else {
    /* Now, at this stage we want to make sure neither of .dpkg-new and
     * .dpkg-tmp are hanging around. */
    path_remove_tree(fnamenewvb.buf);
    path_remove_tree(fnametmpvb.buf);

    /* Now we start to do things that we need to be able to undo
     * if something goes wrong. Watch out for the CLEANUP comments to
     * keep an eye on what's installed on the disk at each point. */
    push_cleanup(cu_installnew, ~ehflag_normaltidy, 1, nifd->namenode);

    /*
     * CLEANUP: Now we either have the old file on the disk, or not, in
     * its original filename.
     */

    /* Extract whatever it is as .dpkg-new ... */
    tarobject_extract(tc, ti, fnamenewvb.buf, &nodestat, nifd->namenode);
  }

  /* For shared files, check now if the object matches. */
  if (refcounting)
    tarobject_matches(tc, fnamevb.buf, &stab, oldhash,
                          fnamenewvb.buf, ti, nifd->namenode);

  /* If we didn't extract anything, there's nothing else to do. */
  if (refcounting && !in_force(FORCE_OVERWRITE))
    return 0;

  tarobject_set_perms(ti, fnamenewvb.buf, &nodestat);
  tarobject_set_mtime(ti, fnamenewvb.buf);
  tarobject_set_se_context(fnamevb.buf, fnamenewvb.buf, nodestat.mode);

  /*
   * CLEANUP: Now we have extracted the new object in .dpkg-new (or,
   * if the file already exists as a directory and we were trying to
   * extract a directory or symlink, we returned earlier, so we don't
   * need to worry about that here).
   *
   * The old file is still in the original filename,
   */

  /* First, check to see if it's a conffile. If so we don't install
   * it now - we leave it in .dpkg-new for --configure to take care of. */
  if (nifd->namenode->flags & FNNF_NEW_CONFF) {
    debug(dbg_conffdetail,"tarobject conffile extracted");
    nifd->namenode->flags |= FNNF_ELIDE_OTHER_LISTS;
    return 0;
  }

  /* Now we move the old file out of the way, the backup file will
   * be deleted later. */
  if (statr) {
    /* Don't try to back it up if it didn't exist. */
    debug(dbg_eachfiledetail,"tarobject new - no backup");
  } else {
    if (ti->type == TAR_FILETYPE_DIR || S_ISDIR(stab.st_mode)) {
      /* One of the two is a directory - can't do atomic install. */
      debug(dbg_eachfiledetail,"tarobject directory, nonatomic");
      nifd->namenode->flags |= FNNF_NO_ATOMIC_OVERWRITE;
      if (rename(fnamevb.buf,fnametmpvb.buf))
        ohshite(_("unable to move aside '%.255s' to install new version"),
                ti->name);
    } else if (S_ISLNK(stab.st_mode)) {
      ssize_t linksize;
      int rc;

      /* We can't make a symlink with two hardlinks, so we'll have to
       * copy it. (Pretend that making a copy of a symlink is the same
       * as linking to it.) */
      linksize = file_readlink(fnamevb.buf, &symlinkfn, stab.st_size);
      if (linksize < 0)
        ohshite(_("unable to read link '%.255s'"), ti->name);
      else if (linksize > stab.st_size)
        ohshit(_("symbolic link '%.250s' size has changed from %jd to %zd"),
               fnamevb.buf, (intmax_t)stab.st_size, linksize);
      else if (linksize < stab.st_size)
        warning(_("symbolic link '%.250s' size has changed from %jd to %zd"),
               fnamevb.buf, (intmax_t)stab.st_size, linksize);
      if (symlink(symlinkfn.buf,fnametmpvb.buf))
        ohshite(_("unable to make backup symlink for '%.255s'"), ti->name);
      rc = lchown(fnametmpvb.buf, stab.st_uid, stab.st_gid);
      if (forcible_nonroot_error(rc))
        ohshite(_("unable to chown backup symlink for '%.255s'"), ti->name);
      tarobject_set_se_context(fnamevb.buf, fnametmpvb.buf, stab.st_mode);
    } else {
      debug(dbg_eachfiledetail, "tarobject nondirectory, 'link' backup");
      if (link(fnamevb.buf,fnametmpvb.buf))
        ohshite(_("unable to make backup link of '%.255s' before installing new version"),
                ti->name);
    }
  }

  /*
   * CLEANUP: Now the old file is in .dpkg-tmp, and the new file is still
   * in .dpkg-new.
   */

  if (ti->type == TAR_FILETYPE_FILE || ti->type == TAR_FILETYPE_HARDLINK ||
      ti->type == TAR_FILETYPE_SYMLINK) {
    nifd->namenode->flags |= FNNF_DEFERRED_RENAME;

    debug(dbg_eachfiledetail, "tarobject done and installation deferred");
  } else {
    if (rename(fnamenewvb.buf, fnamevb.buf))
      ohshite(_("unable to install new version of '%.255s'"), ti->name);

    /*
     * CLEANUP: Now the new file is in the destination file, and the
     * old file is in .dpkg-tmp to be cleaned up later. We now need
     * to take a different attitude to cleanup, because we need to
     * remove the new file.
     */

    nifd->namenode->flags |= FNNF_PLACED_ON_DISK;
    nifd->namenode->flags |= FNNF_ELIDE_OTHER_LISTS;

    debug(dbg_eachfiledetail, "tarobject done and installed");
  }

  return 0;
}

#if defined(SYNC_FILE_RANGE_WAIT_BEFORE)
static void
tar_writeback_barrier(struct fsys_namenode_list *files, struct pkginfo *pkg)
{
  struct fsys_namenode_list *cfile;

  for (cfile = files; cfile; cfile = cfile->next) {
    struct fsys_namenode *usenode;
    int fd;

    if (!(cfile->namenode->flags & FNNF_DEFERRED_FSYNC))
      continue;

    usenode = namenodetouse(cfile->namenode, pkg, &pkg->available);

    setupfnamevbs(usenode->name);

    fd = open(fnamenewvb.buf, O_WRONLY);
    if (fd < 0)
      ohshite(_("unable to open '%.255s'"), fnamenewvb.buf);
    /* Ignore the return code as it should be considered equivalent to an
     * asynchronous hint for the kernel, we are doing an fsync() later on
     * anyway. */
    sync_file_range(fd, 0, 0, SYNC_FILE_RANGE_WAIT_BEFORE);
    if (close(fd))
      ohshite(_("error closing/writing '%.255s'"), fnamenewvb.buf);
  }
}
#else
static void
tar_writeback_barrier(struct fsys_namenode_list *files, struct pkginfo *pkg)
{
}
#endif

void
tar_deferred_extract(struct fsys_namenode_list *files, struct pkginfo *pkg)
{
  struct fsys_namenode_list *cfile;
  struct fsys_namenode *usenode;

  tar_writeback_barrier(files, pkg);

  for (cfile = files; cfile; cfile = cfile->next) {
    debug(dbg_eachfile, "deferred extract of '%.255s'", cfile->namenode->name);

    if (!(cfile->namenode->flags & FNNF_DEFERRED_RENAME))
      continue;

    usenode = namenodetouse(cfile->namenode, pkg, &pkg->available);

    setupfnamevbs(usenode->name);

    if (cfile->namenode->flags & FNNF_DEFERRED_FSYNC) {
      int fd;

      debug(dbg_eachfiledetail, "deferred extract needs fsync");

      fd = open(fnamenewvb.buf, O_WRONLY);
      if (fd < 0)
        ohshite(_("unable to open '%.255s'"), fnamenewvb.buf);
      if (fsync(fd))
        ohshite(_("unable to sync file '%.255s'"), fnamenewvb.buf);
      if (close(fd))
        ohshite(_("error closing/writing '%.255s'"), fnamenewvb.buf);

      cfile->namenode->flags &= ~FNNF_DEFERRED_FSYNC;
    }

    debug(dbg_eachfiledetail, "deferred extract needs rename");

    if (rename(fnamenewvb.buf, fnamevb.buf))
      ohshite(_("unable to install new version of '%.255s'"),
              cfile->namenode->name);

    cfile->namenode->flags &= ~FNNF_DEFERRED_RENAME;

    /*
     * CLEANUP: Now the new file is in the destination file, and the
     * old file is in .dpkg-tmp to be cleaned up later. We now need
     * to take a different attitude to cleanup, because we need to
     * remove the new file.
     */

    cfile->namenode->flags |= FNNF_PLACED_ON_DISK;
    cfile->namenode->flags |= FNNF_ELIDE_OTHER_LISTS;

    debug(dbg_eachfiledetail, "deferred extract done and installed");
  }
}

void
enqueue_deconfigure(struct pkginfo *pkg, struct pkginfo *pkg_removal,
                    enum pkgwant reason)
{
  struct pkg_deconf_list *newdeconf;

  ensure_package_clientdata(pkg);
  pkg->clientdata->istobe = PKG_ISTOBE_DECONFIGURE;
  newdeconf = m_malloc(sizeof(*newdeconf));
  newdeconf->next = deconfigure;
  newdeconf->pkg = pkg;
  newdeconf->pkg_removal = pkg_removal;
  newdeconf->reason = reason;
  deconfigure = newdeconf;
}

void
clear_deconfigure_queue(void)
{
  struct pkg_deconf_list *deconf, *deconf_next;

  for (deconf = deconfigure; deconf; deconf = deconf_next) {
    deconf_next = deconf->next;
    free(deconf);
  }
  deconfigure = NULL;
}

/**
 * Try if we can deconfigure the package for installation and queue it if so.
 *
 * This function gets called in the Breaks context, when trying to install
 * a package that might require another to be deconfigured to be able to
 * proceed.
 *
 * First checks whether the pdep is forced.
 *
 * @retval 0 Not possible (why is printed).
 * @retval 1 Deconfiguration queued ok (no message printed).
 * @retval 2 Forced (no deconfiguration needed, why is printed).
 */
static int
try_deconfigure_can(struct pkginfo *pkg, struct deppossi *pdep,
                    struct pkginfo *pkg_install, const char *why)
{
  if (force_breaks(pdep)) {
    warning(_("ignoring dependency problem with installation of %s:\n%s"),
            pkgbin_name(pkg_install, &pkg->available, pnaw_nonambig), why);
    return 2;
  } else if (f_autodeconf) {
    enqueue_deconfigure(pkg, NULL, PKG_WANT_INSTALL);
    return 1;
  } else {
    notice(_("no, cannot proceed with installation of %s (--auto-deconfigure will help):\n%s"),
           pkgbin_name(pkg_install, &pkg->available, pnaw_nonambig), why);
    return 0;
  }
}

/**
 * Try if we can deconfigure the package for removal and queue it if so.
 *
 * This function gets called in the Conflicts+Depends context, when trying
 * to install a package that might require another to be fully removed to
 * be able to proceed.
 *
 * First checks whether the pdep is forced, then if auto-configure is enabled
 * we make sure Essential and Protected are not allowed to be removed unless
 * forced.
 *
 * @retval 0 Not possible (why is printed).
 * @retval 1 Deconfiguration queued ok (no message printed).
 * @retval 2 Forced (no deconfiguration needed, why is printed).
 */
static int
try_remove_can(struct deppossi *pdep,
               struct pkginfo *pkg_removal, const char *why)
{
  struct pkginfo *pkg = pdep->up->up;

  if (force_depends(pdep)) {
    warning(_("ignoring dependency problem with removal of %s:\n%s"),
            pkg_name(pkg_removal, pnaw_nonambig), why);
    return 2;
  } else if (f_autodeconf) {
    if (pkg->installed.essential) {
      if (in_force(FORCE_REMOVE_ESSENTIAL)) {
        warning(_("considering deconfiguration of essential\n"
                  " package %s, to enable removal of %s"),
                pkg_name(pkg, pnaw_nonambig), pkg_name(pkg_removal, pnaw_nonambig));
      } else {
        notice(_("no, %s is essential, will not deconfigure\n"
                 " it in order to enable removal of %s"),
               pkg_name(pkg, pnaw_nonambig), pkg_name(pkg_removal, pnaw_nonambig));
        return 0;
      }
    }
    if (pkg->installed.is_protected) {
      if (in_force(FORCE_REMOVE_PROTECTED)) {
        warning(_("considering deconfiguration of protected\n"
                  " package %s, to enable removal of %s"),
                pkg_name(pkg, pnaw_nonambig), pkg_name(pkg_removal, pnaw_nonambig));
      } else {
        notice(_("no, %s is protected, will not deconfigure\n"
                 " it in order to enable removal of %s"),
               pkg_name(pkg, pnaw_nonambig), pkg_name(pkg_removal, pnaw_nonambig));
        return 0;
      }
    }

    enqueue_deconfigure(pkg, pkg_removal, PKG_WANT_DEINSTALL);
    return 1;
  } else {
    notice(_("no, cannot proceed with removal of %s (--auto-deconfigure will help):\n%s"),
           pkg_name(pkg_removal, pnaw_nonambig), why);
    return 0;
  }
}

void check_breaks(struct dependency *dep, struct pkginfo *pkg,
                  const char *pfilename) {
  struct pkginfo *fixbydeconf;
  struct varbuf why = VARBUF_INIT;
  int ok;

  fixbydeconf = NULL;
  if (depisok(dep, &why, &fixbydeconf, NULL, false)) {
    varbuf_destroy(&why);
    return;
  }


  if (fixbydeconf && f_autodeconf) {
    ensure_package_clientdata(fixbydeconf);

    if (fixbydeconf->clientdata->istobe != PKG_ISTOBE_NORMAL)
      internerr("package %s being fixed by deconf is not to be normal, "
                "is to be %d",
                pkg_name(pkg, pnaw_always), fixbydeconf->clientdata->istobe);

    notice(_("considering deconfiguration of %s, which would be broken by installation of %s ..."),
           pkg_name(fixbydeconf, pnaw_nonambig),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig));

    ok = try_deconfigure_can(fixbydeconf, dep->list, pkg, varbuf_str(&why));
    if (ok == 1) {
      notice(_("yes, will deconfigure %s (broken by %s)"),
             pkg_name(fixbydeconf, pnaw_nonambig),
             pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
    }
  } else {
    notice(_("regarding %s containing %s:\n%s"), pfilename,
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
           varbuf_str(&why));
    ok= 0;
  }
  varbuf_destroy(&why);
  if (ok > 0) return;

  if (force_breaks(dep->list)) {
    warning(_("ignoring breakage, may proceed anyway!"));
    return;
  }

  if (fixbydeconf && !f_autodeconf) {
    ohshit(_("installing %.250s would break %.250s, and\n"
             " deconfiguration is not permitted (--auto-deconfigure might help)"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
           pkg_name(fixbydeconf, pnaw_nonambig));
  } else {
    ohshit(_("installing %.250s would break existing software"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
  }
}

void check_conflict(struct dependency *dep, struct pkginfo *pkg,
                    const char *pfilename) {
  struct pkginfo *fixbyrm;
  struct deppossi *pdep, flagdeppossi = { 0 };
  struct varbuf conflictwhy = VARBUF_INIT, removalwhy = VARBUF_INIT;
  struct dependency *providecheck;

  fixbyrm = NULL;
  if (depisok(dep, &conflictwhy, &fixbyrm, NULL, false)) {
    varbuf_destroy(&conflictwhy);
    varbuf_destroy(&removalwhy);
    return;
  }
  if (fixbyrm) {
    ensure_package_clientdata(fixbyrm);
    if (fixbyrm->clientdata->istobe == PKG_ISTOBE_INSTALLNEW) {
      fixbyrm= dep->up;
      ensure_package_clientdata(fixbyrm);
    }
    if (((pkg->available.essential || pkg->available.is_protected) &&
         (fixbyrm->installed.essential || fixbyrm->installed.is_protected)) ||
        (((fixbyrm->want != PKG_WANT_INSTALL &&
           fixbyrm->want != PKG_WANT_HOLD) ||
          does_replace(pkg, &pkg->available, fixbyrm, &fixbyrm->installed)) &&
         ((!fixbyrm->installed.essential || in_force(FORCE_REMOVE_ESSENTIAL)) ||
          (!fixbyrm->installed.is_protected || in_force(FORCE_REMOVE_PROTECTED))))) {
      if (fixbyrm->clientdata->istobe != PKG_ISTOBE_NORMAL &&
          fixbyrm->clientdata->istobe != PKG_ISTOBE_DECONFIGURE)
        internerr("package %s to be fixed by removal is not to be normal "
                  "nor deconfigure, is to be %d",
                  pkg_name(pkg, pnaw_always), fixbyrm->clientdata->istobe);
      fixbyrm->clientdata->istobe = PKG_ISTOBE_REMOVE;
      notice(_("considering removing %s in favour of %s ..."),
             pkg_name(fixbyrm, pnaw_nonambig),
             pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
      if (!(fixbyrm->status == PKG_STAT_INSTALLED ||
            fixbyrm->status == PKG_STAT_TRIGGERSPENDING ||
            fixbyrm->status == PKG_STAT_TRIGGERSAWAITED)) {
        notice(_("%s is not properly installed; ignoring any dependencies on it"),
               pkg_name(fixbyrm, pnaw_nonambig));
        pdep = NULL;
      } else {
        for (pdep = fixbyrm->set->depended.installed;
             pdep;
             pdep = pdep->rev_next) {
          if (pdep->up->type != dep_depends && pdep->up->type != dep_predepends)
            continue;
          if (depisok(pdep->up, &removalwhy, NULL, NULL, false))
            continue;
          if (!try_remove_can(pdep, fixbyrm, varbuf_str(&removalwhy)))
            break;
        }
        if (!pdep) {
          /* If we haven't found a reason not to yet, let's look some more. */
          for (providecheck= fixbyrm->installed.depends;
               providecheck;
               providecheck= providecheck->next) {
            if (providecheck->type != dep_provides) continue;
            for (pdep = providecheck->list->ed->depended.installed;
                 pdep;
                 pdep = pdep->rev_next) {
              if (pdep->up->type != dep_depends && pdep->up->type != dep_predepends)
                continue;
              if (depisok(pdep->up, &removalwhy, NULL, NULL, false))
                continue;
              notice(_("may have trouble removing %s, as it provides %s ..."),
                     pkg_name(fixbyrm, pnaw_nonambig),
                     providecheck->list->ed->name);
              if (!try_remove_can(pdep, fixbyrm, varbuf_str(&removalwhy)))
                goto break_from_both_loops_at_once;
            }
          }
        break_from_both_loops_at_once:;
        }
      }
      if (!pdep && skip_due_to_hold(fixbyrm)) {
        pdep= &flagdeppossi;
      }
      if (!pdep && (fixbyrm->eflag & PKG_EFLAG_REINSTREQ)) {
        if (in_force(FORCE_REMOVE_REINSTREQ)) {
          notice(_("package %s requires reinstallation, but will "
                   "remove anyway as you requested"),
                 pkg_name(fixbyrm, pnaw_nonambig));
        } else {
          notice(_("package %s requires reinstallation, will not remove"),
                 pkg_name(fixbyrm, pnaw_nonambig));
          pdep= &flagdeppossi;
        }
      }
      if (!pdep) {
        /* This conflict is OK - we'll remove the conflictor. */
        enqueue_conflictor(fixbyrm);
        varbuf_destroy(&conflictwhy); varbuf_destroy(&removalwhy);
        notice(_("yes, will remove %s in favour of %s"),
               pkg_name(fixbyrm, pnaw_nonambig),
               pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
        return;
      }
      /* Put it back. */
      fixbyrm->clientdata->istobe = PKG_ISTOBE_NORMAL;
    }
  }
  notice(_("regarding %s containing %s:\n%s"), pfilename,
         pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
         varbuf_str(&conflictwhy));
  if (!force_conflicts(dep->list))
    ohshit(_("conflicting packages - not installing %.250s"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
  warning(_("ignoring conflict, may proceed anyway!"));
  varbuf_destroy(&conflictwhy);

  return;
}

void cu_cidir(int argc, void **argv) {
  char *cidir= (char*)argv[0];
  char *cidirrest= (char*)argv[1];
  cidirrest[-1] = '\0';
  path_remove_tree(cidir);
  free(cidir);
}

void cu_fileslist(int argc, void **argv) {
  tar_pool_release();
}

int
archivefiles(const char *const *argv)
{
  const char *const *volatile argp;
  char **volatile arglist = NULL;
  int i;
  jmp_buf ejbuf;
  enum modstatdb_rw msdbflags;

  trigproc_install_hooks();

  if (!f_act)
    msdbflags = msdbrw_readonly;
  else if (cipaction->arg_int == act_avail)
    msdbflags = msdbrw_readonly | msdbrw_available_write;
  else if (in_force(FORCE_NON_ROOT))
    msdbflags = msdbrw_write;
  else
    msdbflags = msdbrw_needsuperuser;

  modstatdb_open(msdbflags);

  checkpath();
  pkg_infodb_upgrade();

  log_message("startup archives %s", cipaction->olong);

  if (f_recursive) {
    const char *const *ap;
    int nfiles = 0;

    if (!*argv)
      badusage(_("--%s --recursive needs at least one path argument"),cipaction->olong);

    for (ap = argv; *ap; ap++) {
      struct treeroot *tree;
      struct treenode *node;

      tree = treewalk_open((const char *)*ap, TREEWALK_FOLLOW_LINKS, NULL);

      while ((node = treewalk_next(tree))) {
        const char *nodename;

        if (!S_ISREG(treenode_get_mode(node)))
          continue;

        /* Check if it looks like a .deb file. */
        nodename = treenode_get_pathname(node);
        if (strcmp(nodename + strlen(nodename) - 4, DEBEXT) != 0)
          continue;

        arglist = m_realloc(arglist, sizeof(char *) * (nfiles + 2));
        arglist[nfiles++] = m_strdup(nodename);
      }

      treewalk_close(tree);
    }

    if (!nfiles)
      ohshit(_("searched, but found no packages (files matching *.deb)"));

    arglist[nfiles] = NULL;
    argp = (const char **volatile)arglist;
  } else {
    if (!*argv) badusage(_("--%s needs at least one package archive file argument"),
                         cipaction->olong);
    argp= argv;
  }

  /* Perform some sanity checks on the passed archives. */
  for (i = 0; argp[i]; i++) {
    struct stat st;

    /* We need the filename to exist. */
    if (stat(argp[i], &st) < 0)
      ohshite(_("cannot access archive '%s'"), argp[i]);

    /* We cannot work with anything that is not a regular file. */
    if (!S_ISREG(st.st_mode))
      ohshit(_("archive '%s' is not a regular file"), argp[i]);
  }

  currenttime = time(NULL);

  /* Initialize fname variables contents. */

  varbuf_set_str(&fnamevb, dpkg_fsys_get_dir());
  varbuf_set_str(&fnametmpvb, dpkg_fsys_get_dir());
  varbuf_set_str(&fnamenewvb, dpkg_fsys_get_dir());

  varbuf_snapshot(&fnamevb, &fname_state);
  varbuf_snapshot(&fnametmpvb, &fnametmp_state);
  varbuf_snapshot(&fnamenewvb, &fnamenew_state);

  ensure_diversions();
  ensure_statoverrides(STATDB_PARSE_NORMAL);

  for (i = 0; argp[i]; i++) {
    if (setjmp(ejbuf)) {
      pop_error_context(ehflag_bombout);
      if (abort_processing)
        break;
      continue;
    }
    push_error_context_jump(&ejbuf, print_error_perarchive, argp[i]);

    dpkg_selabel_load();

    process_archive(argp[i]);
    onerr_abort++;
    m_output(stdout, _("<standard output>"));
    m_output(stderr, _("<standard error>"));
    onerr_abort--;

    pop_error_context(ehflag_normaltidy);
  }

  dpkg_selabel_close();

  if (arglist) {
    for (i = 0; arglist[i]; i++)
      free(arglist[i]);
    free(arglist);
  }

  switch (cipaction->arg_int) {
  case act_install:
  case act_configure:
  case act_triggers:
  case act_remove:
  case act_purge:
    process_queue();
  case act_unpack:
  case act_avail:
    break;
  default:
    internerr("unknown action '%d'", cipaction->arg_int);
  }

  trigproc_run_deferred();
  modstatdb_shutdown();

  return 0;
}

/**
 * Decide whether we want to install a new version of the package.
 *
 * @param pkg The package with the version we might want to install
 *
 * @retval true  If the package should be skipped.
 * @retval false If the package should be installed.
 */
bool
wanttoinstall(struct pkginfo *pkg)
{
  int rc;

  if (pkg->want != PKG_WANT_INSTALL && pkg->want != PKG_WANT_HOLD) {
    if (f_alsoselect) {
      printf(_("Selecting previously unselected package %s.\n"),
             pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
      return true;
    } else {
      printf(_("Skipping unselected package %s.\n"),
             pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
      return false;
    }
  }

  if (pkg->eflag & PKG_EFLAG_REINSTREQ)
    return true;
  if (pkg->status < PKG_STAT_UNPACKED)
    return true;

  rc = dpkg_version_compare(&pkg->available.version, &pkg->installed.version);
  if (rc > 0) {
    return true;
  } else if (rc == 0) {
    /* Same version fully installed. */
    if (f_skipsame && pkg->available.arch == pkg->installed.arch) {
      notice(_("package %.250s (%.250s) with same version already installed, skipping"),
             pkg_name(pkg, pnaw_nonambig),
             versiondescribe(&pkg->installed.version, vdew_nonambig));
      return false;
    } else {
      return true;
    }
  } else if (in_force(FORCE_DOWNGRADE)) {
    warning(_("downgrading %.250s (%.250s) to (%.250s)"),
            pkg_name(pkg, pnaw_nonambig),
            versiondescribe(&pkg->installed.version, vdew_nonambig),
            versiondescribe(&pkg->available.version, vdew_nonambig));
    return true;
  } else {
    notice(_("will not downgrade %.250s (%.250s) to (%.250s), skipping"),
           pkg_name(pkg, pnaw_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig),
           versiondescribe(&pkg->available.version, vdew_nonambig));
    return false;
  }
}
