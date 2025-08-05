/*
 * libdpkg - Debian packaging suite library routines
 * fields.c - parsing of all the different fields, when reading in
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
 * Copyright © 2009 Canonical Ltd.
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
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

#include <string.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>
#include <dpkg/string.h>
#include <dpkg/path.h>
#include <dpkg/parsedump.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/triglib.h>

/**
 * Flags to parse a name associated to a value.
 */
enum parse_nv_mode {
  /** Expect no more words (default). */
  PARSE_NV_LAST		= 0,
  /** Expect another word after the parsed name. */
  PARSE_NV_NEXT		= 1,
  /** Do not fail if there is no name with an associated value found. */
  PARSE_NV_FALLBACK	= 2,
};

/**
 * Parses a name and returns its associated value.
 *
 * Gets a pointer to the string to parse in @a strp, and modifies the pointer
 * to the string to point to the end of the parsed text. If no value is found
 * for the name and @a flags is set to #PARSE_NV_FALLBACK then @a strp is set
 * to NULL and returns -1, otherwise a parse error is emitted.
 */
static int
parse_nv(struct parsedb_state *ps, enum parse_nv_mode parse_mode,
         const char **strp, const struct namevalue *nv_head)
{
  const char *str_start = *strp, *str_end;
  const struct namevalue *nv;
  int value;

  dpkg_error_destroy(&ps->err);

  if (str_start[0] == '\0')
    return dpkg_put_error(&ps->err, _("is missing a value"));

  nv = namevalue_find_by_name(nv_head, str_start);
  if (nv == NULL) {
    /* We got no match, skip further string validation. */
    if (parse_mode != PARSE_NV_FALLBACK)
      return dpkg_put_error(&ps->err, _("has invalid value '%.50s'"), str_start);

    str_end = NULL;
    value = -1;
  } else {
    str_end = str_start + nv->length;
    while (c_isspace(str_end[0]))
      str_end++;
    value = nv->value;

    if (parse_mode != PARSE_NV_NEXT && str_is_set(str_end))
      return dpkg_put_error(&ps->err, _("has trailing junk"));
  }

  *strp = str_end;

  return value;
}

void
f_name(struct pkginfo *pkg, struct pkgbin *pkgbin,
       struct parsedb_state *ps,
       const char *value, const struct fieldinfo *fip)
{
  const char *e;

  e = pkg_name_is_illegal(value);
  if (e != NULL)
    parse_error(ps, _("invalid package name in '%s' field: %s"), fip->name, e);
  /* We use the new name, as pkg_hash_find_set() may have done a tolower for us. */
  pkg->set->name = pkg_hash_find_set(value)->name;
}

void
f_archives(struct pkginfo *pkg, struct pkgbin *pkgbin,
           struct parsedb_state *ps,
           const char *value, const struct fieldinfo *fip)
{
  struct archivedetails *fdp, **fdpp;
  char *cpos, *space;
  int allowextend;

  if (!*value)
    parse_error(ps, _("empty archive details '%s' field"), fip->name);
  if (!(ps->flags & pdb_recordavailable))
    parse_error(ps,
                _("archive details '%s' field not allowed in status file"),
               fip->name);
  allowextend = !pkg->archives;
  fdpp = &pkg->archives;
  cpos= nfstrsave(value);
  while (*cpos) {
    space = cpos;
    while (*space && !c_isspace(*space))
      space++;
    if (*space)
      *space++ = '\0';
    fdp= *fdpp;
    if (!fdp) {
      if (!allowextend)
        parse_error(ps,
                    _("too many values in archive details '%s' field "
                      "(compared to others)"), fip->name);
      fdp = nfmalloc(sizeof(*fdp));
      fdp->next= NULL;
      fdp->name= fdp->msdosname= fdp->size= fdp->md5sum= NULL;
      *fdpp= fdp;
    }
    STRUCTFIELD(fdp, fip->integer, const char *) = cpos;
    fdpp= &fdp->next;
    while (*space && c_isspace(*space))
      space++;
    cpos= space;
  }
  if (*fdpp)
    parse_error(ps,
                _("too few values in archive details '%s' field "
                  "(compared to others)"), fip->name);
}

