/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2002 Michael v.Ostheim <ostheimm@users.berlios.de>
 */

#include <stdlib.h>
#include <unistd.h>

#include <vector>

#include "xf86configpath.h"

using namespace std;

XF86ConfigPath::XF86ConfigPath()
{
    vector<string> searchPaths;
    searchPaths.push_back("/etc/X11/XF86Config-4");
    searchPaths.push_back("/etc/X11/XF86Config");
    searchPaths.push_back("/etc/XF86Config");
    searchPaths.push_back("/usr/X11R6/etc/X11/XF86Config-4");
    searchPaths.push_back("/usr/X11R6/etc/X11/XF86Config");
    searchPaths.push_back("/usr/X11R6/lib/X11/XF86Config-4");
    searchPaths.push_back("/usr/X11R6/lib/X11/XF86Config");

    searchPaths.push_back("/etc/X11/xorg.conf-4");
    searchPaths.push_back("/etc/X11/xorg.conf");
    searchPaths.push_back("/etc/xorg.conf");
    searchPaths.push_back("/usr/X11R6/etc/X11/xorg.conf-4");
    searchPaths.push_back("/usr/X11R6/etc/X11/xorg.conf");
    searchPaths.push_back("/usr/X11R6/lib/X11/xorg.conf-4");
    searchPaths.push_back("/usr/X11R6/lib/X11/xorg.conf");

    vector<string>::iterator it = searchPaths.begin();
    for (; it != searchPaths.end(); ++it) {
        if (!access((Path = *it).c_str(), F_OK)) {
            break;
        }
    }
}

XF86ConfigPath::~XF86ConfigPath()
{
}

/** Returns path */
const char *XF86ConfigPath::get()
{
    return (Path.c_str());
}
