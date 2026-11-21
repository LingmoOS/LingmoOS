# Desktop modes

Settings → Desktop & Dock → Desktop mode selects Normal or Efficient without
logging out. `org.lingmo.Dock` at `/Dock` exposes `setEfficientMode(b)` and the
read-only `efficientMode` property, with PropertiesChanged notifications.
`Desktop/Mode` in shell.conf persists 0 (Normal, default) or 1 (Efficient).
The settings client waits for confirmed service state, disables the control
while unavailable/pending, and refreshes after a service restart.

Efficient mode applies a bottom taskbar on the primary logical output:

- A left-aligned Dock application area, full-width across its allocation,
  fixed 32px base icons and 52px bar height. Icons shrink when space is limited.
- The existing tray, custom status items, volume/network/battery, control
  center, notification button and two-line clock/date occupy the right area.
- The top bar is moved, not duplicated; tray registrations and existing app
  windows remain intact. Both areas use the theme's opaque background.
- Hover enlargement, launch/attention bounce, the separate minimized-window
  area and user-selected auto-hide are disabled while Efficient is active.
- The app-menu registrar releases its D-Bus name so applications restore their
  local menu fallback. Normal mode takes the name again and apps re-register.
  Old asynchronous PID lookups cannot repopulate registrations after a switch.
- Open popups are closed on mode switch; the control center opens above a
  bottom taskbar. Notification Center remains inside the usable work area.
- The top exclusive zone is replaced by a bottom zone; output changes update
  both areas. Normal Dock preferences remain stored and are restored on exit.

The two taskbar regions are separate compositor-owned surfaces, coordinated by
one DockSettings mode signal and the right area's width changes. This retains
tray/menu/capture/input ownership while presenting one bottom taskbar. It is an
icon-based taskbar, not a copy of all Windows taskbar features. Existing Dock
context menus, pinning and application-window grouping remain in use.

Tests: Dock settings and D-Bus persistence, registrar release/reacquisition,
repeated layout switching, work-area restoration, output resize, and bottom
control-center popup placement; Settings has a standalone private-bus test at
`tests/desktop-mode`. Full real-session testing should additionally verify
multiple displays, fullscreen windows, tray overflow, live notifications and
third-party applications that do not implement a local menu fallback.

Validation for this change: the full compositor links successfully. The full
Dock backend, status-bar backend and status-bar view suites pass, along with
the new bottom-control-center case and the standalone Settings D-Bus suite.
Taskbar and Settings snapshots were rendered headlessly. The broader existing
shell-components/control-center suites are not fully green: existing tests
still assume the old Dock headroom offset/tooltip fade and expanded control
center height/camera tile. Those failures were observed during verification;
this change does not claim the complete desktop suite passes. No running
session was replaced and no real multi-monitor/fullscreen acceptance was done.