void
f_charfield(struct pkginfo *pkg, struct pkgbin *pkgbin,
            struct parsedb_state *ps,
            const char *value, const struct fieldinfo *fip)
{
  if (*value)
    STRUCTFIELD(pkgbin, fip->integer, char *) = nfstrsave(value);
}

void
f_boolean(struct pkginfo *pkg, struct pkgbin *pkgbin,
          struct parsedb_state *ps,
          const char *value, const struct fieldinfo *fip)
{
  bool boolean;

  if (!*value)
    return;

  boolean = parse_nv(ps, PARSE_NV_LAST, &value, booleaninfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("boolean (yes/no) '%s' field: %s"),
                fip->name, ps->err.str);

  STRUCTFIELD(pkgbin, fip->integer, bool) = boolean;
}

void
f_multiarch(struct pkginfo *pkg, struct pkgbin *pkgbin,
            struct parsedb_state *ps,
            const char *value, const struct fieldinfo *fip)
{
  int multiarch;

  if (!*value)
    return;

  multiarch = parse_nv(ps, PARSE_NV_LAST, &value, multiarchinfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("quadstate (foreign/allowed/same/no) '%s' field: %s"),
                fip->name, ps->err.str);
  STRUCTFIELD(pkgbin, fip->integer, int) = multiarch;
}

void
f_architecture(struct pkginfo *pkg, struct pkgbin *pkgbin,
               struct parsedb_state *ps,
               const char *value, const struct fieldinfo *fip)
{
  pkgbin->arch = dpkg_arch_find(value);
  if (pkgbin->arch->type == DPKG_ARCH_ILLEGAL)
    parse_warn(ps, _("'%s' is not a valid architecture name in '%s' field: %s"),
               value, fip->name, dpkg_arch_name_is_illegal(value));
}

void
f_section(struct pkginfo *pkg, struct pkgbin *pkgbin,
          struct parsedb_state *ps,
          const char *value, const struct fieldinfo *fip)
{
  if (!*value) return;
  pkg->section = nfstrsave(value);
}

void
f_priority(struct pkginfo *pkg, struct pkgbin *pkgbin,
           struct parsedb_state *ps,
           const char *value, const struct fieldinfo *fip)
{
  const char *str = value;
  int priority;

  if (!*value) return;

  priority = parse_nv(ps, PARSE_NV_FALLBACK, &str, priorityinfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("word in '%s' field: %s"), fip->name, ps->err.str);

  if (str == NULL) {
    pkg->priority = PKG_PRIO_OTHER;
    pkg->otherpriority = nfstrsave(value);
  } else {
    pkg->priority = priority;
  }
}

void
f_obs_class(struct pkginfo *pkg, struct pkgbin *pkgbin,
        struct parsedb_state *ps,
        const char *value, const struct fieldinfo *fip)
{
  parse_warn(ps, _("obsolete '%s' field used"), fip->name);
  f_priority(pkg, pkgbin, ps, value, fip);
}

void
f_status(struct pkginfo *pkg, struct pkgbin *pkgbin,
         struct parsedb_state *ps,
         const char *value, const struct fieldinfo *fip)
{
  if (ps->flags & pdb_rejectstatus)
    parse_error(ps,
                _("value for '%s' field not allowed in this context"),
                fip->name);
  if (ps->flags & pdb_recordavailable)
    return;

  pkg->want = parse_nv(ps, PARSE_NV_NEXT, &value, wantinfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("first (want) word in '%s' field: %s"),
                fip->name, ps->err.str);
  pkg->eflag = parse_nv(ps, PARSE_NV_NEXT, &value, eflaginfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("second (error) word in '%s' field: %s"),
                fip->name, ps->err.str);
  pkg->status = parse_nv(ps, PARSE_NV_LAST, &value, statusinfos);
  if (dpkg_has_error(&ps->err))
    parse_error(ps, _("third (status) word in '%s' field: %s"),
                fip->name, ps->err.str);
}

