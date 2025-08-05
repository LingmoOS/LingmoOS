/*
 * libdpkg - Debian packaging suite library routines
 * pkg-format.c - customizable package formatting
 *
 * Copyright © 2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2008-2015 Guillem Jover <guillem@debian.org>
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
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/error.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>
#include <dpkg/parsedump.h>
#include <dpkg/pkg-show.h>
#include <dpkg/pkg-format.h>

enum pkg_format_type {
	PKG_FORMAT_INVALID,
	PKG_FORMAT_STRING,
	PKG_FORMAT_FIELD,
};

struct pkg_format_node {
	struct pkg_format_node *next;
	enum pkg_format_type type;
	int width;
	char *data;
};


static struct pkg_format_node *
pkg_format_node_new(void)
{
	struct pkg_format_node *buf;

	buf = m_malloc(sizeof(*buf));
	buf->type = PKG_FORMAT_INVALID;
	buf->next = NULL;
	buf->data = NULL;
	buf->width = 0;

	return buf;
}

static bool
parsefield(struct pkg_format_node *node, const char *fmt, const char *fmtend,
           struct dpkg_error *err)
{
	int len;
	const char *ws;

	len = fmtend - fmt + 1;

	ws = memchr(fmt, ';', len);
	if (ws) {
		char *endptr;
		long w;

		errno = 0;
		w = strtol(ws + 1, &endptr, 0);
		if (endptr[0] != '}') {
			dpkg_put_error(err,
			               _("invalid character '%c' in field width"),
			               *endptr);
			return false;
		}
		if (w < INT_MIN || w > INT_MAX || errno == ERANGE) {
			dpkg_put_error(err, _("field width is out of range"));
			return false;
		}

		node->width = w;

		len = ws - fmt;
	}

	node->type = PKG_FORMAT_FIELD;
	node->data = m_malloc(len + 1);
	memcpy(node->data, fmt, len);
	node->data[len] = '\0';

	return true;
}

static bool
parsestring(struct pkg_format_node *node, const char *fmt, const char *fmtend,
            struct dpkg_error *err)
{
	int len;
	char *write;

	len = fmtend - fmt + 1;

	node->type = PKG_FORMAT_STRING;
	node->data = write = m_malloc(len + 1);
	while (fmt <= fmtend) {
		if (*fmt == '\\') {
			fmt++;
			switch (*fmt) {
			case 'n':
				*write = '\n';
				break;
			case 't':
				*write = '\t';
				break;
			case 'r':
				*write = '\r';
				break;
			case '\\':
			default:
				*write = *fmt;
				break;
			}
		} else
			*write = *fmt;
		write++;
		fmt++;
	}
	*write = '\0';

	return true;
}

void
pkg_format_free(struct pkg_format_node *head)
{
	struct pkg_format_node *node;

	while (head) {
		node = head;
		head = node->next;

		free(node->data);
		free(node);
	}
}

struct pkg_format_node *
pkg_format_parse(const char *fmt, struct dpkg_error *err)
{
	struct pkg_format_node *head, *node;
	const char *fmtend;

	head = node = NULL;

	while (*fmt) {
		if (node)
			node = node->next = pkg_format_node_new();
		else
			head = node = pkg_format_node_new();

		if (fmt[0] == '$' && fmt[1] == '{') {
			fmtend = strchr(fmt, '}');
			if (!fmtend) {
				dpkg_put_error(err, _("missing closing brace"));
				pkg_format_free(head);
				return NULL;
			}

			if (!parsefield(node, fmt + 2, fmtend - 1, err)) {
				pkg_format_free(head);
				return NULL;
			}
			fmt = fmtend + 1;
		} else {
			fmtend = fmt;
			do {
				fmtend += 1;
				fmtend = strchrnul(fmtend, '$');
			} while (fmtend[0] && fmtend[1] != '{');

			if (!parsestring(node, fmt, fmtend - 1, err)) {
				pkg_format_free(head);
				return NULL;
			}
			fmt = fmtend;
		}
	}

	if (!head)
		dpkg_put_error(err, _("may not be empty string"));

	return head;
}

static void
virt_package(struct varbuf *vb,
             const struct pkginfo *pkg, const struct pkgbin *pkgbin,
             enum fwriteflags flags, const struct fieldinfo *fip)
{
	varbuf_add_pkgbin_name(vb, pkg, pkgbin, pnaw_nonambig);
}

static void
virt_status_abbrev(struct varbuf *vb,
                   const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                   enum fwriteflags flags, const struct fieldinfo *fip)
{
	if (pkgbin != &pkg->installed)
		return;

	varbuf_add_char(vb, pkg_abbrev_want(pkg));
	varbuf_add_char(vb, pkg_abbrev_status(pkg));
	varbuf_add_char(vb, pkg_abbrev_eflag(pkg));
}

static void
virt_status_want(struct varbuf *vb,
                 const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                 enum fwriteflags flags, const struct fieldinfo *fip)
{
	if (pkgbin != &pkg->installed)
		return;

	varbuf_add_str(vb, pkg_want_name(pkg));
}

static void
virt_status_status(struct varbuf *vb,
                   const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                   enum fwriteflags flags, const struct fieldinfo *fip)
{
	if (pkgbin != &pkg->installed)
		return;

	varbuf_add_str(vb, pkg_status_name(pkg));
}

static void
virt_status_eflag(struct varbuf *vb,
                  const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                  enum fwriteflags flags, const struct fieldinfo *fip)
{
	if (pkgbin != &pkg->installed)
		return;

	varbuf_add_str(vb, pkg_eflag_name(pkg));
}

static void
virt_synopsis(struct varbuf *vb,
              const struct pkginfo *pkg, const struct pkgbin *pkgbin,
              enum fwriteflags flags, const struct fieldinfo *fip)
{
	const char *desc;
	int len;

	desc = pkgbin_synopsis(pkg, pkgbin, &len);

	varbuf_add_buf(vb, desc, len);
}

static void
virt_fsys_last_modified(struct varbuf *vb,
                        const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                        enum fwriteflags flags, const struct fieldinfo *fip)
{
	const char *listfile;
	struct stat st;
	intmax_t mtime;

	if (pkg->status == PKG_STAT_NOTINSTALLED)
		return;

	listfile = pkg_infodb_get_file(pkg, pkgbin, LISTFILE);

	if (stat(listfile, &st) < 0) {
		if (errno == ENOENT)
			return;

		ohshite(_("cannot get package %s filesystem last modification time"),
		        pkgbin_name_const(pkg, pkgbin, pnaw_nonambig));
	}

	mtime = st.st_mtime;
	varbuf_add_fmt(vb, "%jd", mtime);
}

/*
 * This function requires the caller to have loaded the package fsys metadata,
 * otherwise it will do nothing.
 */
