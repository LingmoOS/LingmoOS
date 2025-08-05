/*
 * dselect - Debian package maintenance user interface
 * methparse.cc - access method list parsing
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2011, 2013-2015 Guillem Jover <guillem@debian.org>
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/file.h>

#include "dselect.h"
#include "bindings.h"
#include "method.h"

int noptions=0;
struct dselect_option *options = nullptr, *coption = nullptr;
struct method *methods = nullptr;

static void DPKG_ATTR_NORET
badmethod(varbuf &pathname, const char *why)
{
  ohshit(_("syntax error in method options file '%.250s' -- %s"),
         pathname.str(), why);
}

static void DPKG_ATTR_NORET
eofmethod(varbuf &pathname, FILE *f, const char *why)
{
  if (ferror(f))
    ohshite(_("error reading options file '%.250s'"), pathname.str());
  badmethod(pathname,why);
}

void readmethods(const char *pathbase, dselect_option **optionspp, int *nread) {
  static const char *const methodprograms[]= {
    METHODSETUPSCRIPT,
    METHODUPDATESCRIPT,
    METHODINSTALLSCRIPT,
    nullptr
  };
  const char *const *ccpp;
  int methodlen;
  DIR *dir;
  FILE *names;
  struct dirent *dent;
  struct varbuf vb;
  method *meth;
  dselect_option *opt;

  varbuf path;
  path += pathbase;
  path += "/";

  dir = opendir(path.str());
  if (!dir) {
    if (errno == ENOENT)
      return;
    ohshite(_("unable to read '%.250s' directory for reading methods"),
            path.str());
  }

  debug(dbg_general, "readmethods('%s',...) directory open", pathbase);

  while ((dent = readdir(dir)) != nullptr) {
    int c = dent->d_name[0];
    debug(dbg_general, "readmethods('%s',...) considering '%s' ...",
          pathbase, dent->d_name);
    if (c != '_' && !c_isalpha(c))
      continue;
    char *p = dent->d_name + 1;
    while ((c = *p) != 0 && c_isalnum(c) && c != '_')
      p++;
    if (c) continue;
    methodlen= strlen(dent->d_name);
    if (methodlen > IMETHODMAXLEN)
      ohshit(_("method '%.250s' has name that is too long (%d > %d characters)"),
             dent->d_name, methodlen, IMETHODMAXLEN);
    /* Check if there is a localized version of this method */

    path.reset();
    path += pathbase;
    path += "/";
    path += dent->d_name;
    path += "/";

    varbuf pathmeth;

    for (ccpp= methodprograms; *ccpp; ccpp++) {
      pathmeth = path;
      pathmeth += *ccpp;
      if (access(pathmeth.str(), R_OK | X_OK))
        ohshite(_("unable to access method script '%.250s'"), pathmeth.str());
    }
    debug(dbg_general, " readmethods('%s',...) scripts ok", pathbase);

    varbuf pathopts;
    pathopts = path;
    pathopts += METHODOPTIONSFILE;
    names = fopen(pathopts.str(), "r");
    if (!names)
      ohshite(_("unable to read method options file '%.250s'"), pathopts.str());

    meth= new method;
    meth->name = varbuf(dent->d_name);
    meth->path = path;
    meth->next= methods;
    meth->prev = nullptr;
    if (methods)
      methods->prev = meth;
    methods= meth;
    debug(dbg_general, " readmethods('%s',...) new method"
                       " name='%s' path='%s'",
          pathbase, meth->name.str(), meth->path.str());

    while ((c= fgetc(names)) != EOF) {
      if (c_isspace(c))
        continue;
      opt= new dselect_option;
      opt->meth= meth;
      vb.reset();
      do {
        if (!c_isdigit(c))
          badmethod(pathopts, _("non-digit where digit wanted"));
        vb += c;
        c= fgetc(names);
        if (c == EOF)
          eofmethod(pathopts, names, _("end of file in index string"));
      } while (!c_isspace(c));
      if (vb.len() > OPTIONINDEXMAXLEN)
        badmethod(pathopts, _("index string too long"));
      opt->index = vb;
      do {
        if (c == '\n')
          badmethod(pathopts, _("newline before option name start"));
        c= fgetc(names);
        if (c == EOF)
          eofmethod(pathopts, names, _("end of file before option name start"));
      } while (c_isspace(c));
      vb.reset();
      if (!c_isalpha(c) && c != '_')
        badmethod(pathopts, _("nonalpha where option name start wanted"));
      do {
        if (!c_isalnum(c) && c != '_')
          badmethod(pathopts, _("non-alphanum in option name"));
        vb += c;
        c= fgetc(names);
        if (c == EOF)
          eofmethod(pathopts, names, _("end of file in option name"));
      } while (!c_isspace(c));
      opt->name = vb;
      do {
        if (c == '\n')
          badmethod(pathopts, _("newline before summary"));
        c= fgetc(names);
        if (c == EOF)
          eofmethod(pathopts, names, _("end of file before summary"));
      } while (c_isspace(c));
      vb.reset();
      do {
        vb += c;
        c= fgetc(names);
        if (c == EOF)
          eofmethod(pathopts, names, _("end of file in summary - missing newline"));
      } while (c != '\n');
      opt->summary = vb;

      varbuf pathoptsdesc;
      pathoptsdesc += pathopts;
      pathoptsdesc += OPTIONSDESCPFX;
      pathoptsdesc += opt->name;

      dpkg_error err = DPKG_ERROR_INIT;

      if (file_slurp(pathoptsdesc.str(), &opt->description, &err) < 0) {
        if (err.syserrno != ENOENT)
          dpkg_error_print(&err, _("cannot load option description file '%s'"),
                                 pathoptsdesc.str());
      }

      debug(dbg_general,
            " readmethods('%s',...) new option index='%s' name='%s'"
            " summary='%.20s' len(description=%s)=%zu method name='%s'"
            " path='%s'",
            pathbase,
            opt->index.str(), opt->name.str(), opt->summary.str(),
            opt->description.len() ? "'...'" : "<none>",
            opt->description.len() ? opt->description.len() : -1,
            opt->meth->name.str(), opt->meth->path.str());

      dselect_option **optinsert = optionspp;
      while (*optinsert && strcmp(opt->index.str(), (*optinsert)->index.str()) > 0)
        optinsert = &(*optinsert)->next;
      opt->next= *optinsert;
      *optinsert= opt;
      (*nread)++;
    }
    if (ferror(names))
      ohshite(_("error during read of method options file '%.250s'"), pathopts.str());
    fclose(names);
  }
  closedir(dir);
  debug(dbg_general, "readmethods('%s',...) done", pathbase);
}

