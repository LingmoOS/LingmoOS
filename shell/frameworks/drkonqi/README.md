<!--
    SPDX-License-Identifier: CC0-1.0
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Aleix Pol i Gonzalez <aleixpol@kde.org>
-->

# DrKonqi
## Activating the debug button for DrKonqi
Add into `~/.config/drkonqirc`:
```
[DrKonqi]
ShowDebugButton=true
```

## Integrating with coredumpd
We need to make sure that the `drkonqi-coredump-processor@.service` is enabled so that coredumpd knows to tell us when a crash happens. You can inspect it with:

* Check it's enabled: `systemctl is-enabled drkonqi-coredump-processor@.service`
* You can enable it with `systemctl enable drkonqi-coredump-processor@.service`
* The user socket also needs to be present `systemctl --user is-enabled drkonqi-coredump-launcher.socket`
* And can be enabled with `systemctl --user enable --now drkonqi-coredump-launcher.socket `

To make sure the coredumpd setup is working correctly:

* `cat /proc/sys/kernel/core_pattern` should show systemd-coredump as handler
* `KCRASH_DUMP_ONLY=1 kwrite`
* `killall -SEGV kwrite`
* journalctl should show the following:
  * systemd-coredump@.service run an instance (e.g. systemd-coredump@12-66048-0.service)
  * drkonqi-coredump-processor@.service should run an instance with same suffix as systemd-coredump@
  * drkonqi-coredump-launcher@.service should run an instance with yet the same suffix
* A notification to start drkonqi should appear

To use the notifytruck (catch crashes in all software) add `KDE_COREDUMP_NOTIFY=1` to your /etc/environment. This
enables the NotifyTruck for drkonqi-coredump-launcher@.service and offers access to gdb for all crashes. This is a
feature targeted at developers:

* `gedit`
* `killall -SEGV gedit`
* journalctl should look like above
* A notification to start konsole+gdb should appear (if no notification appears something has gone horribly wrong)

### Workflow, Service, and Enablement overview

* KCrash writes metadata files with context information when an application crashes
* systemd-coredump@.service; from upstream; must be enabled; catches the actual core
* drkonqi-coredump-processor@.service; WantedBy systemd-coredump@.service;
  must be enabled (`systemd-coredump@.service.wants/drkonqi-coredump-processor@.service` symlink must exist in one of
  the `systemd-analyze unit-paths`!); passes along the crash into the relevant user session
* drkonqi-coredump-launcher.socket; user socket; must be enabled per user or in one of the global
  `systemd-analyze unit-paths`; the processor talks to this socket
* drkonqi-coredump-launcher@.service; user service; doesn't need to be enabled; invoked by the socket as needed and
  processes the metadata, created by KCrash in the very beginning, to figure out if and how to invoke drkonqi
* drkonqi-coredump-cleanup.{service,timer}; cleanup tech for long running sessions to avoid cluttering $HOME with
  dangling metadata files
* launcher's main.cpp runs through a set of DumpTruck instances, the first that wants to handle the crash may. By default
  that would be the DrkonqiTruck for crashes in KDE software

### IncludeAll Mode

Add into `~/.config/drkonqirc`:
```
[General]
IncludeAll=true
```

This operation mode primarily targets Lingmo Mobile. It effectively catches and reports **all** coredumpd tracked
crashes. This is notably including non-KDE software for which drkonqi will only offer tracing facilities but no active
bug reporting. The use is that for systems where we are completely in control we'll want to give our users the ability
to differentiate a crash in middleware from one in the application space (or defects caused by the former in the latter).

Please note that this mode requires correctly set up coredumpd support as outlined above.

## Application Not Responding (ANR)

DrKonqi supports detection of ANR scenarios with some help from the outside. Specifically a metadata file must be created
in `~/.cache/drkonqi/application-not-responding/` followed by an ABRT signal to the unresponsive application.

The metadata file must be named `$exe.$bootId.$pid.$timestamp.json`. It is currently empty but may gain optional content
in the future.

In a regular Lingmo session this metadata file is created by KWin when an application is being killed for unresponsiveness.

This feature is only supported with the coredumpd backend.
Other backends record the ABRT as crash but will not be able to identify it as ANR.
