# Lingmo Clock

Native Lingmo desktop application for world clocks, alarms, stopwatch laps and parallel countdown timers.

## Build

Requires the Lingmo SDK development package, CMake 3.22+, a C++20 compiler, SQLite and gettext.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The application uses native Lingmo SDK views and the Settings-style unified `SidebarList`/`SidebarItem` contract. It exports a global menu with an in-window fallback when the registrar is absent.

## Alarm service

`lingmo-clockd` stores alarms under `$XDG_DATA_HOME/lingmo-clock/clock.db`, owns the session D-Bus name `org.lingmo.Clock`, and starts in each user session through `default.target` (D-Bus activation is also available). Its absolute `CLOCK_REALTIME` timerfd is armed with `TFD_TIMER_CANCEL_ON_SET`, so wall-clock changes cause a rescan; `timedate1` notifications trigger a rescan after timezone changes. Alarm recurrences are calculated from local civil time, including daylight-saving transitions. The service continues after the UI exits and reloads its schedule from SQLite at login/restart.

The D-Bus interface is documented in `data/org.lingmo.Clock.xml`. It provides `Reschedule`, `ListAlarms`, `SetAlarmEnabled`, and `RemoveAlarm`; the same interface is installed in `/usr/share/dbus-1/interfaces`.

Alarms cannot wake a machine from suspend unless the platform and system configuration provide a wake-capable alarm source. This application does not claim that capability; a pending alarm is delivered when the session resumes. Notification delivery depends on the session notification service.

Stopwatch and active countdowns use `std::chrono::steady_clock`, not wall time. Stopwatch and countdowns stay active while navigating between app sections. Countdown state is currently tied to the app process; closing the window stops those timers. Only alarms are scheduled by the background service.

## Preview and desktop integration

Build runs include `lingmo-clock-capture` as a headless UI preview test. The PNG is written to `build/tests/preview/lingmo-clock-preview.png` (or `build/preview/lingmo-clock-preview.png`, depending on generator). For session verification, run `lingmo-clock`, create a one-minute alarm, close the window, and verify the desktop notification still appears. Also test clock changes, zone changes, DST dates, service restart, light/dark theme, and 100%/150% scaling in a Lingmo Wayland session.

## License

GPL-3.0-or-later.
