/*
 * libdpkg - Debian packaging suite library routines
 * trigdeferred.c - parsing of triggers/Unincorp (was …/Deferred)
 *
 * Copyright © 2007 Canonical Ltd
 * written by Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2016 Guillem Jover <guillem@debian.org>
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

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/c-ctype.h>
#include <dpkg/file.h>
#include <dpkg/dir.h>
#include <dpkg/trigdeferred.h>
#include <dpkg/triglib.h>

static struct varbuf fn, newfn;

static const struct trigdefmeths *trigdef;

/*---------- Deferred file handling ----------*/

static char *triggersdir;
static int lock_fd = -1;
static FILE *old_deferred;
static FILE *trig_new_deferred;

static void
constructfn(struct varbuf *vb, const char *dir, const char *tail)
{
	varbuf_reset(vb);
	varbuf_add_dir(vb, dir);
	varbuf_add_str(vb, tail);
}

/**
 * Start processing of the triggers deferred file.
 *
 * @retval -1 Lock ENOENT with O_CREAT (directory does not exist).
 * @retval -2 Unincorp empty, TDUF_WRITE_IF_EMPTY unset.
 * @retval -3 Unincorp ENOENT, TDUF_WRITE_IF_ENOENT unset.
 * @retval  1 Unincorp ENOENT, TDUF_WRITE_IF_ENOENT set.
 * @retval  2 Ok.
 *
 * For positive return values the caller must call trigdef_update_done!
 */
enum trigdef_update_status
trigdef_update_start(enum trigdef_update_flags uf)
{
	free(triggersdir);
	triggersdir = dpkg_db_get_path(TRIGGERSDIR);

	if (uf & TDUF_WRITE) {
		constructfn(&fn, triggersdir, TRIGGERSLOCKFILE);
		if (lock_fd < 0) {
			lock_fd = open(fn.buf, O_RDWR | O_CREAT | O_TRUNC, 0600);
			if (lock_fd < 0) {
				if (!(errno == ENOENT && (uf & TDUF_NO_LOCK_OK)))
					ohshite(_("unable to open/create "
					          "triggers lock file '%.250s'"),
					        fn.buf);
				return TDUS_ERROR_NO_DIR;
			}
		}

		file_lock(&lock_fd, FILE_LOCK_WAIT, fn.buf,
		          _("triggers database lock"));
	}

	constructfn(&fn, triggersdir, TRIGGERSDEFERREDFILE);

	if (old_deferred)
		fclose(old_deferred);
	old_deferred = fopen(fn.buf, "r");
	if (!old_deferred) {
		if (errno != ENOENT)
			ohshite(_("unable to open triggers deferred file '%.250s'"),
			        fn.buf);
		if (!(uf & TDUF_WRITE_IF_ENOENT)) {
			if (uf & TDUF_WRITE)
				pop_cleanup(ehflag_normaltidy);
			return TDUS_ERROR_NO_DEFERRED;
		}
	} else {
		struct stat stab;
		int rc;

		setcloexec(fileno(old_deferred), fn.buf);

		rc = fstat(fileno(old_deferred), &stab);
		if (rc < 0)
			ohshite(_("unable to stat triggers deferred file '%.250s'"),
			        fn.buf);

		if (stab.st_size == 0 && !(uf & TDUF_WRITE_IF_EMPTY)) {
			if (uf & TDUF_WRITE)
				pop_cleanup(ehflag_normaltidy);
			return TDUS_ERROR_EMPTY_DEFERRED;
		}
	}

	if (uf & TDUF_WRITE) {
		constructfn(&newfn, triggersdir, TRIGGERSDEFERREDFILE ".new");
		if (trig_new_deferred)
			fclose(trig_new_deferred);
		trig_new_deferred = fopen(newfn.buf, "w");
		if (!trig_new_deferred)
			ohshite(_("unable to open/create new triggers deferred file '%.250s'"),
			        newfn.buf);

		setcloexec(fileno(trig_new_deferred), newfn.buf);
	}

	if (!old_deferred)
		return TDUS_NO_DEFERRED;

	return TDUS_OK;
}