void
f_version(struct pkginfo *pkg, struct pkgbin *pkgbin,
          struct parsedb_state *ps,
          const char *value, const struct fieldinfo *fip)
{
  if (parse_db_version(ps, &pkgbin->version, value) < 0)
    parse_problem(ps, _("'%s' field value '%.250s'"), fip->name, value);
}

void
f_obs_revision(struct pkginfo *pkg, struct pkgbin *pkgbin,
               struct parsedb_state *ps,
               const char *value, const struct fieldinfo *fip)
{
  char *newversion;

  parse_warn(ps, _("obsolete '%s' field used"), fip->name);
  if (!*value) return;
  if (str_is_set(pkgbin->version.revision)) {
    newversion = nfmalloc(strlen(pkgbin->version.version) +
                          strlen(pkgbin->version.revision) + 2);
    sprintf(newversion, "%s-%s", pkgbin->version.version,
                                 pkgbin->version.revision);
    pkgbin->version.version = newversion;
  }
  pkgbin->version.revision = nfstrsave(value);
}

void
f_configversion(struct pkginfo *pkg, struct pkgbin *pkgbin,
                struct parsedb_state *ps,
                const char *value, const struct fieldinfo *fip)
{
  if (ps->flags & pdb_rejectstatus)
    parse_error(ps,
                _("value for '%s' field not allowed in this context"),
                fip->name);
  if (ps->flags & pdb_recordavailable)
    return;

  if (parse_db_version(ps, &pkg->configversion, value) < 0)
    parse_problem(ps, _("'%s' field value '%.250s'"), fip->name, value);
}

/*
 * The code in f_conffiles ensures that value[-1] == ' ', which is helpful.
 */
static void conffvalue_lastword(const char *value, const char *from,
                                const char *endent,
                                const char **word_start_r, int *word_len_r,
                                const char **new_from_r,
                                struct parsedb_state *ps)
{
  const char *lastspc;

  if (from <= value+1) goto malformed;
  for (lastspc= from-1; *lastspc != ' '; lastspc--);
  if (lastspc <= value+1 || lastspc >= endent-1) goto malformed;

  *new_from_r= lastspc;
  *word_start_r= lastspc + 1;
  *word_len_r= (int)(from - *word_start_r);
  return;

malformed:
  parse_error(ps,
              _("value for '%s' field has malformed line '%.*s'"),
              "Conffiles", (int)min(endent - value, 250), value);
}

