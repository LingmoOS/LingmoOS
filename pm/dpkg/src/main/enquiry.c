/*
 * dpkg - main program for package management
 * enquiry.c - status enquiry and listing options
 *
 * Copyright © 1995,1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2016 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-show.h>
#include <dpkg/triglib.h>
#include <dpkg/string.h>
#include <dpkg/options.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"

struct audit_problem {
  bool (*check)(struct pkginfo *, const struct audit_problem *problem);
  union {
    int number;
    const char *string;
  } value;
  const char *explanation;
};

static bool
audit_reinstreq(struct pkginfo *pkg, const struct audit_problem *problem)
{
  return pkg->eflag & PKG_EFLAG_REINSTREQ;
}

static bool
audit_status(struct pkginfo *pkg, const struct audit_problem *problem)
{
  if (pkg->eflag & PKG_EFLAG_REINSTREQ)
    return false;
  return (int)pkg->status == problem->value.number;
}

static bool
audit_infofile(struct pkginfo *pkg, const struct audit_problem *problem)
{
  if (pkg->status < PKG_STAT_HALFINSTALLED)
    return false;
  return !pkg_infodb_has_file(pkg, &pkg->installed, problem->value.string);
}

static bool
audit_arch(struct pkginfo *pkg, const struct audit_problem *problem)
{
  if (pkg->status < PKG_STAT_HALFINSTALLED)
    return false;
  return pkg->installed.arch->type == (enum dpkg_arch_type)problem->value.number;
}

static const struct audit_problem audit_problems[] = {
  {
    .check = audit_reinstreq,
    .value.number = 0,
    .explanation = N_(
    "The following packages are in a mess due to serious problems during\n"
    "installation.  They must be reinstalled for them (and any packages\n"
    "that depend on them) to function properly:\n")
  }, {
    .check = audit_status,
    .value.number = PKG_STAT_UNPACKED,
    .explanation = N_(
    "The following packages have been unpacked but not yet configured.\n"
    "They must be configured using dpkg --configure or the configure\n"
    "menu option in dselect for them to work:\n")
  }, {
    .check = audit_status,
    .value.number = PKG_STAT_HALFCONFIGURED,
    .explanation = N_(
    "The following packages are only half configured, probably due to problems\n"
    "configuring them the first time.  The configuration should be retried using\n"
    "dpkg --configure <package> or the configure menu option in dselect:\n")
  }, {
    .check = audit_status,
    .value.number = PKG_STAT_HALFINSTALLED,
    .explanation = N_(
    "The following packages are only half installed, due to problems during\n"
    "installation.  The installation can probably be completed by retrying it;\n"
    "the packages can be removed using dselect or dpkg --remove:\n")
  }, {
    .check = audit_status,
    .value.number = PKG_STAT_TRIGGERSAWAITED,
    .explanation = N_(
    "The following packages are awaiting processing of triggers that they\n"
    "have activated in other packages.  This processing can be requested using\n"
    "dselect or dpkg --configure --pending (or dpkg --triggers-only):\n")
  }, {
    .check = audit_status,
    .value.number = PKG_STAT_TRIGGERSPENDING,
    .explanation = N_(
    "The following packages have been triggered, but the trigger processing\n"
    "has not yet been done.  Trigger processing can be requested using\n"
    "dselect or dpkg --configure --pending (or dpkg --triggers-only):\n")
  }, {
    .check = audit_infofile,
    .value.string = LISTFILE,
    .explanation = N_(
    "The following packages are missing the list control file in the\n"
    "database, they need to be reinstalled:\n")
  }, {
    .check = audit_infofile,
    .value.string = HASHFILE,
    .explanation = N_(
    "The following packages are missing the md5sums control file in the\n"
    "database, they need to be reinstalled:\n")
  }, {
    .check = audit_arch,
    .value.number = DPKG_ARCH_EMPTY,
    .explanation = N_("The following packages do not have an architecture:\n")
  }, {
    .check = audit_arch,
    .value.number = DPKG_ARCH_ILLEGAL,
    .explanation = N_("The following packages have an illegal architecture:\n")
  }, {
    .check = audit_arch,
    .value.number = DPKG_ARCH_UNKNOWN,
    .explanation = N_(
    "The following packages have an unknown foreign architecture, which will\n"
    "cause dependency issues on front-ends. This can be fixed by registering\n"
    "the foreign architecture with dpkg --add-architecture:\n")
  }, {
    .check = NULL
  }
};

static void describebriefly(struct pkginfo *pkg) {
  int maxl, l;
  const char *pdesc;

  maxl= 57;
  l= strlen(pkg->set->name);
  if (l>20) maxl -= (l-20);

  pdesc = pkgbin_synopsis(pkg, &pkg->installed, &l);
  l = min(l, maxl);

  printf(" %-20s %.*s\n", pkg_name(pkg, pnaw_nonambig), l, pdesc);
}

static struct pkginfo *
pkg_array_mapper(const char *name)
{
  struct pkginfo *pkg;

  pkg = dpkg_options_parse_pkgname(cipaction, name);
  if (pkg->status == PKG_STAT_NOTINSTALLED)
    notice(_("package '%s' is not installed"), pkg_name(pkg, pnaw_nonambig));

  return pkg;
}

int
audit(const char *const *argv)
{
  const struct audit_problem *problem;
  struct pkg_array array;
  bool head_running = false;
  int i;

  modstatdb_open(msdbrw_readonly);

  if (!*argv)
    pkg_array_init_from_hash(&array);
  else
    pkg_array_init_from_names(&array, pkg_array_mapper, (const char **)argv);

  pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);

  for (problem = audit_problems; problem->check; problem++) {
    bool head = false;

    for (i = 0; i < array.n_pkgs; i++) {
      struct pkginfo *pkg = array.pkgs[i];

      if (!problem->check(pkg, problem))
        continue;
      if (!head_running) {
        if (modstatdb_is_locked())
          puts(_(
"Another process has locked the database for writing, and might currently be\n"
"modifying it, some of the following problems might just be due to that.\n"));
        head_running = true;
      }
      if (!head) {
        fputs(gettext(problem->explanation), stdout);
        head = true;
      }
      describebriefly(pkg);
    }

    if (head) putchar('\n');
  }

  pkg_array_destroy(&array);

  m_output(stdout, _("<standard output>"));

  return 0;
}

struct sectionentry {
  struct sectionentry *next;
  const char *name;
  int count;
};

static bool
yettobeunpacked(struct pkginfo *pkg, const char **thissect)
{
  if (pkg->want != PKG_WANT_INSTALL)
    return false;

  switch (pkg->status) {
  case PKG_STAT_UNPACKED:
  case PKG_STAT_INSTALLED:
  case PKG_STAT_HALFCONFIGURED:
  case PKG_STAT_TRIGGERSPENDING:
  case PKG_STAT_TRIGGERSAWAITED:
    return false;
  case PKG_STAT_NOTINSTALLED:
  case PKG_STAT_HALFINSTALLED:
  case PKG_STAT_CONFIGFILES:
    if (thissect)
      *thissect = str_is_set(pkg->section) ? pkg->section :
                                             C_("section", "<unknown>");
    return true;
  default:
    internerr("unknown package status '%d'", pkg->status);
  }
  return false;
}

int
unpackchk(const char *const *argv)
{
  int totalcount, sects;
  struct sectionentry *sectionentries, *se, **sep;
  struct pkg_hash_iter *iter;
  struct pkginfo *pkg;
  const char *thissect;
  char buf[20];
  int width;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  modstatdb_open(msdbrw_readonly);

  totalcount= 0;
  sectionentries = NULL;
  sects= 0;
  iter = pkg_hash_iter_new();
  while ((pkg = pkg_hash_iter_next_pkg(iter))) {
    if (!yettobeunpacked(pkg, &thissect)) continue;
    for (se= sectionentries; se && strcasecmp(thissect,se->name); se= se->next);
    if (!se) {
      se = nfmalloc(sizeof(*se));
      for (sep= &sectionentries;
           *sep && strcasecmp(thissect,(*sep)->name) > 0;
           sep= &(*sep)->next);
      se->name= thissect;
      se->count= 0;
      se->next= *sep;
      *sep= se;
      sects++;
    }
    se->count++; totalcount++;
  }
  pkg_hash_iter_free(iter);

  if (totalcount == 0)
    return 0;

  if (totalcount <= 12) {
    iter = pkg_hash_iter_new();
    while ((pkg = pkg_hash_iter_next_pkg(iter))) {
      if (!yettobeunpacked(pkg, NULL))
        continue;
      describebriefly(pkg);
    }
    pkg_hash_iter_free(iter);
  } else if (sects <= 12) {
    for (se= sectionentries; se; se= se->next) {
      snprintf(buf, sizeof(buf), "%d", se->count);
      printf(_(" %d in %s: "),se->count,se->name);
      width= 70-strlen(se->name)-strlen(buf);
      while (width > 59) { putchar(' '); width--; }
      iter = pkg_hash_iter_new();
      while ((pkg = pkg_hash_iter_next_pkg(iter))) {
        const char *pkgname;

        if (!yettobeunpacked(pkg,&thissect)) continue;
        if (strcasecmp(thissect,se->name)) continue;
        pkgname = pkg_name(pkg, pnaw_nonambig);
        width -= strlen(pkgname);
        width--;
        if (width < 4) { printf(" ..."); break; }
        printf(" %s", pkgname);
      }
      pkg_hash_iter_free(iter);
      putchar('\n');
    }
  } else {
    printf(P_(" %d package, from the following section:",
              " %d packages, from the following sections:", totalcount),
           totalcount);
    width= 0;
    for (se= sectionentries; se; se= se->next) {
      snprintf(buf, sizeof(buf), "%d", se->count);
      width -= (6 + strlen(se->name) + strlen(buf));
      if (width < 0) { putchar('\n'); width= 73 - strlen(se->name) - strlen(buf); }
      printf("   %s (%d)",se->name,se->count);
    }
    putchar('\n');
  }

  m_output(stdout, _("<standard output>"));

  return 0;
}

static const struct assert_feature {
  const char *name;
  const char *desc;
  const char *version;
} assert_features[] = {
  {
    .name = "support-predepends",
    .desc = N_("the Pre-Depends field"),
    .version = "1.1.0",
  }, {
    .name = "working-epoch",
    .desc = N_("epochs in versions"),
    .version = "1.4.0.7",
  }, {
    .name = "long-filenames",
    .desc = N_("long filenames in .deb archives"),
    .version = "1.4.1.17",
  }, {
    .name = "multi-conrep",
    .desc = N_("multiple Conflicts and Replaces"),
    .version = "1.4.1.19",
  }, {
    .name = "multi-arch",
    .desc = N_("multi-arch fields and semantics"),
    .version = "1.16.2",
  }, {
    .name = "versioned-provides",
    .desc = N_("versioned relationships in the Provides field"),
    .version = "1.17.11",
  }, {
    .name = "protected-field",
    .desc = N_("the Protected field"),
    .version = "1.20.1",
  }, {
    .name = NULL,
  }
};

static int
assert_version_support(const char *const *argv,
                       const struct assert_feature *feature)
{
  const char *running_version_str;
  struct dpkg_version running_version = DPKG_VERSION_INIT;
  struct dpkg_version version = { 0, feature->version, NULL };
  struct dpkg_error err = DPKG_ERROR_INIT;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  /*
   * When using the feature asserts, we want to know whether the currently
   * running dpkg, which we might be running under (say from within a
   * maintainer script) and which might have a different version, supports
   * the requested feature. As dpkg is an Essential package, it is expected
   * to work even when just unpacked, and so its own version is enough.
   */
  running_version_str = getenv("DPKG_RUNNING_VERSION");

  /*
   * If we are not running from within a maintainer script, then that means
   * once we do, the executed dpkg will support the requested feature, if
   * we know about it. Always return success in that case.
   */
  if (str_is_unset(running_version_str))
    return 0;

  if (parseversion(&running_version, running_version_str, &err) < 0)
    ohshit(_("cannot parse dpkg running version '%s': %s"),
           running_version_str, err.str);

  if (dpkg_version_relate(&running_version, DPKG_RELATION_GE, &version))
    return 0;

  printf(_("Running version of dpkg does not support %s.\n"
           " Please upgrade to at least dpkg %s, and then try again.\n"),
         feature->desc, versiondescribe(&version, vdew_nonambig));
  return 1;
}