static void
virt_fsys_files(struct varbuf *vb,
                const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                enum fwriteflags flags, const struct fieldinfo *fip)
{
	struct fsys_namenode_list *node;

	if (!pkg->files_list_valid)
		return;

	for (node = pkg->files; node; node = node->next) {
		varbuf_add_char(vb, ' ');
		varbuf_add_str(vb, node->namenode->name);
		varbuf_add_char(vb, '\n');
	}
}

static void
virt_source_package(struct varbuf *vb,
                    const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                    enum fwriteflags flags, const struct fieldinfo *fip)
{
	const char *name;
	size_t len;

	name = pkgbin->source;
	if (name == NULL)
		name = pkg->set->name;

	len = strcspn(name, " ");

	varbuf_add_buf(vb, name, len);
}

static void
virt_source_version(struct varbuf *vb,
                    const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                    enum fwriteflags flags, const struct fieldinfo *fip)
{
	varbuf_add_source_version(vb, pkg, pkgbin);
}

static void
virt_source_upstream_version(struct varbuf *vb,
                             const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                             enum fwriteflags flags, const struct fieldinfo *fip)
{
	struct dpkg_version version;

	pkg_source_version(&version, pkg, pkgbin);

	if (version.version)
		varbuf_add_str(vb, version.version);
}