void
trigdef_set_methods(const struct trigdefmeths *methods)
{
	trigdef = methods;
}

void
trigdef_update_printf(const char *format, ...)
{
	va_list ap;

	if (trig_new_deferred == NULL)
		return;

	va_start(ap, format);
	vfprintf(trig_new_deferred, format, ap);
	va_end(ap);
}

static void
trigdef_parse_error(int line_num, const char *line, const char *ptr)
{
	ohshit(_("syntax error in triggers deferred file '%.250s' at "
	         "line %d character %zd '%s'"),
	       fn.buf, line_num, ptr - line + 1, ptr);
}

/* Trim leading space. */
static char *
trigdef_skip_whitespace(char *ptr)
{
	while (*ptr) {
		if (!c_iswhite(*ptr))
			break;
		ptr++;
	}

	return ptr;
}

int
trigdef_parse(void)
{
	char line[_POSIX2_LINE_MAX];
	char *ptr, *ptr_ini;
	int line_num = 0;

	while (fgets_checked(line, sizeof(line), old_deferred, fn.buf) > 0) {
		line_num++;

		ptr = trigdef_skip_whitespace(line);

		/* Skip comments and empty lines. */
		if (*ptr == '\0' || *ptr == '#')
			continue;

		/* Parse the trigger directive. */
		ptr_ini = ptr;
		while (*ptr) {
			if (*ptr < 0x21 || *ptr > 0x7e)
				break;
			ptr++;
		}

		if (*ptr == '\0' || ptr_ini == ptr)
			trigdef_parse_error(line_num, line, ptr);
		*ptr++ = '\0';

		/* Set the trigger directive. */
		trigdef->trig_begin(ptr_ini);

		/* Parse the package names. */
		while (*ptr) {
			ptr = trigdef_skip_whitespace(ptr);

			ptr_ini = ptr;
			if (*ptr == '\0' ||
			    !(c_isdigit(*ptr) || c_islower(*ptr) || *ptr == '-'))
				trigdef_parse_error(line_num, line, ptr);

			while (*++ptr) {
				if (!c_isdigit(*ptr) && !c_islower(*ptr) &&
				    *ptr != '-' && *ptr != ':' &&
				    *ptr != '+' && *ptr != '.')
					break;
			}

			if (*ptr != '\0' && *ptr != '#' && !c_iswhite(*ptr))
				trigdef_parse_error(line_num, line, ptr);
			*ptr++ = '\0';

			if (ptr_ini[0] == '-' && ptr_ini[1])
				ohshit(_("invalid package name '%.250s' in "
				         "triggers deferred file '%.250s'"),
				       ptr_ini, fn.buf);

			/* Set the package name. */
			trigdef->package(ptr_ini);
		}

		trigdef->trig_end();
	}

	return 0;
}

void
trigdef_process_done(void)
{
	if (old_deferred) {
		if (ferror(old_deferred))
			ohshite(_("error reading triggers deferred file '%.250s'"),
			        fn.buf);
		fclose(old_deferred);
		old_deferred = NULL;
	}

	if (trig_new_deferred) {
		int rc;

		if (ferror(trig_new_deferred))
			ohshite(_("unable to write new triggers deferred "
			          "file '%.250s'"), newfn.buf);
		rc = fclose(trig_new_deferred);
		trig_new_deferred = NULL;
		if (rc)
			ohshite(_("unable to close new triggers deferred "
			          "file '%.250s'"), newfn.buf);

		if (rename(newfn.buf, fn.buf))
			ohshite(_("unable to install new triggers deferred "
			          "file '%.250s'"), fn.buf);

		dir_sync_path(triggersdir);
	}

	free(triggersdir);
	triggersdir = NULL;

	/* Unlock. */
	if (lock_fd >= 0)
		pop_cleanup(ehflag_normaltidy);
}