const char *assert_feature_name;

int
assert_feature(const char *const *argv)
{
  const struct assert_feature *feature;

  if (strcmp(assert_feature_name, "help") == 0) {
    printf(_("%s assert options - assert whether features are supported:\n"),
           dpkg_get_progname());

    for (feature = assert_features; feature->name; feature++) {
      printf("  %-19s %-9s %s\n", feature->name, feature->version,
             gettext(feature->desc));
    }

    exit(0);
  }

  for (feature = assert_features; feature->name; feature++) {
    if (strcmp(feature->name, assert_feature_name) != 0)
      continue;

    return assert_version_support(argv, feature);
  }

  badusage(_("unknown --%s-<feature>"), cipaction->olong);
}

/**
 * Print a single package which:
 *  (a) is the target of one or more relevant predependencies.
 *  (b) has itself no unsatisfied pre-dependencies.
 *
 * If such a package is present output is the Packages file entry,
 * which can be massaged as appropriate.
 *
 * Exit status:
 *  0 = a package printed, OK
 *  1 = no suitable package available
 *  2 = error
 */
int
predeppackage(const char *const *argv)
{
  static struct varbuf vb;

  struct pkg_hash_iter *iter;
  struct pkginfo *pkg = NULL, *startpkg, *trypkg;
  struct dependency *dep;
  struct deppossi *possi, *provider;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  modstatdb_open(msdbrw_readonly | msdbrw_available_readonly);
  /* We use clientdata->istobe to detect loops. */
  clear_istobes();

  dep = NULL;
  iter = pkg_hash_iter_new();
  while (!dep && (pkg = pkg_hash_iter_next_pkg(iter))) {
    /* Ignore packages user doesn't want. */
    if (pkg->want != PKG_WANT_INSTALL)
      continue;
    /* Ignore packages not available. */
    if (!pkg->archives)
      continue;
    pkg->clientdata->istobe = PKG_ISTOBE_PREINSTALL;
    for (dep= pkg->available.depends; dep; dep= dep->next) {
      if (dep->type != dep_predepends) continue;
      if (depisok(dep, &vb, NULL, NULL, true))
        continue;
      /* This will leave dep non-NULL, and so exit the loop. */
      break;
    }
    pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    /* If dep is NULL we go and get the next package. */
  }
  pkg_hash_iter_free(iter);

  if (!dep)
    return 1; /* Not found. */
  if (pkg == NULL)
    internerr("unexpected unfound package");

  startpkg= pkg;
  pkg->clientdata->istobe = PKG_ISTOBE_PREINSTALL;

  /* OK, we have found an unsatisfied predependency.
   * Now go and find the first thing we need to install, as a first step
   * towards satisfying it. */
  do {
    /* We search for a package which would satisfy dep, and put it in pkg. */
    for (possi = dep->list, pkg = NULL;
         !pkg && possi;
         possi=possi->next) {
      struct deppossi_pkg_iterator *possi_iter;

      possi_iter = deppossi_pkg_iter_new(possi, wpb_available);
      while (!pkg && (trypkg = deppossi_pkg_iter_next(possi_iter))) {
        if (trypkg->archives &&
            trypkg->clientdata->istobe == PKG_ISTOBE_NORMAL &&
            versionsatisfied(&trypkg->available, possi)) {
          pkg = trypkg;
          break;
        }
        for (provider = possi->ed->depended.available;
             !pkg && provider;
             provider = provider->next) {
          if (provider->up->type != dep_provides)
            continue;
          if (!pkg_virtual_deppossi_satisfied(possi, provider))
            continue;
          trypkg = provider->up->up;
          if (!trypkg->archives)
            continue;
          if (trypkg->clientdata->istobe == PKG_ISTOBE_NORMAL) {
            pkg = trypkg;
            break;
          }
        }
      }
      deppossi_pkg_iter_free(possi_iter);
    }
    if (!pkg) {
      varbuf_reset(&vb);
      describedepcon(&vb,dep);
      notice(_("cannot see how to satisfy pre-dependency:\n %s"), vb.buf);
      ohshit(_("cannot satisfy pre-dependencies for %.250s (wanted due to %.250s)"),
             pkgbin_name(dep->up, &dep->up->available, pnaw_nonambig),
             pkgbin_name(startpkg, &startpkg->available, pnaw_nonambig));
    }
    pkg->clientdata->istobe = PKG_ISTOBE_PREINSTALL;
    for (dep= pkg->available.depends; dep; dep= dep->next) {
      if (dep->type != dep_predepends) continue;
      if (depisok(dep, &vb, NULL, NULL, true))
        continue;
      /* This will leave dep non-NULL, and so exit the loop. */
      break;
    }
  } while (dep);

  /* OK, we've found it - pkg has no unsatisfied pre-dependencies! */
  write_stanza(stdout, _("<standard output>"), pkg, &pkg->available);

  m_output(stdout, _("<standard output>"));

  return 0;
}

