/*
 * dpkg-deb - construction and deconstruction of *.deb archives
 * extract.c - extracting archives
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2014 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/fdio.h>
#include <dpkg/buffer.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/compress.h>
#include <dpkg/ar.h>
#include <dpkg/deb-version.h>
#include <dpkg/options.h>

#include "dpkg-deb.h"

static void
movecontrolfiles(const char *dir, const char *thing)
{
  char *cmd;
  pid_t pid;

  cmd = str_fmt("mv %s/%s/* %s/ && rmdir %s/%s", dir, thing, dir, dir, thing);
  pid = subproc_fork();
  if (pid == 0) {
    command_shell(cmd, _("shell command to move files"));
  }
  subproc_reap(pid, _("shell command to move files"), 0);
  free(cmd);
}

static void DPKG_ATTR_NORET
read_fail(int rc, const char *filename, const char *what)
{
  if (rc >= 0)
    ohshit(_("unexpected end of file in %s in %.255s"),what,filename);
  else
    ohshite(_("error reading %s from file %.255s"), what, filename);
}

static ssize_t
read_line(int fd, char *buf, size_t min_size, size_t max_size)
{
  ssize_t line_size = 0;
  size_t n = min_size;

  while (line_size < (ssize_t)max_size) {
    ssize_t nread;
    char *nl;

    nread = fd_read(fd, buf + line_size, n);
    if (nread <= 0)
      return nread;

    nl = memchr(buf + line_size, '\n', nread);
    line_size += nread;

    if (nl != NULL) {
      nl[1] = '\0';
      return line_size;
    }

    n = 1;
  }

  buf[line_size] = '\0';
  return line_size;
}

void
extracthalf(const char *debar, const char *dir,
            enum dpkg_tar_options taroption, int admininfo)
{
  struct dpkg_error err;
  const char *errstr;
  struct dpkg_ar *ar;
  char versionbuf[40];
  struct deb_version version;
  off_t ctrllennum, memberlen = 0;
  ssize_t rc;
  int dummy;
  pid_t c1=0,c2,c3;
  int p1[2], p2[2];
  int p2_out;
  char nlc;
  struct compress_params decompress_params = {
    .type = COMPRESSOR_TYPE_GZIP,
    .threads_max = compress_params.threads_max,
  };

  ar = dpkg_ar_open(debar);

  rc = read_line(ar->fd, versionbuf, strlen(DPKG_AR_MAGIC), sizeof(versionbuf) - 1);
  if (rc <= 0)
    read_fail(rc, debar, _("archive magic version number"));

  if (strcmp(versionbuf, DPKG_AR_MAGIC) == 0) {
    int adminmember = -1;
    bool header_done = false;

    ctrllennum= 0;
    for (;;) {
      struct dpkg_ar_hdr arh;

      rc = fd_read(ar->fd, &arh, sizeof(arh));
      if (rc != sizeof(arh))
        read_fail(rc, debar, _("archive member header"));

      if (dpkg_ar_member_is_illegal(&arh))
        ohshit(_("file '%.250s' is corrupt - bad archive header magic"), debar);

      dpkg_ar_normalize_name(&arh);

      memberlen = dpkg_ar_member_get_size(ar, &arh);
      if (!header_done) {
        char *infobuf;

        if (strncmp(arh.ar_name, DEBMAGIC, sizeof(arh.ar_name)) != 0)
          ohshit(_("file '%.250s' is not a Debian binary archive (try dpkg-split?)"),
                 debar);
        infobuf= m_malloc(memberlen+1);
        rc = fd_read(ar->fd, infobuf, memberlen + (memberlen & 1));
        if (rc != (memberlen + (memberlen & 1)))
          read_fail(rc, debar, _("archive information header member"));
        infobuf[memberlen] = '\0';

        if (strchr(infobuf, '\n') == NULL)
          ohshit(_("archive has no newlines in header"));
        errstr = deb_version_parse(&version, infobuf);
        if (errstr)
          ohshit(_("archive has invalid format version: %s"), errstr);
        if (version.major != 2)
          ohshit(_("archive is format version %d.%d; get a newer dpkg-deb"),
                 version.major, version.minor);

        free(infobuf);

        header_done = true;
      } else if (arh.ar_name[0] == '_') {
        /* Members with ‘_’ are noncritical, and if we don't understand
         * them we skip them. */
        if (fd_skip(ar->fd, memberlen + (memberlen & 1), &err) < 0)
          ohshit(_("cannot skip archive member from '%s': %s"), ar->name, err.str);
      } else {
        if (strncmp(arh.ar_name, ADMINMEMBER, strlen(ADMINMEMBER)) == 0) {
          const char *extension = arh.ar_name + strlen(ADMINMEMBER);

	  adminmember = 1;
          decompress_params.type = compressor_find_by_extension(extension);
          if (decompress_params.type != COMPRESSOR_TYPE_NONE &&
              decompress_params.type != COMPRESSOR_TYPE_GZIP &&
              decompress_params.type != COMPRESSOR_TYPE_ZSTD &&
              decompress_params.type != COMPRESSOR_TYPE_XZ)
            ohshit(_("archive '%s' uses unknown compression for member '%.*s', "
                     "giving up"),
                   debar, (int)sizeof(arh.ar_name), arh.ar_name);

          if (ctrllennum != 0)
            ohshit(_("archive '%.250s' contains two control members, giving up"),
                   debar);
          ctrllennum = memberlen;
        } else {
          if (adminmember != 1)
            ohshit(_("archive '%s' has premature member '%.*s' before '%s', "
                     "giving up"),
                   debar, (int)sizeof(arh.ar_name), arh.ar_name, ADMINMEMBER);

	  if (strncmp(arh.ar_name, DATAMEMBER, strlen(DATAMEMBER)) == 0) {
	    const char *extension = arh.ar_name + strlen(DATAMEMBER);

	    adminmember= 0;
            decompress_params.type = compressor_find_by_extension(extension);
            if (decompress_params.type == COMPRESSOR_TYPE_UNKNOWN)
              ohshit(_("archive '%s' uses unknown compression for member '%.*s', "
                       "giving up"),
                     debar, (int)sizeof(arh.ar_name), arh.ar_name);
          } else {
            ohshit(_("archive '%s' has premature member '%.*s' before '%s', "
                     "giving up"),
                   debar, (int)sizeof(arh.ar_name), arh.ar_name, DATAMEMBER);
          }
        }
        if (!adminmember != !admininfo) {
          if (fd_skip(ar->fd, memberlen + (memberlen & 1), &err) < 0)
            ohshit(_("cannot skip archive member from '%s': %s"), ar->name, err.str);
        } else {
          /* Yes! - found it. */
          break;
        }
      }
    }

    if (admininfo >= 2) {
      printf(_(" new Debian package, version %d.%d.\n"
               " size %jd bytes: control archive=%jd bytes.\n"),
             version.major, version.minor,
             (intmax_t)ar->size, (intmax_t)ctrllennum);
      m_output(stdout, _("<standard output>"));
    }
  } else if (strncmp(versionbuf, "0.93", 4) == 0) {
    char ctrllenbuf[40];
    int l;

    l = strlen(versionbuf);

    if (strchr(versionbuf, '\n') == NULL)
      ohshit(_("archive has no newlines in header"));
    errstr = deb_version_parse(&version, versionbuf);
    if (errstr)
      ohshit(_("archive has invalid format version: %s"), errstr);

    rc = read_line(ar->fd, ctrllenbuf, 1, sizeof(ctrllenbuf) - 1);
    if (rc <= 0)
      read_fail(rc, debar, _("archive control member size"));
    if (sscanf(ctrllenbuf, "%jd%c%d", (intmax_t *)&ctrllennum, &nlc, &dummy) != 2 ||
        nlc != '\n')
      ohshit(_("archive has malformed control member size '%s'"), ctrllenbuf);

    if (admininfo) {
      memberlen = ctrllennum;
    } else {
      memberlen = ar->size - ctrllennum - strlen(ctrllenbuf) - l;
      if (fd_skip(ar->fd, ctrllennum, &err) < 0)
        ohshit(_("cannot skip archive control member from '%s': %s"), ar->name,
               err.str);
    }

    if (admininfo >= 2) {
      printf(_(" old Debian package, version %d.%d.\n"
               " size %jd bytes: control archive=%jd, main archive=%jd.\n"),
             version.major, version.minor,
             (intmax_t)ar->size, (intmax_t)ctrllennum,
             (intmax_t)(ar->size - ctrllennum - strlen(ctrllenbuf) - l));
      m_output(stdout, _("<standard output>"));
    }
  } else {
    if (strncmp(versionbuf, "!<arch>", 7) == 0) {
      notice(_("file looks like it might be an archive which has been\n"
               " corrupted by being downloaded in ASCII mode"));
    }

    ohshit(_("'%.255s' is not a Debian format archive"), debar);
  }

  m_pipe(p1);
  c1 = subproc_fork();
  if (!c1) {
    close(p1[0]);
    if (fd_fd_copy(ar->fd, p1[1], memberlen, &err) < 0)
      ohshit(_("cannot copy archive member from '%s' to decompressor pipe: %s"),
             ar->name, err.str);
    if (close(p1[1]))
      ohshite(_("cannot close decompressor pipe"));
    exit(0);
  }
  close(p1[1]);

  if (taroption) {
    m_pipe(p2);
    p2_out = p2[1];
  } else {
    p2_out = 1;
  }

  c2 = subproc_fork();
  if (!c2) {
    if (taroption)
      close(p2[0]);
    decompress_filter(&decompress_params, p1[0], p2_out,
                      _("decompressing archive '%s' (size=%jd) member '%s'"),
                      ar->name, (intmax_t)ar->size,
                      admininfo ? ADMINMEMBER : DATAMEMBER);
    dpkg_ar_close(ar);
    exit(0);
  }
  close(p1[0]);
  dpkg_ar_close(ar);

  if (taroption) {
    close(p2[1]);

    c3 = subproc_fork();
    if (!c3) {
      struct command cmd;

      command_init(&cmd, TAR, "tar");
      command_add_arg(&cmd, "tar");

      if ((taroption & DPKG_TAR_LIST) && (taroption & DPKG_TAR_EXTRACT))
        command_add_arg(&cmd, "-xv");
      else if (taroption & DPKG_TAR_EXTRACT)
        command_add_arg(&cmd, "-x");
      else if (taroption & DPKG_TAR_LIST)
        command_add_arg(&cmd, "-tv");
      else
        internerr("unknown or missing tar action '%d'", taroption);

      if (taroption & DPKG_TAR_PERMS)
        command_add_arg(&cmd, "-p");
      if (taroption & DPKG_TAR_NOMTIME)
        command_add_arg(&cmd, "-m");

      command_add_arg(&cmd, "-f");
      command_add_arg(&cmd, "-");
      command_add_arg(&cmd, "--warning=no-timestamp");

      m_dup2(p2[0],0);
      close(p2[0]);

      unsetenv("TAR_OPTIONS");

      if (dir) {
        if (mkdir(dir, 0777) != 0) {
          if (errno != EEXIST)
            ohshite(_("failed to create directory"));

          if (taroption & DPKG_TAR_CREATE_DIR)
            ohshite(_("unexpected pre-existing pathname %s"), dir);
        }
        if (chdir(dir) != 0)
          ohshite(_("failed to chdir to directory"));
      }

      command_exec(&cmd);
    }
    close(p2[0]);
    subproc_reap(c3, "tar", 0);
  }

  subproc_reap(c2, _("<decompress>"), SUBPROC_NOPIPE);
  if (c1 >= 0)
    subproc_reap(c1, _("paste"), 0);
  if (version.major == 0 && admininfo) {
    /* Handle the version as a float to preserve the behavior of old code,
     * because even if the format is defined to be padded by 0's that might
     * not have been always true for really ancient versions... */
    while (version.minor && (version.minor % 10) == 0)
      version.minor /= 10;

    if (version.minor ==  931)
      movecontrolfiles(dir, OLDOLDDEBDIR);
    else if (version.minor == 932 || version.minor == 933)
      movecontrolfiles(dir, OLDDEBDIR);
  }
}

