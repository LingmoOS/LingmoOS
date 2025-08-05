/*
 * dpkg - main program for package management
 * main.c - main program
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2016 Guillem Jover <guillem@debian.org>
 * Copyright © 2010 Canonical Ltd.
 *   written by Martin Pitt <martin.pitt@canonical.com>
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
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/pager.h>
#include <dpkg/options.h>
#include <dpkg/db-fsys.h>

#include "main.h"
#include "filters.h"

static int
printversion(const char *const *argv)
{
  if (f_robot) {
    printf("%s", PACKAGE_VERSION);
  } else {
    printf(_("Debian '%s' package management program version %s.\n"),
           DPKG, PACKAGE_RELEASE);
    printf(_(
"This is free software; see the GNU General Public License version 2 or\n"
"later for copying conditions. There is NO warranty.\n"));
  }

  m_output(stdout, _("<standard output>"));

  return 0;
}

/*
 * FIXME: Options that need fixing:
 * dpkg --command-fd
 */

static int
usage(const char *const *argv)
{
  printf(_(
"Usage: %s [<option>...] <command>\n"
"\n"), DPKG);

  printf(_(
"Commands:\n"
"  -i|--install       <.deb file name>... | -R|--recursive <directory>...\n"
"  --unpack           <.deb file name>... | -R|--recursive <directory>...\n"
"  -A|--record-avail  <.deb file name>... | -R|--recursive <directory>...\n"
"  --configure        <package>... | -a|--pending\n"
"  --triggers-only    <package>... | -a|--pending\n"
"  -r|--remove        <package>... | -a|--pending\n"
"  -P|--purge         <package>... | -a|--pending\n"
"  -V|--verify [<package>...]       Verify the integrity of package(s).\n"
"  --get-selections [<pattern>...]  Get list of selections to stdout.\n"
"  --set-selections                 Set package selections from stdin.\n"
"  --clear-selections               Deselect every non-essential package.\n"
"  --update-avail [<Packages-file>] Replace available packages info.\n"
"  --merge-avail [<Packages-file>]  Merge with info from file.\n"
"  --clear-avail                    Erase existing available info.\n"
"  --forget-old-unavail             Forget uninstalled unavailable pkgs.\n"
"  -s|--status [<package>...]       Display package status details.\n"
"  -p|--print-avail [<package>...]  Display available version details.\n"
"  -L|--listfiles <package>...      List files 'owned' by package(s).\n"
"  -l|--list [<pattern>...]         List packages concisely.\n"
"  -S|--search <pattern>...         Find package(s) owning file(s).\n"
"  -C|--audit [<package>...]        Check for broken package(s).\n"
"  --yet-to-unpack                  Print packages selected for installation.\n"
"  --predep-package                 Print pre-dependencies to unpack.\n"
"  --add-architecture <arch>        Add <arch> to the list of architectures.\n"
"  --remove-architecture <arch>     Remove <arch> from the list of architectures.\n"
"  --print-architecture             Print dpkg architecture.\n"
"  --print-foreign-architectures    Print allowed foreign architectures.\n"
"  --assert-help                    Show help on assertions.\n"
"  --assert-<feature>               Assert support for the specified feature.\n"
"  --validate-<thing> <string>      Validate a <thing>'s <string>.\n"
"  --compare-versions <a> <op> <b>  Compare version numbers - see below.\n"
"  --force-help                     Show help on forcing.\n"
"  -Dh|--debug=help                 Show help on debugging.\n"
"\n"));

  printf(_(
"  -?, --help                       Show this help message.\n"
"      --version                    Show the version.\n"
"\n"));

  printf(_(
"Validatable things: pkgname, archname, trigname, version.\n"
"\n"));

  printf(_(
"Use dpkg with -b, --build, -c, --contents, -e, --control, -I, --info,\n"
"  -f, --field, -x, --extract, -X, --vextract, --ctrl-tarfile, --fsys-tarfile\n"
"on archives (type %s --help).\n"
"\n"), BACKEND);

  printf(_(
"Options:\n"
"  --admindir=<directory>     Use <directory> instead of %s.\n"
"  --root=<directory>         Install on a different root directory.\n"
"  --instdir=<directory>      Change installation dir without changing admin dir.\n"
"  --pre-invoke=<command>     Set a pre-invoke hook.\n"
"  --post-invoke=<command>    Set a post-invoke hook.\n"
"  --path-exclude=<pattern>   Do not install paths which match a shell pattern.\n"
"  --path-include=<pattern>   Re-include a pattern after a previous exclusion.\n"
"  -O|--selected-only         Skip packages not selected for install/upgrade.\n"
"  -E|--skip-same-version     Skip packages with same installed version/arch.\n"
"  -G|--refuse-downgrade      Skip packages with earlier version than installed.\n"
"  -B|--auto-deconfigure      Install even if it would break some other package.\n"
"  --[no-]triggers            Skip or force consequential trigger processing.\n"
"  --verify-format=<format>   Verify output format (supported: 'rpm').\n"
"  --no-pager                 Disables the use of any pager.\n"
"  --no-debsig                Do not try to verify package signatures.\n"
"  --no-act|--dry-run|--simulate\n"
"                             Just say what we would do - don't do it.\n"
"  -D|--debug=<octal>         Enable debugging (see -Dhelp or --debug=help).\n"
"  --status-fd <n>            Send status change updates to file descriptor <n>.\n"
"  --status-logger=<command>  Send status change updates to <command>'s stdin.\n"
"  --log=<filename>           Log status changes and actions to <filename>.\n"
"  --ignore-depends=<package>[,...]\n"
"                             Ignore dependencies involving <package>.\n"
"  --force-<thing>[,...]      Override problems (see --force-help).\n"
"  --no-force-<thing>[,...]   Stop when problems encountered.\n"
"  --refuse-<thing>[,...]     Ditto.\n"
"  --abort-after <n>          Abort after encountering <n> errors.\n"
"  --robot                    Use machine-readable output on some commands.\n"
"\n"), ADMINDIR);

  printf(_(
"Comparison operators for --compare-versions are:\n"
"  lt le eq ne ge gt       (treat empty version as earlier than any version);\n"
"  lt-nl le-nl ge-nl gt-nl (treat empty version as later than any version);\n"
"  < << <= = >= >> >       (only for compatibility with control file syntax).\n"
"\n"));

  printf(_(
"Use 'apt' or 'aptitude' for user-friendly package management.\n"));

  m_output(stdout, _("<standard output>"));

  return 0;
}