void
f_conffiles(struct pkginfo *pkg, struct pkgbin *pkgbin,
            struct parsedb_state *ps,
            const char *value, const struct fieldinfo *fip)
{
  static const char obsolete_str[]= "obsolete";
  static const char remove_on_upgrade_str[] = "remove-on-upgrade";
  struct conffile **lastp;

  lastp = &pkgbin->conffiles;
  while (*value) {
    struct conffile *newlink;
    const char *endent, *endfn, *hashstart;
    char *newptr;
    int c, namelen, hashlen;
    int flags = CONFFILE_NONE;

    c= *value++;
    if (c == '\n') continue;
    if (c != ' ')
      parse_error(ps,
                  _("value for '%s' field has line starting with non-space '%c'"),
                  fip->name, c);
    for (endent = value; (c = *endent) != '\0' && c != '\n'; endent++) ;
    conffvalue_lastword(value, endent, endent,
			&hashstart, &hashlen, &endfn,
                        ps);
    if (hashlen == sizeof(remove_on_upgrade_str) - 1 &&
        memcmp(hashstart, remove_on_upgrade_str, hashlen) == 0) {
      flags |= CONFFILE_REMOVE_ON_UPGRADE;
      conffvalue_lastword(value, endfn, endent, &hashstart, &hashlen, &endfn,
                          ps);
    }

    if (hashlen == sizeof(obsolete_str) - 1 &&
        memcmp(hashstart, obsolete_str, hashlen) == 0) {
      flags |= CONFFILE_OBSOLETE;
      conffvalue_lastword(value, endfn, endent,
			  &hashstart, &hashlen, &endfn,
			  ps);
    }

    newlink = nfmalloc(sizeof(*newlink));
    value = path_skip_slash_dotslash(value);
    namelen= (int)(endfn-value);
    if (namelen <= 0)
      parse_error(ps,
                  _("root or empty directory listed as a conffile in '%s' field"),
                  fip->name);
    newptr = nfmalloc(namelen+2);
    newptr[0]= '/';
    memcpy(newptr+1,value,namelen);
    newptr[namelen+1] = '\0';
    newlink->name= newptr;
    newptr= nfmalloc(hashlen+1);
    memcpy(newptr, hashstart, hashlen);
    newptr[hashlen] = '\0';
    newlink->hash= newptr;
    newlink->flags = flags;
    newlink->next =NULL;
    *lastp= newlink;
    lastp= &newlink->next;
    value= endent;
  }
}

