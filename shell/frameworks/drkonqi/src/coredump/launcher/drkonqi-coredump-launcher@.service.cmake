# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

[Unit]
Description=Launch DrKonqi for a systemd-coredump crash
PartOf=graphical-session.target

[Service]
# Don't need to be working anywhere specific, use tmp.
WorkingDirectory=%T
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR@/drkonqi-coredump-launcher
Slice=app.slice
Restart=no