static char *methoptfile = nullptr;

void getcurrentopt() {
  char methoptbuf[IMETHODMAXLEN+1+IOPTIONMAXLEN+2];
  FILE *cmo;
  int l;
  char *p;

  if (methoptfile == nullptr)
    methoptfile = dpkg_db_get_path(CMETHOPTFILE);

  coption = nullptr;
  cmo= fopen(methoptfile,"r");
  if (!cmo) {
    if (errno == ENOENT) return;
    ohshite(_("unable to open current option file '%.250s'"), methoptfile);
  }
  debug(dbg_general, "getcurrentopt() cmethopt open");
  if (!fgets(methoptbuf,sizeof(methoptbuf),cmo)) { fclose(cmo); return; }
  if (fgetc(cmo) != EOF) { fclose(cmo); return; }
  if (!feof(cmo)) { fclose(cmo); return; }
  debug(dbg_general, "getcurrentopt() cmethopt eof");
  fclose(cmo);
  debug(dbg_general, "getcurrentopt() cmethopt read");
  l= strlen(methoptbuf);  if (!l || methoptbuf[l-1] != '\n') return;
  methoptbuf[--l]= 0;
  debug(dbg_general, "getcurrentopt() cmethopt len and newline");
  p= strchr(methoptbuf,' ');  if (!p) return;
  debug(dbg_general, "getcurrentopt() cmethopt space");
  *p++= 0;
  debug(dbg_general, "getcurrentopt() cmethopt meth name '%s'", methoptbuf);
  method *meth = methods;
  while (meth && strcmp(methoptbuf, meth->name.str()))
    meth = meth->next;
  if (!meth) return;
  debug(dbg_general, "getcurrentopt() cmethopt meth found; opt '%s'", p);
  dselect_option *opt = options;
  while (opt && (opt->meth != meth || strcmp(p, opt->name.str())))
    opt = opt->next;
  if (!opt) return;
  debug(dbg_general, "getcurrentopt() cmethopt opt found");
  coption= opt;
}

void writecurrentopt() {
  struct atomic_file *file;

  if (methoptfile == nullptr)
    internerr("method options filename is nullptr");

  file = atomic_file_new(methoptfile, ATOMIC_FILE_NORMAL);
  atomic_file_open(file);
  if (fprintf(file->fp, "%s %s\n", coption->meth->name.str(), coption->name.str()) == EOF)
    ohshite(_("unable to write new option to '%.250s'"), file->name_new);
  atomic_file_close(file);
  atomic_file_commit(file);
  atomic_file_free(file);
}
