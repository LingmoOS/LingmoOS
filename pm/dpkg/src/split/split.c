/*
 * dpkg-split - splitting and joining of multipart *.deb archives
 * split.c - splitting archives
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/parsedump.h>
#include <dpkg/path.h>
#include <dpkg/string.h>
#include <dpkg/subproc.h>
#include <dpkg/buffer.h>
#include <dpkg/ar.h>
#include <dpkg/options.h>

#include "dpkg-split.h"

/**
 * Parse the control file from a .deb package into a struct pkginfo.
 */
static struct pkginfo *
deb_parse_control(const char *filename)
{
	struct parsedb_state *ps;
	struct pkginfo *pkg;
	pid_t pid;
	int p[2];

	m_pipe(p);

	pid = subproc_fork();
	if (pid == 0) {
		/* Child writes to pipe. */
		m_dup2(p[1], 1);
		close(p[0]);
		close(p[1]);

		execlp(BACKEND, BACKEND, "--info", filename, "control", NULL);
		ohshite(_("unable to execute %s (%s)"),
		        _("package field value extraction"), BACKEND);
	}
	close(p[1]);

	/* Parent reads from pipe. */
	ps = parsedb_new(_("<dpkg-deb --info pipe>"), p[0], pdb_parse_binary);
	parsedb_load(ps);
	parsedb_parse(ps, &pkg);
	parsedb_close(ps);

	close(p[0]);

	subproc_reap(pid, _("package field value extraction"), SUBPROC_NOPIPE);

	return pkg;
}

static intmax_t
parse_timestamp(const char *value)
{
	intmax_t timestamp;
	char *end;

	errno = 0;
	timestamp = strtoimax(value, &end, 10);
	if (value == end || *end)
		ohshit(_("unable to parse timestamp '%.255s'"), value);
	else if (errno != 0)
		ohshite(_("unable to parse timestamp '%.255s'"), value);

	return timestamp;
}

/* Cleanup filename for use in crippled msdos systems. */
static char *
clean_msdos_filename(char *filename)
{
	char *d, *s;

	for (s = d = filename; *s; d++, s++) {
		if (*s == '+')
			*d = 'x';
		else if (c_isupper(*s))
			*d = c_tolower(*s);
		else if (c_islower(*s) || c_isdigit(*s))
			*d = *s;
		else
			s++;
	}

	return filename;
}

static int
mksplit(const char *file_src, const char *prefix, off_t maxpartsize,
        bool msdos)
{
	struct pkginfo *pkg;
	struct dpkg_error err;
	int fd_src;
	struct stat st;
	intmax_t timestamp;
	const char *timestamp_str;
	const char *version;
	char hash[MD5HASHLEN + 1];
	int nparts, curpart;
	off_t partsize;
	off_t cur_partsize, last_partsize;
	char *prefixdir = NULL, *msdos_prefix = NULL;
	struct varbuf file_dst = VARBUF_INIT;
	struct varbuf partmagic = VARBUF_INIT;
	struct varbuf partname = VARBUF_INIT;

	fd_src = open(file_src, O_RDONLY);
	if (fd_src < 0)
		ohshite(_("unable to open source file '%.250s'"), file_src);
	if (fstat(fd_src, &st))
		ohshite(_("unable to fstat source file"));
	if (!S_ISREG(st.st_mode))
		ohshit(_("source file '%.250s' not a plain file"), file_src);

	if (fd_md5(fd_src, hash, -1, &err) < 0)
		ohshit(_("cannot compute MD5 digest for file '%s': %s"),
		       file_src, err.str);
	lseek(fd_src, 0, SEEK_SET);

	pkg  = deb_parse_control(file_src);
	version = versiondescribe(&pkg->available.version, vdew_nonambig);

	timestamp_str = getenv("SOURCE_DATE_EPOCH");
	if (str_is_set(timestamp_str))
		timestamp = parse_timestamp(timestamp_str);
	else
		timestamp = time(NULL);

	partsize = maxpartsize - HEADERALLOWANCE;
	last_partsize = st.st_size % partsize;
	if (last_partsize == 0)
		last_partsize = partsize;
	nparts = (st.st_size + partsize - 1) / partsize;

	printf(P_("Splitting package %s into %d part: ",
	          "Splitting package %s into %d parts: ", nparts),
	       pkg->set->name, nparts);

	if (msdos) {
		char *t;

		t = m_strdup(prefix);
		prefixdir = m_strdup(dirname(t));
		free(t);

		msdos_prefix = m_strdup(path_basename(prefix));
		prefix = clean_msdos_filename(msdos_prefix);
	}

	for (curpart = 1; curpart <= nparts; curpart++) {
		struct dpkg_ar *ar;

		/* Generate output filename. */
		if (msdos) {
			char *refname;
			int prefix_max;

			refname = str_fmt("%dof%d", curpart, nparts);
			prefix_max = max(8 - strlen(refname), 0);
			varbuf_set_fmt(&file_dst, "%s/%.*s%.8s" DEBEXT,
			               prefixdir, prefix_max, prefix, refname);
			free(refname);
		} else {
			varbuf_set_fmt(&file_dst, "%s.%dof%d" DEBEXT,
			               prefix, curpart, nparts);
		}

		if (curpart == nparts)
			cur_partsize = last_partsize;
		else
			cur_partsize = partsize;

		if (cur_partsize > maxpartsize) {
			ohshit(_("header is too long, making part too long; "
			         "the package name or version\n"
			         "numbers must be extraordinarily long, "
			         "or something; giving up"));
		}

		/* Split the data. */
		ar = dpkg_ar_create(file_dst.buf, 0644);
		dpkg_ar_set_mtime(ar, timestamp);

		/* Write the ar header. */
		dpkg_ar_put_magic(ar);

		/* Write the debian-split part. */
		varbuf_set_fmt(&partmagic,
		               "%s\n%s\n%s\n%s\n%jd\n%jd\n%d/%d\n%s\n",
		               SPLITVERSION, pkg->set->name, version, hash,
		               (intmax_t)st.st_size, (intmax_t)partsize,
		               curpart, nparts, pkg->available.arch->name);
		dpkg_ar_member_put_mem(ar, PARTMAGIC,
		                       partmagic.buf, partmagic.used);

		/* Write the data part. */
		varbuf_set_fmt(&partname, "data.%d", curpart);
		dpkg_ar_member_put_file(ar, partname.buf,
		                        fd_src, cur_partsize);

		dpkg_ar_close(ar);

		printf("%d ", curpart);
	}

	varbuf_destroy(&file_dst);
	varbuf_destroy(&partname);
	varbuf_destroy(&partmagic);

	free(prefixdir);
	free(msdos_prefix);

	close(fd_src);

	printf(_("done\n"));

	return 0;
}

int
do_split(const char *const *argv)
{
	const char *sourcefile, *prefix;

	sourcefile = *argv++;
	if (!sourcefile)
		badusage(_("--split needs a source filename argument"));
	prefix = *argv++;
	if (prefix && *argv)
		badusage(_("--split takes at most a source filename and destination prefix"));
	if (!prefix) {
		size_t sourcefile_len = strlen(sourcefile);

		if (str_match_end(sourcefile, DEBEXT))
			sourcefile_len -= strlen(DEBEXT);

		prefix = nfstrnsave(sourcefile, sourcefile_len);
	}

	mksplit(sourcefile, prefix, opt_maxpartsize, opt_msdos);

	return 0;
}
