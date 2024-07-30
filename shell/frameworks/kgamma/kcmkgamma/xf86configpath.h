/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2002 Michael v.Ostheim <ostheimm@users.berlios.de>
 */

#ifndef XF86CONFIGPATH_H
#define XF86CONFIGPATH_H

#include <string>

/**Search for XF86Config or XF86Config-4 which can be located at different
  places.
  *@author Michael v.Ostheim
  */

class XF86ConfigPath
{
public:
    XF86ConfigPath();
    ~XF86ConfigPath();

    /** Returns Path variable */
    const char *get();

private: // Private attributes
    /** Contains the path of XF86Config file */
    std::string Path;
};

#endif
