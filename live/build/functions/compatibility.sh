#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.

# We've added the option to also include files before packages renamed the old directory.
# This function auto detects which version should be used.
select_includes_chroot(){
    local OLD_DIR="includes.chroot"
    local NEW_DIR="includes.chroot_after_packages"

    if Find_files "config/${NEW_DIR}/" && Find_files "config/${OLD_DIR}/"
    then
        Echo_error "You have files in ${OLD_DIR} and ${NEW_DIR}. Only one directory is allowed."
        exit 1
    fi

    if Find_files "config/${NEW_DIR}/"
    then
        echo -n "${NEW_DIR}"
    elif Find_files "config/${OLD_DIR}/"
    then
        echo -n "${OLD_DIR}"
    fi
}
