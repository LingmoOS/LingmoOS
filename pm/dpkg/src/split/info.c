/*
 * dpkg-split - splitting and joining of multipart *.deb archives
 * info.c - information about split archives
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/fdio.h>
#include <dpkg/ar.h>
#include <dpkg/options.h>

#include "dpkg-split.h"

static intmax_t
parse_intmax(const char *value, const char *fn, const char *what)
{
  intmax_t ret;
  char *endp;

  errno = 0;
  ret = strtoimax(value, &endp, 10);
  if (value == endp || *endp)
    ohshit(_("file '%.250s' is corrupt - bad digit (code %d) in %s"),
           fn, *endp, what);
  if (ret < 0 || errno == ERANGE)
    ohshit(_("file '%s' is corrupt; out of range integer in %s"), fn, what);

  return ret;
}

static char *nextline(char **ripp, const char *fn, const char *what) {
  char *newline, *rip;

  rip= *ripp;
  if (!rip)
    ohshit(_("file '%.250s' is corrupt - %.250s missing"), fn, what);
  newline= strchr(rip,'\n');
  if (!newline)
    ohshit(_("file '%.250s' is corrupt - missing newline after %.250s"),
           fn, what);
  *ripp= newline+1;

  str_rtrim_spaces(rip, newline);

  return rip;
}

/**
 * Read a deb-split part archive.
 *
 * @return Part info (nfmalloc'd) if was an archive part and we read it,
 *         NULL if it wasn't.
 */
struct partinfo *
read_info(struct dpkg_ar *ar, struct partinfo *ir)
{
  static struct varbuf format_member = VARBUF_INIT;

  size_t thisilen;
  intmax_t templong;
  char magicbuf[sizeof(DPKG_AR_MAGIC) - 1], *rip, *partnums, *slash;
  const char *err;
  struct dpkg_ar_hdr arh;
  ssize_t rc;

  rc = fd_read(ar->fd, magicbuf, sizeof(magicbuf));
  if (rc != sizeof(magicbuf)) {
    if (rc < 0)
      ohshite(_("error reading %.250s"), ar->name);
    else
      return NULL;
  }
  if (memcmp(magicbuf, DPKG_AR_MAGIC, sizeof(magicbuf)))
    return NULL;

  rc = fd_read(ar->fd, &arh, sizeof(arh));
  if (rc != sizeof(arh))
    read_fail(rc, ar->name, "ar header");

  dpkg_ar_normalize_name(&arh);

  if (strncmp(arh.ar_name, PARTMAGIC, sizeof(arh.ar_name)) != 0)
    return NULL;
  if (dpkg_ar_member_is_illegal(&arh))
    ohshit(_("file '%.250s' is corrupt - bad magic at end of first header"),
           ar->name);
  thisilen = dpkg_ar_member_get_size(ar, &arh);

  varbuf_reset(&format_member);
  varbuf_grow(&format_member, thisilen + 2);

  rc = fd_read(ar->fd, format_member.buf, thisilen + (thisilen & 1));
  if (rc != (ssize_t)(thisilen + (thisilen & 1)))
    read_fail(rc, ar->name, "reading header member");
  if (thisilen & 1) {
    int c = format_member.buf[thisilen];

    if (c != '\n')
      ohshit(_("file '%.250s' is corrupt - bad padding character (code %d)"),
             ar->name, c);
  }
  varbuf_trunc(&format_member, thisilen);
  if (memchr(format_member.buf, 0, thisilen))
    ohshit(_("file '%.250s' is corrupt - nulls in info section"), ar->name);

  ir->filename = ar->name;

  rip = format_member.buf;
  err = deb_version_parse(&ir->fmtversion,
                          nextline(&rip, ar->name, _("format version number")));
  if (err)
    ohshit(_("file '%.250s' has invalid format version: %s"), ar->name, err);
  if (ir->fmtversion.major != 2)
    ohshit(_("file '%.250s' is format version %d.%d; get a newer dpkg-split"),
           ar->name, ir->fmtversion.major, ir->fmtversion.minor);

  ir->package = nfstrsave(nextline(&rip, ar->name, _("package name")));
  ir->version = nfstrsave(nextline(&rip, ar->name, _("package version number")));
  ir->md5sum = nfstrsave(nextline(&rip, ar->name, _("package file MD5 checksum")));
  if (strlen(ir->md5sum) != MD5HASHLEN ||
      strspn(ir->md5sum, "0123456789abcdef") != MD5HASHLEN)
    ohshit(_("file '%.250s' is corrupt - bad MD5 checksum '%.250s'"),
           ar->name, ir->md5sum);

  ir->orglength = parse_intmax(nextline(&rip, ar->name, _("archive total size")),
                               ar->name, _("archive total size"));
  ir->maxpartlen = parse_intmax(nextline(&rip, ar->name, _("archive part offset")),
                                ar->name, _("archive part offset"));

  partnums = nextline(&rip, ar->name, _("archive part numbers"));
  slash= strchr(partnums,'/');
  if (!slash)
    ohshit(_("file '%.250s' is corrupt - no slash between archive part numbers"), ar->name);
  *slash++ = '\0';

  templong = parse_intmax(slash, ar->name, _("number of archive parts"));
  if (templong <= 0 || templong > INT_MAX)
    ohshit(_("file '%.250s' is corrupt - bad number of archive parts"), ar->name);
  ir->maxpartn= templong;
  templong = parse_intmax(partnums, ar->name, _("archive parts number"));
  if (templong <= 0 || templong > ir->maxpartn)
    ohshit(_("file '%.250s' is corrupt - bad archive part number"), ar->name);
  ir->thispartn= templong;

  /* If the package was created with dpkg 1.16.1 or later it will include
   * the architecture. */
  if (*rip != '\0')
    ir->arch = nfstrsave(nextline(&rip, ar->name, _("package architecture")));
  else
    ir->arch = NULL;

  rc = fd_read(ar->fd, &arh, sizeof(arh));
  if (rc != sizeof(arh))
    read_fail(rc, ar->name, "reading data part member ar header");

  dpkg_ar_normalize_name(&arh);

  if (dpkg_ar_member_is_illegal(&arh))
    ohshit(_("file '%.250s' is corrupt - bad magic at end of second header"),
           ar->name);
  if (strncmp(arh.ar_name,"data",4))
    ohshit(_("file '%.250s' is corrupt - second member is not data member"),
           ar->name);

  ir->thispartlen = dpkg_ar_member_get_size(ar, &arh);
  ir->thispartoffset= (ir->thispartn-1)*ir->maxpartlen;

  if (ir->maxpartn != (ir->orglength+ir->maxpartlen-1)/ir->maxpartlen)
    ohshit(_("file '%.250s' is corrupt - wrong number of parts for quoted sizes"),
           ar->name);
  if (ir->thispartlen !=
      (ir->thispartn == ir->maxpartn
       ? ir->orglength - ir->thispartoffset : ir->maxpartlen))
    ohshit(_("file '%.250s' is corrupt - size is wrong for quoted part number"),
           ar->name);

  ir->filesize = (strlen(DPKG_AR_MAGIC) +
                  sizeof(arh) + thisilen + (thisilen & 1) +
                  sizeof(arh) + ir->thispartlen + (ir->thispartlen & 1));

  if (S_ISREG(ar->mode)) {
    /* Don't do this check if it's coming from a pipe or something.  It's
     * only an extra sanity check anyway. */
    if (ar->size < ir->filesize)
      ohshit(_("file '%.250s' is corrupt - too short"), ar->name);
  }

  ir->headerlen = strlen(DPKG_AR_MAGIC) +
                  sizeof(arh) + thisilen + (thisilen & 1) + sizeof(arh);

  return ir;
}

