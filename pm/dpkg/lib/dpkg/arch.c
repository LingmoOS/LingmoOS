/*
 * libdpkg - Debian packaging suite library routines
 * arch.c - architecture database functions
 *
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
 * Copyright © 2011-2014 Guillem Jover <guillem@debian.org>
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

#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/ehandle.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/dir.h>
#include <dpkg/fsys.h>
#include <dpkg/varbuf.h>
#include <dpkg/arch.h>

#define DPKG_DB_ARCH_FILE "arch"
#define DPKG_DB_ARCH_NATIVE_FILE "arch-native"

/**
 * Verify if the architecture name is valid.
 *
 * Returns NULL if the architecture name is valid. Otherwise it returns a
 * string describing why it's not valid. Currently it ensures the name
 * starts with an alphanumeric and is then composed of a combinations of
 * hyphens and alphanumerics.
 *
 * The function will abort if you pass it a NULL pointer.
 *
 * @param name The architecture name to verify.
 */
const char *
dpkg_arch_name_is_illegal(const char *name)
{
	static char buf[150];
	const char *p = name;

	if (p == NULL)
		internerr("arch name argument is NULL");
	if (!*p)
		return _("may not be empty string");
	if (!c_isalnum(*p))
		return _("must start with an alphanumeric");
	while (*++p != '\0')
		if (!c_isalnum(*p) && *p != '-')
			break;
	if (*p == '\0')
		return NULL;

	snprintf(buf, sizeof(buf), _("character '%c' not allowed (only "
	                             "letters, digits and characters '%s')"),
	         *p, "-");
	return buf;
}

/* This is a special architecture used to guarantee we always have a valid
 * structure to handle. */
static struct dpkg_arch arch_item_none = {
	.name = "",
	.type = DPKG_ARCH_NONE,
	.next = NULL,
};
static struct dpkg_arch arch_item_empty = {
	.name = "",
	.type = DPKG_ARCH_EMPTY,
	.next = NULL,
};

static struct dpkg_arch arch_item_any = {
	.name = "any",
	.type = DPKG_ARCH_WILDCARD,
	.next = NULL,
};
static struct dpkg_arch arch_item_all = {
	.name = "all",
	.type = DPKG_ARCH_ALL,
	.next = &arch_item_any,
};
static struct dpkg_arch arch_item_native = {
	.name = ARCHITECTURE,
	.type = DPKG_ARCH_NATIVE,
	.next = &arch_item_all,
};
static struct dpkg_arch *arch_head = &arch_item_native;
static struct dpkg_arch *arch_builtin_tail = &arch_item_any;
static bool arch_list_dirty;

static struct dpkg_arch *
dpkg_arch_new(const char *name, enum dpkg_arch_type type)
{
	struct dpkg_arch *new;

	new = nfmalloc(sizeof(*new));
	new->next = NULL;
	new->name = nfstrsave(name);
	new->type = type;

	return new;
}

/**
 * Retrieve the struct dpkg_arch for the given architecture.
 *
 * Create a new structure for the architecture if it is not yet known from
 * the system, in that case it will have type == DPKG_ARCH_UNKNOWN, if the
 * architecture is illegal it will have type == DPKG_ARCH_ILLEGAL, if name
 * is an empty string it will have type == DPKG_ARCH_EMPTY, and if it is
 * NULL then it will have type == DPKG_ARCH_NONE.
 *
 * @param name The architecture name.
 */
struct dpkg_arch *
dpkg_arch_find(const char *name)
{
	struct dpkg_arch *arch, *last_arch = NULL;
	enum dpkg_arch_type type;

	if (name == NULL)
		return &arch_item_none;
	if (name[0] == '\0')
		return &arch_item_empty;

	for (arch = arch_head; arch; arch = arch->next) {
		if (strcmp(arch->name, name) == 0)
			return arch;
		last_arch = arch;
	}

	if (dpkg_arch_name_is_illegal(name))
		type = DPKG_ARCH_ILLEGAL;
	else
		type = DPKG_ARCH_UNKNOWN;

	arch = dpkg_arch_new(name, type);
	last_arch->next = arch;

	return arch;
}

/**
 * Return the struct dpkg_arch corresponding to the architecture type.
 *
 * The function only returns instances for types which are unique. For
 * forward-compatibility any unknown type will return NULL.
 */
struct dpkg_arch *
dpkg_arch_get(enum dpkg_arch_type type)
{
	switch (type) {
	case DPKG_ARCH_NONE:
		return &arch_item_none;
	case DPKG_ARCH_EMPTY:
		return &arch_item_empty;
	case DPKG_ARCH_WILDCARD:
		return &arch_item_any;
	case DPKG_ARCH_ALL:
		return &arch_item_all;
	case DPKG_ARCH_NATIVE:
		return &arch_item_native;
	case DPKG_ARCH_ILLEGAL:
	case DPKG_ARCH_FOREIGN:
	case DPKG_ARCH_UNKNOWN:
		internerr("architecture type %d is not unique", type);
	default:
		/* Ignore unknown types for forward-compatibility. */
		return NULL;
	}
}

