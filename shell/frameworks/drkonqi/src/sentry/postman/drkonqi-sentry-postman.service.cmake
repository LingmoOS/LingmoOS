# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2021-2023 Harald Sitter <sitter@kde.org>

[Unit]
Description=Submitting pending crash events
PartOf=graphical-session.target
After=lingmo-core.target

[Service]
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR@/drkonqi-sentry-postman
RuntimeMaxSec=30 minutes
Restart=no