int
do_ctrltarfile(const char *const *argv)
{
  const char *debar;

  debar = *argv++;
  if (debar == NULL)
    badusage(_("--%s needs a .deb filename argument"), cipaction->olong);
  if (*argv)
    badusage(_("--%s takes only one argument (.deb filename)"),
             cipaction->olong);

  extracthalf(debar, NULL, DPKG_TAR_PASSTHROUGH, 1);

  return 0;
}

int
do_fsystarfile(const char *const *argv)
{
  const char *debar;

  debar = *argv++;
  if (debar == NULL)
    badusage(_("--%s needs a .deb filename argument"),cipaction->olong);
  if (*argv)
    badusage(_("--%s takes only one argument (.deb filename)"),cipaction->olong);
  extracthalf(debar, NULL, DPKG_TAR_PASSTHROUGH, 0);

  return 0;
}

int
do_control(const char *const *argv)
{
  const char *debar, *dir;

  debar = *argv++;
  if (debar == NULL)
    badusage(_("--%s needs a .deb filename argument"), cipaction->olong);

  dir = *argv++;
  if (dir == NULL)
    dir = EXTRACTCONTROLDIR;
  else if (*argv)
    badusage(_("--%s takes at most two arguments (.deb and directory)"),
             cipaction->olong);

  extracthalf(debar, dir, DPKG_TAR_EXTRACT, 1);

  return 0;
}

