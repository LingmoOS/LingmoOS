# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

[Unit]
Description=Consume pending crashes using DrKonqi
PartOf=graphical-session.target
Requires=drkonqi-coredump-launcher.socket
After=lingmo-core.target
After=drkonqi-coredump-launcher.socket

[Service]
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR@/drkonqi-coredump-processor --settle-first --pickup --uid %U
RuntimeMaxSec=30 minutes

[Install]
WantedBy=lingmo-core.target