int
printarch(const char *const *argv)
{
  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  dpkg_arch_load_native();

  printf("%s\n", dpkg_arch_get(DPKG_ARCH_NATIVE)->name);

  m_output(stdout, _("<standard output>"));

  return 0;
}

int
print_foreign_arches(const char *const *argv)
{
  struct dpkg_arch *arch;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  dpkg_arch_load_list();

  for (arch = dpkg_arch_get_list(); arch; arch = arch->next) {
    if (arch->type != DPKG_ARCH_FOREIGN)
      continue;

    printf("%s\n", arch->name);
  }

  m_output(stdout, _("<standard output>"));

  return 0;
}

int
validate_pkgname(const char *const *argv)
{
  const char *emsg;

  if (!argv[0] || argv[1])
    badusage(_("--%s takes one <pkgname> argument"), cipaction->olong);

  emsg = pkg_name_is_illegal(argv[0]);
  if (emsg)
    ohshit(_("package name '%s' is invalid: %s"), argv[0], emsg);

  return 0;
}

int
validate_trigname(const char *const *argv)
{
  const char *emsg;

  if (!argv[0] || argv[1])
    badusage(_("--%s takes one <trigname> argument"), cipaction->olong);

  emsg = trig_name_is_illegal(argv[0]);
  if (emsg)
    ohshit(_("trigger name '%s' is invalid: %s"), argv[0], emsg);

  return 0;
}