void
f_dependency(struct pkginfo *pkg, struct pkgbin *pkgbin,
             struct parsedb_state *ps,
             const char *value, const struct fieldinfo *fip)
{
  const char *p, *emsg;
  static struct varbuf depname, version;

  struct dependency **ldypp;

  /* Empty fields are ignored. */
  if (!*value)
    return;
  p= value;

  ldypp = &pkgbin->depends;
  while (*ldypp)
    ldypp = &(*ldypp)->next;

   /* Loop creating new struct dependency's. */
  for (;;) {
    struct deppossi **ldopp;
    struct dependency *dyp;

    dyp = nfmalloc(sizeof(*dyp));
    /* Set this to NULL for now, as we don't know what our real
     * struct pkginfo address (in the database) is going to be yet. */
    dyp->up = NULL;
    dyp->next= NULL; *ldypp= dyp; ldypp= &dyp->next;
    dyp->list= NULL; ldopp= &dyp->list;
    dyp->type= fip->integer;

    /* Loop creating new struct deppossi's. */
    for (;;) {
      const char *depnamestart;
      int depnamelength;
      struct deppossi *dop;

      depnamestart= p;
      /* Skip over package name characters. */
      while (*p && !c_isspace(*p) && *p != ':' && *p != '(' && *p != ',' &&
             *p != '|')
        p++;
      depnamelength= p - depnamestart ;
      if (depnamelength == 0)
        parse_error(ps,
                    _("'%s' field, missing package name, or garbage where "
                      "package name expected"), fip->name);

      varbuf_set_buf(&depname, depnamestart, depnamelength);

      emsg = pkg_name_is_illegal(depname.buf);
      if (emsg)
        parse_error(ps,
                    _("'%s' field, invalid package name '%.255s': %s"),
                    fip->name, depname.buf, emsg);
      dop = nfmalloc(sizeof(*dop));
      dop->up= dyp;
      dop->ed = pkg_hash_find_set(depname.buf);
      dop->next= NULL; *ldopp= dop; ldopp= &dop->next;

      /* Don't link this (which is after all only ‘new_pkg’ from
       * the main parsing loop in parsedb) into the depended on
       * packages' lists yet. This will be done later when we
       * install this (in parse.c). For the moment we do the
       * ‘forward’ links in deppossi (‘ed’) only, and the ‘backward’
       * links from the depended on packages to dop are left undone. */
      dop->rev_next = NULL;
      dop->rev_prev = NULL;

      dop->cyclebreak = false;

      /* See if we have an architecture qualifier. */
      if (*p == ':') {
        static struct varbuf arch;
        const char *archstart;
        int archlength;

        archstart = ++p;
        while (*p && !c_isspace(*p) && *p != '(' && *p != ',' && *p != '|')
          p++;
        archlength = p - archstart;
        if (archlength == 0)
          parse_error(ps, _("'%s' field, missing architecture name, or garbage "
                            "where architecture name expected"), fip->name);

        varbuf_set_buf(&arch, archstart, archlength);

        dop->arch_is_implicit = false;
        dop->arch = dpkg_arch_find(arch.buf);

        if (dop->arch->type == DPKG_ARCH_ILLEGAL)
          emsg = dpkg_arch_name_is_illegal(arch.buf);
        if (emsg)
          parse_error(ps, _("'%s' field, reference to '%.255s': "
                            "invalid architecture name '%.255s': %s"),
                      fip->name, depname.buf, arch.buf, emsg);
      } else if (fip->integer == dep_conflicts || fip->integer == dep_breaks ||
                 fip->integer == dep_replaces) {
        /* Conflicts/Breaks/Replaces get an implicit "any" arch qualifier. */
        dop->arch_is_implicit = true;
        dop->arch = dpkg_arch_get(DPKG_ARCH_WILDCARD);
      } else {
        /* Otherwise use the pkgbin architecture, which will be assigned to
         * later on by parse.c, once we can guarantee we have parsed it from
         * the control stanza. */
        dop->arch_is_implicit = true;
        dop->arch = NULL;
      }

      /* Skip whitespace after package name. */
      while (c_isspace(*p))
        p++;

      /* See if we have a versioned relation. */
      if (*p == '(') {
        char c1;
        const char *versionstart;
        int versionlength;

        p++;
        while (c_isspace(*p))
          p++;
        c1= *p;
        if (c1 == '<' || c1 == '>') {
          char c2;

          c2= *++p;
          dop->verrel = (c1 == '<') ? DPKG_RELATION_LT : DPKG_RELATION_GT;
          if (c2 == '=') {
            dop->verrel |= DPKG_RELATION_EQ;
            p++;
          } else if (c2 == c1) {
            /* Either ‘<<’ or ‘>>’. */
            p++;
          } else if (c2 == '<' || c2 == '>') {
            parse_error(ps,
                        _("'%s' field, reference to '%.255s':\n"
                          " bad version relationship %c%c"),
                        fip->name, depname.buf, c1, c2);
          } else {
            parse_warn(ps,
                       _("'%s' field, reference to '%.255s':\n"
                         " '%c' is obsolete, use '%c=' or '%c%c' instead"),
                       fip->name, depname.buf, c1, c1, c1, c1);
            dop->verrel |= DPKG_RELATION_EQ;
          }
        } else if (c1 == '=') {
          dop->verrel = DPKG_RELATION_EQ;
          p++;
        } else {
          parse_warn(ps,
                     _("'%s' field, reference to '%.255s':\n"
                       " implicit exact match on version number, "
                       "suggest using '=' instead"),
                     fip->name, depname.buf);
          dop->verrel = DPKG_RELATION_EQ;
        }
        if ((dop->verrel != DPKG_RELATION_EQ) && (fip->integer == dep_provides))
          parse_lax_problem(ps, pdb_lax_stanza_parser,
                            _("only exact versions may be used for '%s' field"),
                            fip->name);

        if (!c_isspace(*p) && !c_isalnum(*p)) {
          parse_warn(ps,
                     _("'%s' field, reference to '%.255s':\n"
                       " version value starts with non-alphanumeric, "
                       "suggest adding a space"),
                     fip->name, depname.buf);
        }
        /* Skip spaces between the relation and the version. */
        while (c_isspace(*p))
          p++;

	versionstart= p;
        while (*p && *p != ')' && *p != '(') {
          if (c_isspace(*p))
            break;
          p++;
        }
	versionlength= p - versionstart;
        while (c_isspace(*p))
          p++;
        if (*p == '\0')
          parse_error(ps,
                      _("'%s' field, reference to '%.255s': "
                        "version unterminated"), fip->name, depname.buf);
        else if (*p != ')')
          parse_error(ps,
                      _("'%s' field, reference to '%.255s': "
                        "version contains '%c' instead of '%c'"),
                      fip->name, depname.buf, *p, ')');
        varbuf_set_buf(&version, versionstart, versionlength);
        if (parse_db_version(ps, &dop->version, version.buf) < 0)
          parse_problem(ps,
                        _("'%s' field, reference to '%.255s': version '%s'"),
                        fip->name, depname.buf, version.buf);
        p++;
        while (c_isspace(*p))
          p++;
      } else {
        dop->verrel = DPKG_RELATION_NONE;
        dpkg_version_blank(&dop->version);
      }
      if (!*p || *p == ',') break;
      if (*p != '|')
        parse_error(ps,
                    _("'%s' field, syntax error after reference to package '%.255s'"),
                    fip->name, dop->ed->name);
      if (fip->integer == dep_conflicts ||
          fip->integer == dep_breaks ||
          fip->integer == dep_provides ||
          fip->integer == dep_replaces)
        parse_error(ps, _("alternatives ('|') not allowed in '%s' field"),
                    fip->name);
      p++;
      while (c_isspace(*p))
        p++;
    }
    if (!*p) break;
    p++;
    while (c_isspace(*p))
      p++;
  }
}