int
do_extract(const char *const *argv)
{
  const char *debar, *dir;
  enum dpkg_tar_options options = DPKG_TAR_EXTRACT | DPKG_TAR_PERMS;

  if (opt_verbose)
    options |= DPKG_TAR_LIST;

  debar = *argv++;
  if (debar == NULL)
    badusage(_("--%s needs .deb filename and directory arguments"),
             cipaction->olong);

  dir = *argv++;
  if (dir == NULL)
    badusage(_("--%s needs a target directory.\n"
               "Perhaps you should be using dpkg --install ?"),
             cipaction->olong);
  else if (*argv)
    badusage(_("--%s takes at most two arguments (.deb and directory)"),
             cipaction->olong);

  extracthalf(debar, dir, options, 0);

  return 0;
}

int
do_vextract(const char *const *argv)
{
  /* XXX: Backward compatibility. */
  opt_verbose = 1;
  return do_extract(argv);
}

int
do_raw_extract(const char *const *argv)
{
  enum dpkg_tar_options data_options;
  const char *debar, *dir;
  char *control_dir;

  debar = *argv++;
  if (debar == NULL)
    badusage(_("--%s needs .deb filename and directory arguments"),
             cipaction->olong);
  else if (strcmp(debar, "-") == 0)
    badusage(_("--%s does not support (yet) reading the .deb from standard input"),
             cipaction->olong);

  dir = *argv++;
  if (dir == NULL)
    badusage(_("--%s needs a target directory.\n"
               "Perhaps you should be using dpkg --install ?"),
             cipaction->olong);
  else if (*argv)
    badusage(_("--%s takes at most two arguments (.deb and directory)"),
             cipaction->olong);

  control_dir = str_fmt("%s/%s", dir, EXTRACTCONTROLDIR);

  data_options = DPKG_TAR_EXTRACT | DPKG_TAR_PERMS;
  if (opt_verbose)
    data_options |= DPKG_TAR_LIST;

  extracthalf(debar, dir, data_options, 0);
  extracthalf(debar, control_dir, DPKG_TAR_EXTRACT | DPKG_TAR_CREATE_DIR, 1);

  free(control_dir);

  return 0;
}