int
validate_archname(const char *const *argv)
{
  const char *emsg;

  if (!argv[0] || argv[1])
    badusage(_("--%s takes one <archname> argument"), cipaction->olong);

  emsg = dpkg_arch_name_is_illegal(argv[0]);
  if (emsg)
    ohshit(_("architecture name '%s' is invalid: %s"), argv[0], emsg);

  return 0;
}

int
validate_version(const char *const *argv)
{
  struct dpkg_version version;
  struct dpkg_error err;

  if (!argv[0] || argv[1])
    badusage(_("--%s takes one <version> argument"), cipaction->olong);

  if (parseversion(&version, argv[0], &err) < 0) {
    dpkg_error_print(&err, _("version '%s' has bad syntax"), argv[0]);
    dpkg_error_destroy(&err);

    return 1;
  }

  return 0;
}

int
cmpversions(const char *const *argv)
{
  struct relationinfo {
    const char *op;
    /* These values are exit status codes. */
    int if_lesser, if_equal, if_greater;
    int if_none_a, if_none_both, if_none_b;
    bool obsolete;
  };

  static const struct relationinfo relationinfos[]= {
    {
      .op = "le",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "lt",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "eq",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "ne",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = "ge",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = "gt",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_SUCCESS,
    },

    /* These treat an empty version as later than any version. */
    {
      .op = "le-nl",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = "lt-nl",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = "ge-nl",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "gt-nl",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_FAILURE,
    },

    /* For compatibility with dpkg control file syntax. */
    {
      .op = "<",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
      .obsolete = true,
    }, {
      .op = "<=",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "<<",
      .if_lesser = EXIT_SUCCESS,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_SUCCESS,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = "=",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_FAILURE,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_FAILURE,
    }, {
      .op = ">",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_SUCCESS,
      .obsolete = true,
    }, {
      .op = ">=",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_SUCCESS,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_SUCCESS,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = ">>",
      .if_lesser = EXIT_FAILURE,
      .if_equal = EXIT_FAILURE,
      .if_greater = EXIT_SUCCESS,
      .if_none_a = EXIT_FAILURE,
      .if_none_both = EXIT_FAILURE,
      .if_none_b = EXIT_SUCCESS,
    }, {
      .op = NULL,
    }
  };

  const struct relationinfo *rip;
  struct dpkg_version a, b;
  struct dpkg_error err;
  int rc;

  if (!argv[0] || !argv[1] || !argv[2] || argv[3])
    badusage(_("--compare-versions takes three arguments:"
             " <version> <relation> <version>"));

  for (rip = relationinfos; rip->op && strcmp(rip->op, argv[1]); rip++)
    ;

  if (!rip->op)
    badusage(_("--compare-versions bad relation"));

  if (rip->obsolete)
    warning(_("--%s used with obsolete relation operator '%s'"),
            cipaction->olong, rip->op);

  if (*argv[0] && strcmp(argv[0],"<unknown>")) {
    if (parseversion(&a, argv[0], &err) < 0) {
      dpkg_error_print(&err, _("version '%s' has bad syntax"), argv[0]);
      dpkg_error_destroy(&err);
    }
  } else {
    dpkg_version_blank(&a);
  }
  if (*argv[2] && strcmp(argv[2],"<unknown>")) {
    if (parseversion(&b, argv[2], &err) < 0) {
      dpkg_error_print(&err, _("version '%s' has bad syntax"), argv[2]);
      dpkg_error_destroy(&err);
    }
  } else {
    dpkg_version_blank(&b);
  }
  if (!dpkg_version_is_informative(&a)) {
    if (dpkg_version_is_informative(&b))
      return rip->if_none_a;
    else
      return rip->if_none_both;
  } else if (!dpkg_version_is_informative(&b)) {
    return rip->if_none_b;
  }
  rc = dpkg_version_compare(&a, &b);
  debug(dbg_general, "cmpversions a='%s' b='%s' r=%d",
        versiondescribe_c(&a,vdew_always),
        versiondescribe_c(&b,vdew_always),
        rc);
  if (rc > 0)
    return rip->if_greater;
  else if (rc < 0)
    return rip->if_lesser;
  else
    return rip->if_equal;
}
