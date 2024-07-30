#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>

SCRIPTFILE=/etc/pico-wizard/scripts.d/01-copy-cec-config

if [[ -f "$SCRIPTFILE" ]]; then
    echo "$SCRIPTFILE exist skipping"
else
    mkdir -p /etc/pico-wizard/scripts.d/
    printf "#! /bin/bash\n\ncp /tmp/plasma-remotecontrollersrc /etc/xdg/plasma-remotecontrollersrc \n" >> /etc/pico-wizard/scripts.d/01-copy-cec-config
    chmod 755 /etc/pico-wizard/scripts.d/01-copy-cec-config
fi
