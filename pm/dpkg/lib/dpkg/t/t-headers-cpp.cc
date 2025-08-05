/*
 * libdpkg - Debian packaging suite library routines
 * t-headers-cpp.cc - test C++ inclusion of headers
 *
 * Copyright © 2018 Guillem Jover <guillem@debian.org>
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

#include <dpkg/ar.h>
#include <dpkg/arch.h>
#include <dpkg/atomic-file.h>
#include <dpkg/buffer.h>
#include <dpkg/c-ctype.h>
#include <dpkg/color.h>
#include <dpkg/command.h>
#include <dpkg/compress.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>
#include <dpkg/deb-version.h>
#include <dpkg/debug.h>
#include <dpkg/dir.h>
#include <dpkg/dlist.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/dpkg.h>
#include <dpkg/ehandle.h>
#include <dpkg/error.h>
#include <dpkg/fdio.h>
#include <dpkg/file.h>
#include <dpkg/fsys.h>
#include <dpkg/glob.h>
#include <dpkg/i18n.h>
#include <dpkg/macros.h>
#include <dpkg/namevalue.h>
#include <dpkg/options.h>
#include <dpkg/pager.h>
#include <dpkg/parsedump.h>
#include <dpkg/path.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-files.h>
#include <dpkg/pkg-format.h>
#include <dpkg/pkg-list.h>
#include <dpkg/pkg-queue.h>
#include <dpkg/pkg-show.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/pkg.h>
#include <dpkg/progname.h>
#include <dpkg/program.h>
#include <dpkg/progress.h>
#include <dpkg/report.h>
#include <dpkg/string.h>
#include <dpkg/subproc.h>
#include <dpkg/tarfn.h>
#include <dpkg/test.h>
#include <dpkg/treewalk.h>
#include <dpkg/trigdeferred.h>
#include <dpkg/triglib.h>
#include <dpkg/varbuf.h>
#include <dpkg/version.h>

TEST_ENTRY(test)
{
	test_plan(1);

	test_pass(true);
}
