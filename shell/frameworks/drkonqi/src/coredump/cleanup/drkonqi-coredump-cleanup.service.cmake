# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

[Unit]
Description=Cleanup lingering KCrash metadata
ConditionPathExistsGlob=%C/kcrash-metadata/*.ini
PartOf=graphical-session.target
After=lingmo-core.target

[Service]
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR@/drkonqi-coredump-cleanup %C/kcrash-metadata/
RuntimeMaxSec=30 minutes

[Install]
# This is a bit of a stop-gap. Ideally we should have a different service run on login to
# pick up lingering crashes that are relevant (e.g. crash on last logout) and only what is
# left over should then get cleaned up (if the file is old enough).
# Since we currently lack the UI infrastructure for that we had better clean up on login
# unconditionally.
WantedBy=default.target