void mustgetpartinfo(const char *filename, struct partinfo *ri) {
  struct dpkg_ar *part;

  part = dpkg_ar_open(filename);
  if (!part)
    ohshite(_("cannot open archive part file '%.250s'"), filename);
  if (!read_info(part, ri))
    ohshite(_("file '%.250s' is not an archive part"), filename);
  dpkg_ar_close(part);
}

void print_info(const struct partinfo *pi) {
  printf(_("%s:\n"
         "    Part format version:            %d.%d\n"
         "    Part of package:                %s\n"
         "        ... version:                %s\n"
         "        ... architecture:           %s\n"
         "        ... MD5 checksum:           %s\n"
         "        ... length:                 %jd bytes\n"
         "        ... split every:            %jd bytes\n"
         "    Part number:                    %d/%d\n"
         "    Part length:                    %jd bytes\n"
         "    Part offset:                    %jd bytes\n"
         "    Part file size (used portion):  %jd bytes\n\n"),
         pi->filename,
         pi->fmtversion.major, pi->fmtversion.minor,
         pi->package,
         pi->version,
         pi->arch ? pi->arch : C_("architecture", "<unknown>"),
         pi->md5sum,
         (intmax_t)pi->orglength,
         (intmax_t)pi->maxpartlen,
         pi->thispartn,
         pi->maxpartn,
         (intmax_t)pi->thispartlen,
         (intmax_t)pi->thispartoffset,
         (intmax_t)pi->filesize);
}

int
do_info(const char *const *argv)
{
  const char *thisarg;

  if (!*argv)
    badusage(_("--%s requires one or more part file arguments"),
             cipaction->olong);

  while ((thisarg= *argv++)) {
    struct partinfo *pi, ps;
    struct dpkg_ar *part;

    part = dpkg_ar_open(thisarg);
    if (!part)
      ohshite(_("cannot open archive part file '%.250s'"), thisarg);
    pi = read_info(part, &ps);
    dpkg_ar_close(part);
    if (pi) {
      print_info(pi);
    } else {
      printf(_("file '%s' is not an archive part\n"), thisarg);
    }
    m_output(stdout, _("<standard output>"));
  }

  return 0;
}
