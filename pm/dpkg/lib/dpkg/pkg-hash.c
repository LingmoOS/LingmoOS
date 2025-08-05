/*
 * libdpkg - Debian packaging suite library routines
 * pkg-hash.c - low level package database routines (hash tables, etc.)
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/string.h>
#include <dpkg/arch.h>

/*
 * This must always be a prime for optimal performance.
 *
 * We use a number that is close to the amount of packages currently present
 * in a Debian suite, so that installed and available packages do not add
 * tons of collisions.
 *
 * The memory usage is «BINS * sizeof(void *)».
 */
#define BINS 65521

static struct pkgset *bins[BINS];
static int npkg, nset;

/**
 * Return the package set with the given name.
 *
 * If the package already exists in the internal database, then it returns
 * the existing structure. Otherwise it allocates a new one and will return
 * it. The actual name associated to the package set is a lowercase version
 * of the name given in parameter.
 *
 * A package set (struct pkgset) can be composed of multiple package instances
 * (struct pkginfo) where each instance is distinguished by its architecture
 * (as recorded in pkg.installed.arch and pkg.available.arch).
 *
 * @param inname Name of the package set.
 *
 * @return The package set.
 */
struct pkgset *
pkg_hash_find_set(const char *inname)
{
  struct pkgset **setp, *new_set;
  char *name = m_strdup(inname), *p;

  p= name;
  while (*p) {
    *p = c_tolower(*p);
    p++;
  }

  setp = bins + (str_fnv_hash(name) % (BINS));
  while (*setp && strcasecmp((*setp)->name, name))
    setp = &(*setp)->next;
  if (*setp) {
    free(name);
    return *setp;
  }

  new_set = nfmalloc(sizeof(*new_set));
  pkgset_blank(new_set);
  new_set->name = nfstrsave(name);
  new_set->next = NULL;
  *setp = new_set;
  nset++;
  npkg++;

  free(name);

  return new_set;
}

/**
 * Return the singleton package instance from a package set.
 *
 * This means, if none are installed either an instance with native or
 * all arch or the first if none found, the single installed instance,
 * or NULL if more than one instance is installed.
 *
 * @param set The package set to use.
 *
 * @return The singleton package instance.
 */
struct pkginfo *
pkg_hash_get_singleton(struct pkgset *set)
{
  struct pkginfo *pkg;

  switch (pkgset_installed_instances(set)) {
  case 0:
    /* Pick an available candidate. */
    for (pkg = &set->pkg; pkg; pkg = pkg->arch_next) {
      const struct dpkg_arch *arch = pkg->available.arch;

      if (arch->type == DPKG_ARCH_NATIVE || arch->type == DPKG_ARCH_ALL)
        return pkg;
    }
    /* Or failing that, the first entry. */
    return &set->pkg;
  case 1:
    for (pkg = &set->pkg; pkg; pkg = pkg->arch_next) {
      if (pkg->status > PKG_STAT_NOTINSTALLED)
        return pkg;
    }
    internerr("pkgset '%s' should have one installed instance", set->name);
  default:
    return NULL;
  }
}

/**
 * Return the singleton package instance with the given name.
 *
 * @param name The package name.
 *
 * @return The package instance.
 */
struct pkginfo *
pkg_hash_find_singleton(const char *name)
{
  struct pkgset *set;
  struct pkginfo *pkg;

  set = pkg_hash_find_set(name);
  pkg = pkg_hash_get_singleton(set);
  if (pkg == NULL)
    ohshit(_("ambiguous package name '%s' with more "
             "than one installed instance"), set->name);

  return pkg;
}

/**
 * Return the package instance in a set with the given architecture.
 *
 * It traverse the various instances to find out whether there's one
 * matching the given architecture. If found, it returns it. Otherwise it
 * allocates a new instance and registers it in the package set before
 * returning it.
 *
 * @param set  The package set to use.
 * @param arch The requested architecture.
 *
 * @return The package instance.
 */
struct pkginfo *
pkg_hash_get_pkg(struct pkgset *set, const struct dpkg_arch *arch)
{
  struct pkginfo *pkg, **pkgp;

  if (arch == NULL)
    internerr("arch argument is NULL");
  if (arch->type == DPKG_ARCH_NONE)
    internerr("arch argument is none");

  pkg = &set->pkg;

  /* If there's a single unused slot, let's use that. */
  if (pkg->installed.arch->type == DPKG_ARCH_NONE && pkg->arch_next == NULL) {
    /* We can only initialize the arch pkgbin members, because those are used
     * to find instances, anything else will be overwritten at parse time. */
    pkg->installed.arch = arch;
    pkg->available.arch = arch;
    return pkg;
  }

  /* Match the slot with the most appropriate architecture. The installed
   * architecture always has preference over the available one, as there's
   * a small time window on cross-grades, where they might differ. */
  /* cppcheck-suppress[knownConditionTrueFalse,knownPointerToBool]:
   * False positive, the condition always ends up being false. */
  for (pkgp = &pkg; *pkgp; pkgp = &(*pkgp)->arch_next) {
    if ((*pkgp)->installed.arch == arch)
      return *pkgp;
  }

  /* Need to create a new instance for the wanted architecture. */
  pkg = nfmalloc(sizeof(*pkg));
  pkg_blank(pkg);
  pkg->set = set;
  pkg->arch_next = NULL;
  /* We can only initialize the arch pkgbin members, because those are used
   * to find instances, anything else will be overwritten at parse time. */
  pkg->installed.arch = arch;
  pkg->available.arch = arch;
  *pkgp = pkg;
  npkg++;

  return pkg;
}

