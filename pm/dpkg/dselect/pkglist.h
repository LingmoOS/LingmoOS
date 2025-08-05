/*
 * dselect - Debian package maintenance user interface
 * pkglist.h - external definitions for package list handling
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2007-2014 Guillem Jover <guillem@debian.org>
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

#ifndef PKGLIST_H
#define PKGLIST_H

#include <regex.h>

enum showpriority {
  dp_none,      // has not been involved in any unsatisfied things
  dp_may,       // has been involved in an unsatisfied Suggests
  dp_should,	// has been involved in an unsatisfied Recommends
  dp_must,      // has been involved in an unsatisfied Depends/Conflicts
};

enum selpriority {
  // where did the currently suggested value come from, and how important
  //  is it to display this package ?
  // low
  sp_inherit,     // inherited from our parent list
  sp_selecting,   // propagating a selection
  sp_deselecting, // propagating a deselection
  sp_fixed,       // it came from the ‘status’ file and we're not a recursive list
  // high
};

enum ssavailval {        // Availability sorting order, first to last:
  ssa_broken,            //   Brokenly-installed and nothing available
  ssa_notinst_unseen,    //   Entirely new packages (available but not deselected yet)
  ssa_installed_newer,   //   Installed, newer version available
  ssa_installed_gone,    //   Installed but no longer available
  ssa_installed_sameold, //   Same or older version available as installed
  ssa_notinst_seen,      //   Available but not installed
  ssa_notinst_gone,      //   Not available, and only config files left
  ssa_none = -1,
};

enum ssstateval {      // State sorting order, first to last:
  sss_broken,          //   In some way brokenly installed
  sss_installed,       //   Installed
  sss_configfiles,     //   Config files only
  sss_notinstalled,    //   Not installed
  sss_none = -1,
};

struct perpackagestate {
  struct pkginfo *pkg;
  /* The ‘heading’ entries in the list, for “all packages of type foo”,
   * point to a made-up pkginfo, which has pkg->name==0.
   * pkg->priority and pkg->section are set to the values if appropriate, or to
   * PKG_PRIO_UNSET resp. null if the heading refers to all priorities resp.
   * sections.
   * uprec is used when constructing the list initially and when tearing it
   * down and should not otherwise be used; other fields are undefined.
   */
  pkgwant original;                  // set by caller
  pkgwant direct;                    // set by caller
  pkgwant suggested;                 // set by caller, modified by resolvesuggest
  pkgwant selected;                  // not set by caller, will be set by packagelist
  selpriority spriority;             // monotonically increases (used by sublists)
  showpriority dpriority;            // monotonically increases (used by sublists)
  struct perpackagestate *uprec;     // 0 if this is not part of a recursive list
  ssavailval ssavail;
  ssstateval ssstate;
  varbuf relations;

  void free(bool recursive);
};

class packagelist : public baselist {
protected:
  column col_status;
  column col_section;
  column col_priority;
  column col_package;
  column col_archinstalled;
  column col_archavailable;
  column col_versioninstalled;
  column col_versionavailable;
  column col_description;

  // Only used when ‘verbose’ is set
  column col_status_hold;
  column col_status_status;
  column col_status_old_want;
  column col_status_new_want;

  // Table of packages
  struct perpackagestate *datatable;
  struct perpackagestate **table;

  // Misc.
  int nallocated;
  bool recursive, verbose;
  enum {
    so_unsorted,
    so_section,
    so_priority,
    so_alpha,
  } sortorder;
  enum {
    sso_unsorted,
    sso_avail,
    sso_state,
  } statsortorder;
  enum {
    ado_none,
    ado_available,
    ado_both,
  } archdisplayopt;
  enum {
    vdo_none,
    vdo_available,
    vdo_both,
  } versiondisplayopt;
  bool calcssadone, calcsssdone;
  struct perpackagestate *headings;

  // Package searching flags
  bool searchdescr;
  regex_t searchfsm;