static const char printforhelp[] = N_(
"Type dpkg --help for help about installing and deinstalling packages [*];\n"
"Use 'apt' or 'aptitude' for user-friendly package management;\n"
"Type dpkg -Dhelp for a list of dpkg debug flag values;\n"
"Type dpkg --force-help for a list of forcing options;\n"
"Type dpkg-deb --help for help about manipulating *.deb files;\n"
"\n"
"Options marked [*] produce a lot of output - pipe it through 'less' or 'more' !");

int f_act = 1;
int f_alsoselect = 1;
int f_autodeconf = 0;
int f_debsig = 1;
int f_pending = 0;
int f_recursive = 0;
int f_robot = 0;
int f_skipsame = 0;
int f_triggers = 0;

int errabort = 50;
struct pkg_list *ignoredependss = NULL;

#define DBG_DEF(n, d) \
  { .flag = dbg_##n, .name = #n, .desc = d }

static const struct debuginfo {
  int flag;
  const char *name;
  const char *desc;
} debuginfos[] = {
  DBG_DEF(general,         N_("Generally helpful progress information")),
  DBG_DEF(scripts,         N_("Invocation and status of maintainer scripts")),
  DBG_DEF(eachfile,        N_("Output for each file processed")),
  DBG_DEF(eachfiledetail,  N_("Lots of output for each file processed")),
  DBG_DEF(conff,           N_("Output for each configuration file")),
  DBG_DEF(conffdetail,     N_("Lots of output for each configuration file")),
  DBG_DEF(depcon,          N_("Dependencies and conflicts")),
  DBG_DEF(depcondetail,    N_("Lots of dependencies/conflicts output")),
  DBG_DEF(triggers,        N_("Trigger activation and processing")),
  DBG_DEF(triggersdetail,  N_("Lots of output regarding triggers")),
  DBG_DEF(triggersstupid,  N_("Silly amounts of output regarding triggers")),
  DBG_DEF(veryverbose,     N_("Lots of drivel about eg the dpkg/info directory")),
  DBG_DEF(stupidlyverbose, N_("Insane amounts of drivel")),
  { 0, NULL, NULL }
};

static void
set_debug(const struct cmdinfo *cpi, const char *value)
{
  long mask;

  if (*value == 'h') {
    const struct debuginfo *dip;

    printf(_(
"%s debugging option, --debug=<octal> or -D<octal>:\n"
"\n"
" Number  Ref. in source   Description\n"), DPKG);

    for (dip = debuginfos; dip->name; dip++)
      printf(" %6o  %-16s %s\n", dip->flag, dip->name, gettext(dip->desc));

    printf(_("\n"
"Debugging options can be mixed using bitwise-or.\n"
"Note that the meanings and values are subject to change.\n"));
    m_output(stdout, _("<standard output>"));
    exit(0);
  }

  mask = debug_parse_mask(value);
  if (mask < 0)
    badusage(_("--%s requires a positive octal argument"), cpi->olong);
}

static void
set_no_pager(const struct cmdinfo *ci, const char *value)
{
  pager_enable(false);

  /* Let's communicate this to our backends. */
  setenv("DPKG_PAGER", "cat", 1);
}

static void
set_filter(const struct cmdinfo *cip, const char *value)
{
  filter_add(value, cip->arg_int);
}

static void
set_verify_format(const struct cmdinfo *cip, const char *value)
{
  if (!verify_set_output(value))
    badusage(_("unknown verify output format '%s'"), value);
}

static void
set_ignore_depends(const struct cmdinfo *cip, const char *value)
{
  char *copy, *p;

  copy= m_malloc(strlen(value)+2);
  strcpy(copy,value);
  copy[strlen(value) + 1] = '\0';
  for (p=copy; *p; p++) {
    if (*p != ',') continue;
    *p++ = '\0';
    if (!*p || *p==',' || p==copy+1)
      badusage(_("null package name in --%s comma-separated list '%.250s'"),
               cip->olong, value);
  }
  p= copy;
  while (*p) {
    struct pkginfo *pkg;

    pkg = dpkg_options_parse_pkgname(cip, p);
    pkg_list_prepend(&ignoredependss, pkg);

    p+= strlen(p)+1;
  }

  free(copy);
}

static void
set_integer(const struct cmdinfo *cip, const char *value)
{
  *cip->iassignto = dpkg_options_parse_arg_int(cip, value);
}

static void
set_pipe(const struct cmdinfo *cip, const char *value)
{
  long v;

  v = dpkg_options_parse_arg_int(cip, value);

  statusfd_add(v);
}

static bool
is_invoke_action(enum action action)
{
  switch (action) {
  case act_unpack:
  case act_configure:
  case act_install:
  case act_triggers:
  case act_remove:
  case act_purge:
  case act_arch_add:
  case act_arch_remove:
    return true;
  default:
    return false;
  }
}

static bool
can_invoke_hooks(enum action action)
{
  if (!is_invoke_action(action))
    return false;

  if (!f_act)
    return false;

  if (in_force(FORCE_NON_ROOT))
    return true;

  if (getuid() || geteuid())
    return false;

  return true;
}

static struct invoke_list pre_invoke_hooks = {
  .head = NULL,
  .tail = &pre_invoke_hooks.head,
};
static struct invoke_list post_invoke_hooks = {
  .head = NULL,
  .tail = &post_invoke_hooks.head,
};
static struct invoke_list status_loggers = {
  .head = NULL,
  .tail = &status_loggers.head,
};

static void
set_invoke_hook(const struct cmdinfo *cip, const char *value)
{
  struct invoke_list *hook_list = cip->arg_ptr;
  struct invoke_hook *hook_new;

  hook_new = m_malloc(sizeof(*hook_new));
  hook_new->command = m_strdup(value);
  hook_new->next = NULL;

  /* Add the new hook at the tail of the list to preserve the order. */
  *hook_list->tail = hook_new;
  hook_list->tail = &hook_new->next;
}

static void
run_invoke_hooks(const char *action, struct invoke_list *hook_list)
{
  struct invoke_hook *hook;

  setenv("DPKG_HOOK_ACTION", action, 1);

  for (hook = hook_list->head; hook; hook = hook->next) {
    int status;

    /* XXX: As an optimization, use exec instead if no shell metachar are
     * used “!$=&|\\`'"^~;<>{}[]()?*#”. */
    status = system(hook->command);
    if (status != 0)
      ohshit(_("error executing hook '%s', exit code %d"), hook->command,
             status);
  }

  unsetenv("DPKG_HOOK_ACTION");
}

static void
free_invoke_hooks(struct invoke_list *hook_list)
{
  struct invoke_hook *hook, *hook_next;

  for (hook = hook_list->head; hook; hook = hook_next) {
    hook_next = hook->next;
    free(hook->command);
    free(hook);
  }
}

static int
run_logger(struct invoke_hook *hook, const char *name)
{
  pid_t pid;
  int p[2];

  m_pipe(p);

  pid = subproc_fork();
  if (pid == 0) {
    /* Setup stdin and stdout. */
    m_dup2(p[0], 0);
    close(1);

    close(p[0]);
    close(p[1]);

    command_shell(hook->command, name);
  }
  close(p[0]);

  return p[1];
}

static void
run_status_loggers(struct invoke_list *hook_list)
{
  struct invoke_hook *hook;

  for (hook = hook_list->head; hook; hook = hook->next) {
    int fd;

    fd = run_logger(hook, _("status logger"));
    statusfd_add(fd);
  }
}

static int
arch_add(const char *const *argv)
{
  struct dpkg_arch *arch;
  const char *archname = *argv++;

  if (archname == NULL || *argv)
    badusage(_("--%s takes exactly one argument"), cipaction->olong);

  dpkg_arch_load_list();

  arch = dpkg_arch_add(archname);
  switch (arch->type) {
  case DPKG_ARCH_NATIVE:
  case DPKG_ARCH_FOREIGN:
    break;
  case DPKG_ARCH_ILLEGAL:
    ohshit(_("architecture '%s' is illegal: %s"), archname,
           dpkg_arch_name_is_illegal(archname));
  default:
    ohshit(_("architecture '%s' is reserved and cannot be added"), archname);
  }

  dpkg_arch_save_list();

  return 0;
}

static int
arch_remove(const char *const *argv)
{
  const char *archname = *argv++;
  struct dpkg_arch *arch;
  struct pkg_hash_iter *iter;
  struct pkginfo *pkg;

  if (archname == NULL || *argv)
    badusage(_("--%s takes exactly one argument"), cipaction->olong);

  modstatdb_open(msdbrw_readonly);

  arch = dpkg_arch_find(archname);
  if (arch->type != DPKG_ARCH_FOREIGN) {
    warning(_("cannot remove non-foreign architecture '%s'"), arch->name);
    return 0;
  }

  /* Check if it's safe to remove the architecture from the db. */
  iter = pkg_hash_iter_new();
  while ((pkg = pkg_hash_iter_next_pkg(iter))) {
    if (pkg->status < PKG_STAT_HALFINSTALLED)
      continue;
    if (pkg->installed.arch == arch) {
      if (in_force(FORCE_ARCHITECTURE))
        warning(_("removing architecture '%s' currently in use by database"),
                arch->name);
      else
        ohshit(_("cannot remove architecture '%s' currently in use by the database"),
               arch->name);
      break;
    }
  }
  pkg_hash_iter_free(iter);

  dpkg_arch_unmark(arch);
  dpkg_arch_save_list();

  modstatdb_shutdown();

  return 0;
}

int execbackend(const char *const *argv) DPKG_ATTR_NORET;
int commandfd(const char *const *argv);

/* This table has both the action entries in it and the normal options.
 * The action entries are made with the ACTION macro, as they all
 * have a very similar structure. */
static const struct cmdinfo cmdinfos[]= {
#define ACTIONBACKEND(longopt, shortopt, backend) \
 { longopt, shortopt, 0, NULL, NULL, setaction, 0, (void *)backend, execbackend }

  ACTION( "install",                        'i', act_install,              archivefiles    ),
  ACTION( "unpack",                          0,  act_unpack,               archivefiles    ),
  ACTION( "record-avail",                   'A', act_avail,                archivefiles    ),
  ACTION( "configure",                       0,  act_configure,            packages        ),
  ACTION( "remove",                         'r', act_remove,               packages        ),
  ACTION( "purge",                          'P', act_purge,                packages        ),
  ACTION( "triggers-only",                   0,  act_triggers,             packages        ),
  ACTION( "verify",                         'V', act_verify,               verify          ),
  ACTIONBACKEND( "listfiles",               'L', DPKGQUERY),
  ACTIONBACKEND( "status",                  's', DPKGQUERY),
  ACTION( "get-selections",                  0,  act_getselections,        getselections   ),
  ACTION( "set-selections",                  0,  act_setselections,        setselections   ),
  ACTION( "clear-selections",                0,  act_clearselections,      clearselections ),
  ACTIONBACKEND( "print-avail",             'p', DPKGQUERY),
  ACTION( "update-avail",                    0,  act_avreplace,            updateavailable ),
  ACTION( "merge-avail",                     0,  act_avmerge,              updateavailable ),
  ACTION( "clear-avail",                     0,  act_avclear,              updateavailable ),
  ACTION( "forget-old-unavail",              0,  act_forgetold,            forgetold       ),
  ACTION( "audit",                          'C', act_audit,                audit           ),
  ACTION( "yet-to-unpack",                   0,  act_unpackchk,            unpackchk       ),
  ACTIONBACKEND( "list",                    'l', DPKGQUERY),
  ACTIONBACKEND( "search",                  'S', DPKGQUERY),
  ACTION_MUX( "assert",                      0,  act_assert_feature,       assert_feature, &assert_feature_name),
  ACTION( "add-architecture",                0,  act_arch_add,             arch_add        ),
  ACTION( "remove-architecture",             0,  act_arch_remove,          arch_remove     ),
  ACTION( "print-architecture",              0,  act_printarch,            printarch   ),
  ACTION( "print-foreign-architectures",     0,  act_printforeignarches,   print_foreign_arches ),
  ACTION( "predep-package",                  0,  act_predeppackage,        predeppackage   ),
  ACTION( "validate-pkgname",                0,  act_validate_pkgname,     validate_pkgname ),
  ACTION( "validate-trigname",               0,  act_validate_trigname,    validate_trigname ),
  ACTION( "validate-archname",               0,  act_validate_archname,    validate_archname ),
  ACTION( "validate-version",                0,  act_validate_version,     validate_version ),
  ACTION( "compare-versions",                0,  act_cmpversions,          cmpversions     ),
/*
  ACTION( "command-fd",                   'c', act_commandfd,   commandfd     ),
*/
  ACTION( "help",                           '?', act_help,                 usage),
  ACTION( "version",                         0,  act_version,              printversion),

  { "pre-invoke",        0,   1, NULL,          NULL,      set_invoke_hook, 0, &pre_invoke_hooks },
  { "post-invoke",       0,   1, NULL,          NULL,      set_invoke_hook, 0, &post_invoke_hooks },
  { "path-exclude",      0,   1, NULL,          NULL,      set_filter,     0 },
  { "path-include",      0,   1, NULL,          NULL,      set_filter,     1 },
  { "verify-format",     0,   1, NULL,          NULL,      set_verify_format },
  { "status-logger",     0,   1, NULL,          NULL,      set_invoke_hook, 0, &status_loggers },
  { "status-fd",         0,   1, NULL,          NULL,      set_pipe, 0 },
  { "log",               0,   1, NULL,          &log_file, NULL,    0 },
  { "pending",           'a', 0, &f_pending,    NULL,      NULL,    1 },
  { "recursive",         'R', 0, &f_recursive,  NULL,      NULL,    1 },
  { "no-act",            0,   0, &f_act,        NULL,      NULL,    0 },
  { "dry-run",           0,   0, &f_act,        NULL,      NULL,    0 },
  { "simulate",          0,   0, &f_act,        NULL,      NULL,    0 },
  { "no-pager",          0,   0, NULL,          NULL,      set_no_pager,  0 },
  { "no-debsig",         0,   0, &f_debsig,     NULL,      NULL,    0 },
  /* Alias ('G') for --refuse. */
  {  NULL,               'G', 0, NULL,          NULL,      reset_force_option, FORCE_DOWNGRADE },
  { "selected-only",     'O', 0, &f_alsoselect, NULL,      NULL,    0 },
  { "triggers",           0,  0, &f_triggers,   NULL,      NULL,    1 },
  { "no-triggers",        0,  0, &f_triggers,   NULL,      NULL,   -1 },
  /* TODO: Remove ('N') sometime. */
  { "no-also-select",    'N', 0, &f_alsoselect, NULL,      NULL,    0 },
  { "skip-same-version", 'E', 0, &f_skipsame,   NULL,      NULL,    1 },
  { "auto-deconfigure",  'B', 0, &f_autodeconf, NULL,      NULL,    1 },
  { "robot",             0,   0, &f_robot,      NULL,      NULL,    1 },
  { "root",              0,   1, NULL,          NULL,      set_root,      0 },
  { "abort-after",       0,   1, &errabort,     NULL,      set_integer,   0 },
  { "admindir",          0,   1, NULL,          NULL,      set_admindir,  0 },
  { "instdir",           0,   1, NULL,          NULL,      set_instdir,   0 },
  { "ignore-depends",    0,   1, NULL,          NULL,      set_ignore_depends, 0 },
  { "force",             0,   2, NULL,          NULL,      set_force_option,   1 },
  { "refuse",            0,   2, NULL,          NULL,      set_force_option,   0 },
  { "no-force",          0,   2, NULL,          NULL,      set_force_option,   0 },
  { "debug",             'D', 1, NULL,          NULL,      set_debug,     0 },
  ACTIONBACKEND( "build",		'b', BACKEND),
  ACTIONBACKEND( "contents",		'c', BACKEND),
  ACTIONBACKEND( "control",		'e', BACKEND),
  ACTIONBACKEND( "info",		'I', BACKEND),
  ACTIONBACKEND( "field",		'f', BACKEND),
  ACTIONBACKEND( "extract",		'x', BACKEND),
  ACTIONBACKEND( "vextract",		'X', BACKEND),
  ACTIONBACKEND( "ctrl-tarfile",	0,   BACKEND),
  ACTIONBACKEND( "fsys-tarfile",	0,   BACKEND),
  { NULL,                0,   0, NULL,          NULL,      NULL,          0 }
};

int
execbackend(const char *const *argv)
{
  struct command cmd;

  command_init(&cmd, cipaction->arg_ptr, NULL);
  command_add_arg(&cmd, cipaction->arg_ptr);
  command_add_arg(&cmd, str_fmt("--%s", cipaction->olong));

  /* Explicitly separate arguments from options as any user-supplied
   * separator got stripped by the option parser */
  command_add_arg(&cmd, "--");
  command_add_argl(&cmd, (const char **)argv);

  command_exec(&cmd);
}

int
commandfd(const char *const *argv)
{
  struct varbuf linevb = VARBUF_INIT;
  const char * pipein;
  const char **newargs = NULL, **endargs;
  char *ptr, *endptr;
  FILE *in;
  long infd;
  int ret = 0;
  int c, lno, i;
  bool skipchar;

  pipein = *argv++;
  if (pipein == NULL || *argv)
    badusage(_("--%s takes exactly one argument"), cipaction->olong);

  infd = dpkg_options_parse_arg_int(cipaction, pipein);
  in = fdopen(infd, "r");
  if (in == NULL)
    ohshite(_("couldn't open '%i' for stream"), (int)infd);

  lno = 0;

  for (;;) {
    bool mode = false;
    int argc= 1;

    push_error_context();

    do {
      c = getc(in);
      if (c == '\n')
        lno++;
    } while (c != EOF && c_isspace(c));
    if (c == EOF) break;
    if (c == '#') {
      do { c= getc(in); if (c == '\n') lno++; } while (c != EOF && c != '\n');
      continue;
    }
    varbuf_reset(&linevb);
    do {
      varbuf_add_char(&linevb, c);
      c= getc(in);
      if (c == '\n') lno++;

      /* This isn't fully accurate, but overestimating can't hurt. */
      if (c_isspace(c))
        argc++;
    } while (c != EOF && c != '\n');
    if (c == EOF)
      ohshit(_("unexpected end of file before end of line %d"), lno);
    if (!argc) continue;
    newargs = m_realloc(newargs, sizeof(const char *) * (argc + 1));
    argc= 1;
    ptr= linevb.buf;
    endptr = ptr + linevb.used + 1;
    skipchar = false;
    while(ptr < endptr) {
      if (skipchar) {
	skipchar = false;
      } else if (*ptr == '\\') {
	memmove(ptr, (ptr+1), (linevb.used-(linevb.buf - ptr)-1));
	endptr--;
	skipchar = true;
	continue;
      } else if (c_isspace(*ptr)) {
	if (mode == true) {
	  *ptr = '\0';
	  mode = false;
	}
      } else {
	if (mode == false) {
	  newargs[argc]= ptr;
	  argc++;
	  mode = true;
	}
      }
      ptr++;
    }
    *ptr = '\0';
    newargs[argc++] = NULL;

    /* We strdup() each argument, but never free it, because the
     * error messages contain references back to these strings.
     * Freeing them, and reusing the memory, would make those
     * error messages confusing, to say the least. */
    for(i=1;i<argc;i++)
      if (newargs[i])
        newargs[i] = m_strdup(newargs[i]);
    endargs = newargs;

    setaction(NULL, NULL);
    dpkg_options_parse((const char *const **)&endargs, cmdinfos, printforhelp);
    if (!cipaction) badusage(_("need an action option"));

    ret |= cipaction->action(endargs);

    fsys_hash_reset();

    pop_error_context(ehflag_normaltidy);
  }

  fclose(in);

  return ret;
}

int main(int argc, const char *const *argv) {
  char *force_string;
  int ret;

  dpkg_locales_init(PACKAGE);
  dpkg_program_init("dpkg");
  set_force_default(FORCE_ALL);
  dpkg_options_load(DPKG, cmdinfos);
  dpkg_options_parse(&argv, cmdinfos, printforhelp);

  debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

  /* When running as root, make sure our primary group is also root, so
   * that files created by maintainer scripts have correct ownership. */
  if (!in_force(FORCE_NON_ROOT) && getuid() == 0 && getgid() != 0)
    if (setgid(0) < 0)
      ohshite(_("cannot set primary group ID to root"));

  if (!cipaction) badusage(_("need an action option"));

  /* Always set environment, to avoid possible security risks. */
  if (setenv("DPKG_ADMINDIR", dpkg_db_get_dir(), 1) < 0)
    ohshite(_("unable to setenv for subprocesses"));
  if (setenv("DPKG_ROOT", dpkg_fsys_get_dir(), 1) < 0)
    ohshite(_("unable to setenv for subprocesses"));
  force_string = get_force_string();
  if (setenv("DPKG_FORCE", force_string, 1) < 0)
    ohshite(_("unable to setenv for subprocesses"));
  free(force_string);

  if (!f_triggers)
    f_triggers = (cipaction->arg_int == act_triggers && *argv) ? -1 : 1;

  if (can_invoke_hooks(cipaction->arg_int)) {
    run_invoke_hooks(cipaction->olong, &pre_invoke_hooks);
    run_status_loggers(&status_loggers);
  }

  ret = cipaction->action(argv);

  if (can_invoke_hooks(cipaction->arg_int))
    run_invoke_hooks(cipaction->olong, &post_invoke_hooks);

  free_invoke_hooks(&pre_invoke_hooks);
  free_invoke_hooks(&post_invoke_hooks);

  dpkg_program_done();
  dpkg_locales_done();

  return reportbroken_retexitstatus(ret);
}