/**
 * Return the complete list of architectures.
 *
 * In fact it returns the first item of the linked list and you can
 * traverse the list by following arch->next until it's NULL.
 */
struct dpkg_arch *
dpkg_arch_get_list(void)
{
	return arch_head;
}

/**
 * Reset the list of architectures.
 *
 * Must be called before nffreeall() to ensure we don't point to
 * unallocated memory.
 */
void
dpkg_arch_reset_list(void)
{
	arch_builtin_tail->next = NULL;
	arch_list_dirty = false;
}

void
varbuf_add_archqual(struct varbuf *vb, const struct dpkg_arch *arch)
{
	if (arch->type == DPKG_ARCH_NONE)
		return;
	if (arch->type == DPKG_ARCH_EMPTY)
		return;

	varbuf_add_char(vb, ':');
	varbuf_add_str(vb, arch->name);
}

/**
 * Return a descriptive architecture name.
 */
const char *
dpkg_arch_describe(const struct dpkg_arch *arch)
{
	if (arch->type == DPKG_ARCH_NONE)
		return C_("architecture", "<none>");
	if (arch->type == DPKG_ARCH_EMPTY)
		return C_("architecture", "<empty>");

	return arch->name;
}

/**
 * Add a new foreign dpkg_arch architecture.
 */
struct dpkg_arch *
dpkg_arch_add(const char *name)
{
	struct dpkg_arch *arch;

	arch = dpkg_arch_find(name);
	if (arch->type == DPKG_ARCH_UNKNOWN) {
		arch->type = DPKG_ARCH_FOREIGN;
		arch_list_dirty = true;
	}

	return arch;
}

/**
 * Unmark a foreign dpkg_arch architecture.
 */
void
dpkg_arch_unmark(const struct dpkg_arch *arch_remove)
{
	struct dpkg_arch *arch;

	for (arch = arch_builtin_tail->next; arch; arch = arch->next) {
		if (arch->type != DPKG_ARCH_FOREIGN)
			continue;

		if (arch == arch_remove) {
			arch->type = DPKG_ARCH_UNKNOWN;
			arch_list_dirty = true;
			return;
		}
	}
}

/**
 * Load the native architecture name for a non-default root directory.
 */
void
dpkg_arch_load_native(void)
{
	struct varbuf arch_line = VARBUF_INIT;
	struct dpkg_arch *arch;
	char *archfile;
	int rc;

	/* We only honor this file on chroots, for the non-chroot case
	 * the builtin arch is always honored. */
	if (str_is_unset(dpkg_fsys_get_dir()))
		return;

	archfile = dpkg_db_get_path(DPKG_DB_ARCH_NATIVE_FILE);
	rc = file_slurp(archfile, &arch_line, NULL);
	free(archfile);
	if (rc < 0)
		return;

	if (arch_line.used == 0)
		return;

	if (arch_line.buf[arch_line.used - 1] == '\n')
		varbuf_trunc(&arch_line, arch_line.used - 1);

	/* Override the native architecture name. */
	arch = dpkg_arch_get(DPKG_ARCH_NATIVE);
	arch->name = varbuf_detach(&arch_line);
}

/**
 * Load the architecture database.
 */
void
dpkg_arch_load_list(void)
{
	FILE *fp;
	char *archfile;
	char archname[_POSIX2_LINE_MAX];

	dpkg_arch_load_native();

	archfile = dpkg_db_get_path(DPKG_DB_ARCH_FILE);
	fp = fopen(archfile, "r");
	if (fp == NULL) {
		arch_list_dirty = true;
		free(archfile);
		return;
	}

	while (fgets_checked(archname, sizeof(archname), fp, archfile) >= 0)
		dpkg_arch_add(archname);

	free(archfile);
	fclose(fp);
}

/**
 * Save the architecture database.
 */
void
dpkg_arch_save_list(void)
{
	struct atomic_file *file;
	struct dpkg_arch *arch;
	char *archfile;

	if (!arch_list_dirty)
		return;

	archfile = dpkg_db_get_path(DPKG_DB_ARCH_FILE);
	file = atomic_file_new(archfile, ATOMIC_FILE_MKPATH);
	atomic_file_open(file);

	for (arch = arch_head; arch; arch = arch->next) {
		if (arch->type != DPKG_ARCH_FOREIGN &&
		    arch->type != DPKG_ARCH_NATIVE)
			continue;

		if (fprintf(file->fp, "%s\n", arch->name) < 0)
			ohshite(_("error writing to architecture list"));
	}

	atomic_file_sync(file);
	atomic_file_close(file);
	atomic_file_commit(file);
	atomic_file_free(file);

	dir_sync_path(dpkg_db_get_dir());

	arch_list_dirty = false;

	free(archfile);
}