  // Information displays
  struct infotype {
    bool (packagelist::*relevant)(); // null means always relevant
    void (packagelist::*display)(); // null means end of table
  };
  const infotype *currentinfo;
  static const infotype infoinfos[];
  static const infotype *const baseinfo;
  bool itr_recursive();
  bool itr_nonrecursive();
  void severalinfoblurb();
  void itd_mainwelcome();
  void itd_explaindisplay();
  void itd_recurwelcome();
  void itd_relations();
  void itd_description();
  void itd_statuscontrol();
  void itd_availablecontrol();

  // Dependency and sublist processing
  struct doneent { doneent *next; void *dep; } *depsdone, *unavdone;
  bool alreadydone(doneent **, void *);
  int resolvedepcon(dependency*);
  int checkdependers(pkginfo*, int changemade); // returns new changemade
  int deselect_one_of(pkginfo *er, pkginfo *ed, dependency *dep);

  // Define these virtuals
  bool checksearch(varbuf &str) override;
  bool matchsearch(int index) override;
  void redraw1itemsel(int index, int selected) override;
  void redrawcolheads() override;
  void redrawthisstate() override;
  void redrawinfo() override;
  void redrawtitle() override;
  void setwidths() override;
  const char *itemname(int index) override;
  const struct helpmenuentry *helpmenulist() override;

  // Miscellaneous internal routines

  void redraw1package(int index, int selected);
  int compareentries(const struct perpackagestate *a, const struct perpackagestate *b);
  friend int qsort_compareentries(const void *a, const void *b);
  pkgwant reallywant(pkgwant, struct perpackagestate *);
  int describemany(varbuf &vb, const char *prioritystring, const char *section,
                   const struct perpackagestate *pps);
  bool deppossatisfied(deppossi *possi, perpackagestate **fixbyupgrade);

  void sortmakeheads();
  void resortredisplay();
  void movecursorafter(int ncursor);
  void initialsetup();
  void finalsetup();
  void ensurestatsortinfo();

  // To do with building the list, with heading lines in it
  void discardheadings();
  void addheading(enum ssavailval, enum ssstateval,
                  pkgpriority, const char *, const char *section);
  void sortinplace();
  bool affectedmatches(struct pkginfo *pkg, struct pkginfo *comparewith);
  void affectedrange(int *startp, int *endp);
  void setwant(pkgwant nw);
  void sethold(int hold);

 public:

  // Keybinding functions */
  void kd_quit_noop();
  void kd_revert_abort();
  void kd_revertsuggest();
  void kd_revertdirect();
  void kd_revertinstalled();
  void kd_morespecific();
  void kd_lessspecific();
  void kd_swaporder();
  void kd_swapstatorder();
  void kd_select();
  void kd_deselect();
  void kd_purge();
  void kd_hold();
  void kd_unhold();
  void kd_info();
  void kd_toggleinfo();
  void kd_verbose();
  void kd_archdisplay();
  void kd_versiondisplay();

  explicit packagelist(keybindings *kb); // nonrecursive
  packagelist(keybindings *kb, pkginfo **pkgltab); // recursive
  void add(pkginfo **array) { while (*array) add(*array++); }
  void add(pkginfo*);
  void add(pkginfo *, pkgwant);
  void add(pkginfo*, const char *extrainfo, showpriority displayimportance);
  bool add(dependency *, showpriority displayimportance);
  void addunavailable(deppossi*);
  bool useavailable(pkginfo *);
  pkgbin *find_pkgbin(pkginfo *);

  int resolvesuggest();
  int deletelessimp_anyleft(showpriority than);
  pkginfo **display();
  ~packagelist() override;
};

void repeatedlydisplay(packagelist *sub, showpriority,
                       packagelist *unredisplay = nullptr);
int would_like_to_install(pkgwant, pkginfo *pkg);

extern const char *const wantstrings[];
extern const char *const eflagstrings[];
extern const char *const statusstrings[];
extern const char *const prioritystrings[];
extern const char *const priorityabbrevs[];
extern const char *const relatestrings[];
extern const char *const ssastrings[], *const ssaabbrevs[];
extern const char *const sssstrings[], *const sssabbrevs[];
extern const char statuschars[];
extern const char eflagchars[];
extern const char wantchars[];

#endif /* PKGLIST_H */
