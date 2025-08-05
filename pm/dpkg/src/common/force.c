/*
 * dpkg - main program for package management
 * force.c - force operation support
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2019 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/options.h>

#include "force.h"

static int force_mask;
static int force_flags;

enum forcetype {
	FORCETYPE_DISABLED,
	FORCETYPE_ENABLED,
	FORCETYPE_DAMAGE,
};

static const char *
forcetype_str(enum forcetype type)
{
	switch (type) {
	case FORCETYPE_DISABLED:
		return "   ";
	case FORCETYPE_ENABLED:
		return "[*]";
	case FORCETYPE_DAMAGE:
		return "[!]";
	default:
		internerr("unknown force type '%d'", type);
	}
}

static const struct forceinfo {
	const char *name;
	int flag;
	char type;
	const char *desc;
} forceinfos[] = {
	{
		"all",
		FORCE_ALL,
		FORCETYPE_DAMAGE,
		N_("Set all force options"),
	}, {
		"security-mac",
		FORCE_SECURITY_MAC,
		FORCETYPE_ENABLED,
		N_("Use MAC based security if available"),
	}, {
		"downgrade",
		FORCE_DOWNGRADE,
		FORCETYPE_ENABLED,
		N_("Replace a package with a lower version"),
	}, {
		"configure-any",
		FORCE_CONFIGURE_ANY,
		FORCETYPE_DISABLED,
		N_("Configure any package which may help this one"),
	}, {
		"hold",
		FORCE_HOLD,
		FORCETYPE_DISABLED,
		N_("Install or remove incidental packages even when on hold"),
	}, {
		"not-root",
		FORCE_NON_ROOT,
		FORCETYPE_DISABLED,
		N_("Try to (de)install things even when not root"),
	}, {
		"bad-path",
		FORCE_BAD_PATH,
		FORCETYPE_DISABLED,
		N_("PATH is missing important programs, problems likely"),
	}, {
		"bad-verify",
		FORCE_BAD_VERIFY,
		FORCETYPE_DISABLED,
		N_("Install a package even if it fails authenticity check"),
	}, {
		"bad-version",
		FORCE_BAD_VERSION,
		FORCETYPE_DISABLED,
		N_("Process even packages with wrong versions"),
	}, {
		"statoverride-add",
		FORCE_STATOVERRIDE_ADD,
		FORCETYPE_DISABLED,
		N_("Overwrite an existing stat override when adding it"),
	}, {
		"statoverride-remove",
		FORCE_STATOVERRIDE_DEL,
		FORCETYPE_DISABLED,
		N_("Ignore a missing stat override when removing it"),
	}, {
		"overwrite",
		FORCE_OVERWRITE,
		FORCETYPE_DISABLED,
		N_("Overwrite a file from one package with another"),
	}, {
		"overwrite-diverted",
		FORCE_OVERWRITE_DIVERTED,
		FORCETYPE_DISABLED,
		N_("Overwrite a diverted file with an undiverted version"),
	}, {
		"overwrite-dir",
		FORCE_OVERWRITE_DIR,
		FORCETYPE_DAMAGE,
		N_("Overwrite one package's directory with another's file"),
	}, {
		"unsafe-io",
		FORCE_UNSAFE_IO,
		FORCETYPE_DAMAGE,
		N_("Do not perform safe I/O operations when unpacking"),
	}, {
		"script-chrootless",
		FORCE_SCRIPT_CHROOTLESS,
		FORCETYPE_DAMAGE,
		N_("Do not chroot into maintainer script environment"),
	}, {
		"confnew",
		FORCE_CONFF_NEW,
		FORCETYPE_DAMAGE,
		N_("Always use the new config files, don't prompt"),
	}, {
		"confold",
		FORCE_CONFF_OLD,
		FORCETYPE_DAMAGE,
		N_("Always use the old config files, don't prompt"),
	}, {
		"confdef",
		FORCE_CONFF_DEF,
		FORCETYPE_DAMAGE,
		N_("Use the default option for new config files if one\n"
		   "is available, don't prompt. If no default can be found,\n"
		   "you will be prompted unless one of the confold or\n"
		   "confnew options is also given"),
	}, {
		"confmiss",
		FORCE_CONFF_MISS,
		FORCETYPE_DAMAGE,
		N_("Always install missing config files"),
	}, {
		"confask",
		FORCE_CONFF_ASK,
		FORCETYPE_DAMAGE,
		N_("Offer to replace config files with no new versions"),
	}, {
		"architecture",
		FORCE_ARCHITECTURE,
		FORCETYPE_DAMAGE,
		N_("Process even packages with wrong or no architecture"),
	}, {
		"breaks",
		FORCE_BREAKS,
		FORCETYPE_DAMAGE,
		N_("Install even if it would break another package"),
	}, {
		"conflicts",
		FORCE_CONFLICTS,
		FORCETYPE_DAMAGE,
		N_("Allow installation of conflicting packages"),
	}, {
		"depends",
		FORCE_DEPENDS,
		FORCETYPE_DAMAGE,
		N_("Turn all dependency problems into warnings"),
	}, {
		"depends-version",
		FORCE_DEPENDS_VERSION,
		FORCETYPE_DAMAGE,
		N_("Turn dependency version problems into warnings"),
	}, {
		"remove-reinstreq",
		FORCE_REMOVE_REINSTREQ,
		FORCETYPE_DAMAGE,
		N_("Remove packages which require installation"),
	}, {
		"remove-protected",
		FORCE_REMOVE_PROTECTED,
		FORCETYPE_DAMAGE,
		N_("Remove a protected package"),
	}, {
		"remove-essential",
		FORCE_REMOVE_ESSENTIAL,
		FORCETYPE_DAMAGE,
		N_("Remove an essential package"),
	}, {
		NULL
	}
};

bool
in_force(int flags)
{
	return (flags & force_flags) == flags;
}

void
set_force(int flags)
{
	force_flags |= flags;
}

void
reset_force(int flags)
{
	force_flags &= ~flags;
}

char *
get_force_string(void)
{
	const struct forceinfo *fip;
	struct varbuf vb = VARBUF_INIT;

	for (fip = forceinfos; fip->name; fip++) {
		if ((enum force_flags)fip->flag == FORCE_ALL ||
		    (fip->flag & force_mask) != fip->flag ||
		    !in_force(fip->flag))
			continue;

		if (vb.used)
			varbuf_add_char(&vb, ',');
		varbuf_add_str(&vb, fip->name);
	}

	return varbuf_detach(&vb);
}

static inline void
print_forceinfo_line(int type, const char *name, const char *desc)
{
	printf("  %s %-18s %s\n", forcetype_str(type), name, desc);
}

static void
print_forceinfo(const struct forceinfo *fi)
{
	char *desc, *line;

	desc = m_strdup(gettext(fi->desc));

	line = strtok(desc, "\n");
	print_forceinfo_line(fi->type, fi->name, line);
	while ((line = strtok(NULL, "\n")))
		print_forceinfo_line(FORCETYPE_DISABLED, "", line);

	free(desc);
}

void
parse_force(const char *value, bool set)
{
	const struct forceinfo *fip;

	if (strcmp(value, "help") == 0) {
		char *force_string = get_force_string();

		printf(_(
"%s forcing options - control behavior when problems found:\n"
"  warn but continue:  --force-<thing>,<thing>,...\n"
"  stop with error:    --refuse-<thing>,<thing>,... | --no-force-<thing>,...\n"
" Forcing things:\n"), dpkg_get_progname());

		for (fip = forceinfos; fip->name; fip++)
			if ((enum force_flags)fip->flag == FORCE_ALL ||
			    (fip->flag & force_mask) == fip->flag)
				print_forceinfo(fip);

		printf(_(
"\n"
"WARNING - use of options marked [!] can seriously damage your installation.\n"
"Forcing options marked [*] are enabled by default.\n"));
		m_output(stdout, _("<standard output>"));

		printf(_(
"\n"
"Currently enabled options:\n"
" %s\n"), force_string);

		free(force_string);

		exit(0);
	}

	for (;;) {
		const char *comma;
		size_t l;

		comma = strchrnul(value, ',');
		l = (size_t)(comma - value);
		for (fip = forceinfos; fip->name; fip++)
			if (strncmp(fip->name, value, l) == 0 &&
			    strlen(fip->name) == l)
				break;

		if (!fip->name) {
			badusage(_("unknown force/refuse option '%.*s'"),
			         (int)min(l, 250), value);
		} else if (fip->flag) {
			if (set)
				set_force(fip->flag);
			else
				reset_force(fip->flag);
		} else {
			warning(_("obsolete force/refuse option '%s'"),
			        fip->name);
		}

		if (*comma == '\0')
			break;
		value = ++comma;
	}
}

void
set_force_default(int mask)
{
	const char *force_env;
	const struct forceinfo *fip;

	force_mask = mask;

	/* If we get passed force options from the environment, do not
	 * initialize from the built-in defaults. */
	force_env = getenv("DPKG_FORCE");
	if (force_env != NULL) {
		if (force_env[0] != '\0')
			parse_force(force_env, 1);
		return;
	}

	for (fip = forceinfos; fip->name; fip++)
		if (fip->type == FORCETYPE_ENABLED)
			set_force(fip->flag);
}

void
set_force_option(const struct cmdinfo *cip, const char *value)
{
	bool set = cip->arg_int;

	parse_force(value, set);
}

void
reset_force_option(const struct cmdinfo *cip, const char *value)
{
	reset_force(cip->arg_int);
}

void
forcibleerr(int forceflag, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (in_force(forceflag)) {
		warning(_("overriding problem because --force enabled:"));
		warningv(fmt, args);
	} else {
		ohshitv(fmt, args);
	}
	va_end(args);
}

int
forcible_nonroot_error(int rc)
{
	if (in_force(FORCE_NON_ROOT) && errno == EPERM)
		return 0;
	return rc;
}