void
f_obs_dependency(struct pkginfo *pkg, struct pkgbin *pkgbin,
                 struct parsedb_state *ps,
                 const char *value, const struct fieldinfo *fip)
{
  parse_warn(ps, _("obsolete '%s' field used"), fip->name);
  f_dependency(pkg, pkgbin, ps, value, fip);
}

static const char *
scan_word(const char **valp)
{
  static struct varbuf word;
  const char *p, *start, *end;

  p = *valp;
  for (;;) {
    if (!*p) {
      *valp = p;
      return NULL;
    }
    if (c_iswhite(*p)) {
      p++;
      continue;
    }
    start = p;
    break;
  }
  for (;;) {
    if (*p && !c_iswhite(*p)) {
      p++;
      continue;
    }
    end = p;
    break;
  }

  varbuf_set_buf(&word, start, end - start);

  *valp = p;

  return word.buf;
}

void
f_trigpend(struct pkginfo *pend, struct pkgbin *pkgbin,
           struct parsedb_state *ps,
           const char *value, const struct fieldinfo *fip)
{
  const char *word;

  if (ps->flags & pdb_rejectstatus)
    parse_error(ps,
                _("value for '%s' field not allowed in this context"),
                fip->name);

  while ((word = scan_word(&value))) {
    const char *emsg;

    emsg = trig_name_is_illegal(word);
    if (emsg)
      parse_error(ps,
                  _("illegal pending trigger name '%.255s': %s"), word, emsg);

    if (!trig_note_pend_core(pend, nfstrsave(word)))
      parse_error(ps,
                  _("duplicate pending trigger '%.255s'"), word);
  }
}

void
f_trigaw(struct pkginfo *aw, struct pkgbin *pkgbin,
         struct parsedb_state *ps,
         const char *value, const struct fieldinfo *fip)
{
  const char *word;

  if (ps->flags & pdb_rejectstatus)
    parse_error(ps,
                _("value for '%s' field not allowed in this context"),
                fip->name);

  while ((word = scan_word(&value))) {
    struct pkginfo *pend;
    struct dpkg_error err;

    pend = pkg_spec_parse_pkg(word, &err);
    if (pend == NULL)
      parse_error(ps,
                  _("illegal package name in awaited trigger '%.255s': %s"),
                  word, err.str);

    if (!trig_note_aw(pend, aw))
      parse_error(ps,
                  _("duplicate awaited trigger package '%.255s'"), word);

    trig_awaited_pend_enqueue(pend);
  }
}