static const struct fieldinfo virtinfos[] = {
	{ FIELD("binary:Package"), NULL, virt_package },
	{ FIELD("binary:Synopsis"), NULL, virt_synopsis },
	{ FIELD("binary:Summary"), NULL, virt_synopsis },
	{ FIELD("db:Status-Abbrev"), NULL, virt_status_abbrev },
	{ FIELD("db:Status-Want"), NULL, virt_status_want },
	{ FIELD("db:Status-Status"), NULL, virt_status_status },
	{ FIELD("db:Status-Eflag"), NULL, virt_status_eflag },
	{ FIELD("db-fsys:Files"), NULL, virt_fsys_files },
	{ FIELD("db-fsys:Last-Modified"), NULL, virt_fsys_last_modified },
	{ FIELD("source:Package"), NULL, virt_source_package },
	{ FIELD("source:Version"), NULL, virt_source_version },
	{ FIELD("source:Upstream-Version"), NULL, virt_source_upstream_version },
	{ NULL },
};

bool
pkg_format_needs_db_fsys(const struct pkg_format_node *head)
{
	const struct pkg_format_node *node;

	for (node = head; node; node = node->next) {
		if (node->type != PKG_FORMAT_FIELD)
			continue;
		if (strcmp(node->data, "db-fsys:Files") == 0)
			return true;
	}

	return false;
}

static void
pkg_format_item(struct varbuf *vb,
                const struct pkg_format_node *node, const char *str)
{
	if (node->width == 0)
		varbuf_add_str(vb, str);
	else
		varbuf_add_fmt(vb, "%*s", node->width, str);
}

void
pkg_format_print(struct varbuf *vb, const struct pkg_format_node *head,
                 struct pkginfo *pkg, struct pkgbin *pkgbin)
{
	const struct pkg_format_node *node;
	struct varbuf fb = VARBUF_INIT, wb = VARBUF_INIT;

	for (node = head; node; node = node->next) {
		bool ok = false;

		if (node->type == PKG_FORMAT_STRING) {
			pkg_format_item(&fb, node, node->data);
			ok = true;
		} else if (node->type == PKG_FORMAT_FIELD) {
			const struct fieldinfo *fip;

			fip = find_field_info(fieldinfos, node->data);
			if (fip == NULL)
				fip = find_field_info(virtinfos, node->data);

			if (fip) {
				fip->wcall(&wb, pkg, pkgbin, 0, fip);

				pkg_format_item(&fb, node, varbuf_str(&wb));
				varbuf_reset(&wb);
				ok = true;
			} else {
				const struct arbitraryfield *afp;

				afp = find_arbfield_info(pkgbin->arbs, node->data);
				if (afp) {
					pkg_format_item(&fb, node, afp->value);
					ok = true;
				}
			}
		}

		if (ok) {
			size_t len = fb.used;
			size_t width = abs(node->width);

			if ((width != 0) && (len > width))
				len = width;
			varbuf_add_buf(vb, varbuf_str(&fb), len);
		}

		varbuf_reset(&fb);
	}

	varbuf_destroy(&wb);
	varbuf_destroy(&fb);
}

void
pkg_format_show(const struct pkg_format_node *head,
                struct pkginfo *pkg, struct pkgbin *pkgbin)
{
	struct varbuf vb = VARBUF_INIT;

	pkg_format_print(&vb, head, pkg, pkgbin);

	if (vb.buf)
		fputs(vb.buf, stdout);

	varbuf_destroy(&vb);
}
