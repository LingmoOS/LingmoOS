<!--
    SPDX-License-Identifier: CC0-1.0
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>
-->

# Exec

Drkonqi also doesn't technically require a process to actually crash, you can
simply run it manually on any old pid. Do note that in most cases you only need
to define the pid, however not giving certain cmdline options can change
behavior somewhat drastically (e.g. kdeinit vs. notkdeinit behaves radically
different as far as process name detection goes)

example command to doctor a running dolphin:

```
DRKONQI_HTTP_DELAY_MS=1000 \
DRKONQI_TEST_MODE=1 \
DRKONQI_IGNORE_QUALITY=1 \
DRKONQI_KDE_BUGZILLA_URL="https://bugstest.kde.org/" \
bin/drkonqi \
-platform xcb -display :0 --appname org_kde_powerdevil --apppath /usr/lib/x86_64-linux-gnu/libexec --signal 11 --pid `pidof dolphin` --startupid 0 --restarted --bugaddress submit@bugs.kde.org --dialog
```

# Environment

The following environment variables may be used to test drkonqi with actual reports.

- DRKONQI_TEST_MODE: influences everything and the kitchen sink, influences
  UI behavior to streamline test-reporting and also forces bugstest.kde.org
  use so test reports do not clutter up the live bugzilla
- DRKONQI_IGNORE_QUALITY: ignores quality constraints (e.g. backtrace scores
  are always assumed perfect)
- DRKONQI_KDE_BUGZILLA_URL: changes the kde bugzilla url (only really useful
  to set this to bugstest.kde.org - must end with slash)
- DRKONQI_HTTP_DELAY_MS: introduce a delay on every single API request
  (simulates slow connection)

# Backends

Drkonqi has a number of backends that may be used. Backends are stacked by
order of preference, backends in the directory of the binary are most preferred.
This means that you can dump debuggers/internal/ into your build's bin/
directory and override for example gdb. This essentially allows you to replace
the gdb debugger with a `cat` of a fixture file to not have to trace live
processes at all.
