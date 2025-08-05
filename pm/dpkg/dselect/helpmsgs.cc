/*
 * dselect - Debian package maintenance user interface
 * helpmsgs.cc - list of help messages
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <dpkg/i18n.h>

#include "helpmsgs.h"

const struct helpmessage hlp_listkeys = {
  N_("Keystrokes"), N_("\
Motion keys: Next/Previous, Top/End, Up/Down, Backwards/Forwards:\n\
  j, Down-arrow         k, Up-arrow             move highlight\n\
  N, Page-down, Space   P, Page-up, Backspace   scroll list by 1 page\n\
  ^n                    ^p                      scroll list by 1 line\n\
  t, Home               e, End                  jump to top/end of list\n\
  u                     d                       scroll info by 1 page\n\
  ^u                    ^d                      scroll info by 1 line\n\
  B, Left-arrow         F, Right-arrow          pan display by 1/3 screen\n\
  ^b                    ^f                      pan display by 1 character\n\n\
\
Mark packages for later processing:\n\
 +, Insert  install or upgrade      =, H  hold in present version\n\
 -, Delete  remove                  :, G  unhold: upgrade or leave uninstalled\n\
 _          remove & purge config\n\
                                             Miscellaneous:\n\
Quit, exit, overwrite (note capitals!):       ?, F1 request help (also Help)\n\
 Return  Confirm, quit (check dependencies)   i, I  toggle/cycle info displays\n\
   Q     Confirm, quit (override dep.s)       o, O  cycle through sort options\n\
 X, Esc  eXit, abandoning any changes made   v, A, V  change status display opts\n\
   R     Revert to state before this list      ^l   redraw display\n\
   U     set all to sUggested state             /   search (Return to cancel)\n\
   D     set all to Directly requested state  n, \\  repeat last search\n")
};

const struct helpmessage hlp_mainintro = {
  N_("Introduction to package selections"), N_("\
Welcome to dselect's main package listing.\n\n\
\
You will be presented with a list of packages which are installed or available\n\
for installation.  You can navigate around the list using the cursor keys,\n\
mark packages for installation (using '+') or deinstallation (using '-').\n\
Packages can be marked either singly or in groups; initially you will see that\n\
the line 'All packages' is selected.  '+', '-' and so on will affect all the\n\
packages described by the highlighted line.\n\n\
\
Some of your choices will cause conflicts or dependency problems; you will be\n\
given a sub-list of the relevant packages, so that you can solve the problems.\n\n\
\
You should read the list of keys and the explanations of the display.\n\
Much on-line help is available, please make use of it - press '?' at\n\
any time for help.\n\n\
\
When you have finished selecting packages, press <enter> to confirm changes,\n\
or 'X' to quit without saving changes. A final check on conflicts and\n\
dependencies will be done - here too you may see a sublist.\n\n\
\
Press <space> to leave help and enter the list now.\n")
};

const struct helpmessage hlp_readonlyintro = {
  N_("Introduction to read-only package list browser"), N_("\
Welcome to dselect's main package listing.\n\n\
\
You will be presented with a list of packages which are installed or available\n\
for installation.  Since you do not have the privilege necessary to update\n\
package states, you are in a read-only mode.  You can navigate around the\n\
list using the cursor keys (please see the 'Keystrokes' help screen), observe\n\
the status of the packages and read information about them.\n\n\
\
You should read the list of keys and the explanations of the display.\n\
Much on-line help is available, please make use of it - press '?' at\n\
any time for help.\n\n\
\
When you have finished browsing, press 'Q' or <enter> to quit.\n\n\
\
Press <space> to leave help and enter the list now.\n")
};

const struct helpmessage hlp_recurintro = {
  N_("Introduction to conflict/dependency resolution sub-list"), N_("\
Dependency/conflict resolution - introduction.\n\n\
\
One or more of your choices have raised a conflict or dependency problem -\n\
some packages should only be installed in conjunction with certain others, and\n\
some combinations of packages may not be installed together.\n\n\
\
You will see a sub-list containing the packages involved.  The bottom half of\n\
the display shows relevant conflicts and dependencies; use 'i' to cycle between\n\
that, the package descriptions and the internal control information.\n\n\
\
A set of 'suggested' packages has been calculated, and the initial markings in\n\
this sub-list have been set to match those, so you can just hit Return to\n\
accept the suggestions if you wish.  You may abort the change(s) which caused\n\
the problem(s), and go back to the main list, by pressing capital 'X'.\n\n\
\
You can also move around the list and change the markings so that they are more\n\
like what you want, and you can 'reject' my suggestions by using the capital\n\
'D' or 'R' keys (see the keybindings help screen).  You can use capital 'Q' to\n\
force to accept the situation currently displayed, in case you want to\n\
override a recommendation or think that the program is mistaken.\n\n\
\
Press <space> to leave help and enter the sub-list; remember: press '?' for help.\n")
};

const struct helpmessage hlp_displayexplain1 = {
  N_("Display, part 1: package listing and status chars"), N_("\
The top half of the screen shows a list of packages.  For each package you see\n\
four columns for its current status on the system and mark.  In terse mode (use\n\
'v' to toggle verbose display) these are single characters, from left to right:\n\n\
\
 Error flag: Space - no error (but package may be in broken state - see below)\n\
              'R'  - serious error during installation, needs reinstallation;\n\
 Installed state:     Space    - not installed;\n\
                       '*'     - installed;\n\
                       '-'     - not installed but config files remain;\n\
   packages in these { 'U'     - unpacked but not yet configured;\n\
   states are not    { 'C'     - half-configured (an error happened);\n\
   (quite) properly  { 'I'     - half-installed (an error happened);\n\
   installed         { 'W','t' - triggers are awaited resp. pending.\n\
 Old mark: what was requested for this package before presenting this list;\n\
 Mark: what is requested for this package:\n\
  '*': marked for installation or upgrade;\n\
  '-': marked for removal, but any configuration files will remain;\n\
  '=': on hold: package will not be installed, upgraded or removed;\n\
  '_': marked for purge completely - even remove configuration;\n\
  'n': package is new and has yet to be marked for install/remove/&c.\n\n\
\
Also displayed are each package's Priority, Section, name, installed and\n\
available version numbers (shift-V to display/hide) and summary description.\n")
};

const struct helpmessage hlp_displayexplain2 = {
  N_("Display, part 2: list highlight; information display"), N_("\
* Highlight: One line in the package list will be highlighted.  It indicates\n\
  which package(s) will be affected by presses of '+', '-' and '_'.\n\n\
\
* The dividing line in the middle of the screen shows a brief explanation of\n\
  the status of the currently-highlighted package, or a description of which\n\
  group is highlighted if a group line is.  If you don't understand the\n\
  meaning of some of the status characters displayed, go to the relevant\n\
  package and look at this divider line, or use the 'v' key for a verbose\n\
  display (press 'v' again to go back to the terse display).\n\n\
\
* The bottom of the screen shows more information about the\n\
  currently-highlighted package (if there is only one).\n\n\
\
  It can show an extended description of the package, the internal package\n\
  control details (either for the installed or available version of the\n\
  package), or information about conflicts and dependencies involving the\n\
  current package (in conflict/dependency resolution sublists).\n\n\
\
  Use the 'i' key to cycle through the displays, and 'I' to hide the\n\
  information display or expand it to use almost all of the screen.\n")
};

const struct helpmessage hlp_methintro = {
  N_("Introduction to method selection display"), N_("\
dselect and dpkg can do automatic installation, loading the package files to be\n\
installed from one of a number of different possible places.\n\n\
\
This list allows you to select one of these installation methods.\n\n\
\
Move the highlight to the method you wish to use, and hit Enter.  You will then\n\
be prompted for the information required to do the installation.\n\n\
\
As you move the highlight a description of each method, where available, is\n\
displayed in the bottom half of the screen.\n\n\
\
If you wish to quit without changing anything use the 'x' key while in the list\n\
of installation methods.\n\n\
\
A full list of keystrokes is available by pressing 'k' now, or from the help\n\
menu reachable by pressing '?'.\n")
};

const struct helpmessage hlp_methkeys = {
  N_("Keystrokes for method selection"), N_("\
Motion keys: Next/Previous, Top/End, Up/Down, Backwards/Forwards:\n\
  j, Down-arrow         k, Up-arrow             move highlight\n\
  N, Page-down, Space   P, Page-up, Backspace   scroll list by 1 page\n\
  ^n                    ^p                      scroll list by 1 line\n\
  t, Home               e, End                  jump to top/end of list\n\
  u                     d                       scroll info by 1 page\n\
  ^u                    ^d                      scroll info by 1 line\n\
  B, Left-arrow         F, Right-arrow          pan display by 1/3 screen\n\
  ^b                    ^f                      pan display by 1 character\n\
(These are the same motion keys as in the package list display.)\n\n\
\
Quit:\n\
 Return, Enter    select this method and go to its configuration dialogue\n\
 x, X             exit without changing or setting up the installation method\n\n\
\
Miscellaneous:\n\
  ?, Help, F1      request help\n\
 ^l                redraw display\n\
  /                search (just return to cancel)\n\
  \\                repeat last search\n")
};
