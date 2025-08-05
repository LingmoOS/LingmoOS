/*
 * dpkg-deb - construction and deconstruction of *.deb archives
 * dpkg-deb.h - external definitions for this program
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2012 Guillem Jover <guillem@debian.org>
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

#ifndef DPKG_DEB_H
#define DPKG_DEB_H

#include <dpkg/deb-version.h>

action_func do_build;
action_func do_contents;
action_func do_control;
action_func do_showinfo;
action_func do_info;
action_func do_field;
action_func do_extract;
action_func do_vextract;
action_func do_raw_extract;
action_func do_ctrltarfile;
action_func do_fsystarfile;

extern int opt_verbose;
extern int opt_root_owner_group;
extern int opt_uniform_compression;
extern int opt_debug;
extern int opt_check;
extern const char *opt_showformat;

extern struct deb_version deb_format;

enum DPKG_ATTR_ENUM_FLAGS dpkg_tar_options {
	/** Output the tar file directly, without any processing. */
	DPKG_TAR_PASSTHROUGH = 0,
	/** List tar files. */
	DPKG_TAR_LIST = DPKG_BIT(0),
	/** Extract tar files. */
	DPKG_TAR_EXTRACT = DPKG_BIT(1),
	/** Preserve tar permissions on extract. */
	DPKG_TAR_PERMS = DPKG_BIT(2),
	/** Do not set tar mtime on extract. */
	DPKG_TAR_NOMTIME = DPKG_BIT(3),
	/** Guarantee extraction into a new directory, abort if it exists. */
	DPKG_TAR_CREATE_DIR = DPKG_BIT(4),
};

void extracthalf(const char *debar, const char *dir,
                 enum dpkg_tar_options taroption, int admininfo);

extern struct compress_params compress_params_deb0;
extern struct compress_params compress_params;

#define ARCHIVEVERSION		"2.0"

#define BUILDCONTROLDIR		"DEBIAN"
#define EXTRACTCONTROLDIR	BUILDCONTROLDIR

#define OLDARCHIVEVERSION	"0.939000"

#define OLDDEBDIR		"DEBIAN"
#define OLDOLDDEBDIR		".DEBIAN"

#define DEBMAGIC		"debian-binary"
#define ADMINMEMBER		"control.tar"
#define DATAMEMBER		"data.tar"

#ifdef PATH_MAX
# define INTERPRETER_MAX PATH_MAX
#else
# define INTERPRETER_MAX 1024
#endif

#endif /* DPKG_DEB_H */
