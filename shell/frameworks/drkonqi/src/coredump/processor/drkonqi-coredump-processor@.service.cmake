# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

[Unit]
Description=Pass systemd-coredump journal entries to relevant user for potential DrKonqi handling

[Service]
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR@/drkonqi-coredump-processor --boot-id %b --instance %i
# If it doesn't manage to find its relevant coredump in a short while it never will.
# This job is instantiated when systemd-coredump@ starts. They run roughly at the
# same time. Any slow down will at most occur on dumping in of itself.
# This limit is already incredibly lenient.
RuntimeMaxSec=5 minutes
Nice=10
OOMScoreAdjust=500
# Mountain of confinement options. We kind of need to run as root to write to all the
# the various sockets, but we don't need a whole lot of capabilities.
# Largely based on systemd-coredump@'s own configuration.
NoNewPrivileges=yes
CapabilityBoundingSet=~CAP_SYS_PTRACE CAP_SYS_ADMIN CAP_NET_ADMIN CAP_KILL CAP_SYS_CHROOT CAP_BLOCK_SUSPEND CAP_LINUX_IMMUTABLE
SystemCallFilter=@system-service
SystemCallFilter=~@privileged @resources
ProtectClock=yes
IPAddressDeny=any
LockPersonality=yes
MemoryDenyWriteExecute=yes
PrivateDevices=yes
PrivateNetwork=yes
PrivateTmp=yes
ProtectControlGroups=yes
ProtectHostname=yes
ProtectKernelModules=yes
ProtectKernelTunables=yes
ProtectKernelLogs=yes
ProtectSystem=strict
RestrictAddressFamilies=AF_UNIX
RestrictNamespaces=yes
RestrictRealtime=yes
RestrictSUIDSGID=yes
SystemCallArchitectures=native
SystemCallErrorNumber=EPERM

[Install]
# NOTE: This service must be enabled (symlinked) manually - systemctl enable will not do!
# https://github.com/systemd/systemd/issues/19437
WantedBy=systemd-coredump@.service