/**
 * Return the package instance with the given name and architecture.
 *
 * @param name The package name.
 * @param arch The requested architecture.
 *
 * @return The package instance.
 */
struct pkginfo *
pkg_hash_find_pkg(const char *name, const struct dpkg_arch *arch)
{
  struct pkgset *set;
  struct pkginfo *pkg;

  set = pkg_hash_find_set(name);
  pkg = pkg_hash_get_pkg(set, arch);

  return pkg;
}

/**
 * Return the number of package sets available in the database.
 *
 * @return The number of package sets.
 */
int
pkg_hash_count_set(void)
{
  return nset;
}

/**
 * Return the number of package instances available in the database.
 *
 * @return The number of package instances.
 */
int
pkg_hash_count_pkg(void)
{
  return npkg;
}

struct pkg_hash_iter {
  struct pkginfo *pkg;
  int nbinn;
};

/**
 * Create a new package iterator.
 *
 * It can iterate either over package sets or over package instances.
 *
 * @return The iterator.
 */
struct pkg_hash_iter *
pkg_hash_iter_new(void)
{
  struct pkg_hash_iter *iter;

  iter = m_malloc(sizeof(*iter));
  iter->pkg = NULL;
  iter->nbinn = 0;

  return iter;
}

/**
 * Return the next package set in the database.
 *
 * If no further package set is available, it will return NULL.
 *
 * @name iter The iterator.
 *
 * @return A package set.
 */
struct pkgset *
pkg_hash_iter_next_set(struct pkg_hash_iter *iter)
{
  struct pkgset *set;

  while (!iter->pkg) {
    if (iter->nbinn >= BINS)
      return NULL;
    if (bins[iter->nbinn])
      iter->pkg = &bins[iter->nbinn]->pkg;
    iter->nbinn++;
  }

  set = iter->pkg->set;
  if (set->next)
    iter->pkg = &set->next->pkg;
  else
    iter->pkg = NULL;

  return set;
}

/**
 * Return the next package instance in the database.
 *
 * If no further package instance is available, it will return NULL. Note
 * that it will return all instances of a given package set in sequential
 * order. The first instance for a given package set will always correspond
 * to the native architecture even if that package is not installed or
 * available.
 *
 * @name iter The iterator.
 *
 * @return A package instance.
 */
struct pkginfo *
pkg_hash_iter_next_pkg(struct pkg_hash_iter *iter)
{
  struct pkginfo *pkg;

  while (!iter->pkg) {
    if (iter->nbinn >= BINS)
      return NULL;
    if (bins[iter->nbinn])
      iter->pkg = &bins[iter->nbinn]->pkg;
    iter->nbinn++;
  }

  pkg = iter->pkg;
  if (pkg->arch_next)
    iter->pkg = pkg->arch_next;
  else if (pkg->set->next)
    iter->pkg = &pkg->set->next->pkg;
  else
    iter->pkg = NULL;

  return pkg;
}

/**
 * Free the package database iterator.
 *
 * @name iter The iterator.
 */
void
pkg_hash_iter_free(struct pkg_hash_iter *iter)
{
  free(iter);
}

void
pkg_hash_reset(void)
{
  dpkg_arch_reset_list();
  nffreeall();
  nset = 0;
  npkg = 0;
  memset(bins, 0, sizeof(bins));
}

void
pkg_hash_report(FILE *file)
{
  int i, c;
  struct pkgset *pkg;
  int *freq;
  int empty = 0, used = 0, collided = 0;

  freq = m_malloc(sizeof(int) * nset + 1);
  for (i = 0; i <= nset; i++)
    freq[i] = 0;
  for (i=0; i<BINS; i++) {
    for (c=0, pkg= bins[i]; pkg; c++, pkg= pkg->next);
    fprintf(file, "pkg-hash: bin %5d has %7d\n", i, c);
    if (c == 0)
      empty++;
    else if (c == 1)
      used++;
    else {
      used++;
      collided++;
    }
    freq[c]++;
  }
  for (i = nset; i > 0 && freq[i] == 0; i--);
  while (i >= 0) {
    fprintf(file, "pkg-hash: size %7d occurs %5d times\n", i, freq[i]);
    i--;
  }
  fprintf(file, "pkg-hash: bins empty %d\n", empty);
  fprintf(file, "pkg-hash: bins used %d (collided %d)\n", used, collided);

  m_output(file, "<hash report>");

  free(freq);
}
