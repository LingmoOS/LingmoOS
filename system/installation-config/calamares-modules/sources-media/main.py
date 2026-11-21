# SPDX-License-Identifier: GPL-3.0-or-later
import runpy
import libcalamares


def run():
    target = libcalamares.globalstorage.value("rootMountPoint")
    if not target: return ("Missing installation root", "Target system was not mounted")
    try:
        helper = runpy.run_path("/usr/share/calamares/helpers/calamares-sources-media")
        helper["configure"](target)
    except Exception as error:
        return ("Installation source configuration failed", str(error))
    return None
