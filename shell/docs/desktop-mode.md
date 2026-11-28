# Desktop modes

Settings → Desktop & Dock → Desktop mode selects Normal or Efficient without
logging out. `org.lingmo.Dock` at `/Dock` exposes `setEfficientMode(b)` and the
read-only `efficientMode` property, with PropertiesChanged notifications.
`Desktop/Mode` in shell.conf persists 0 (Normal, default) or 1 (Efficient).
The settings client waits for confirmed service state, disables the control
while unavailable/pending, and refreshes after a service restart.
For this release, startup begins in Normal mode without overwriting the saved
selection; Efficient mode remains available to activate at runtime.

Efficient mode applies a bottom taskbar on the primary logical output:

- One full-width taskbar surface owns the unified glass background and rounded
  52px capsule. The launcher stays at the left; application icons are centred
  on the taskbar, with fixed 32px base icons that shrink when space is limited.
- The existing tray, custom status items, volume/network/battery, control
  center, notification button and two-line clock/date occupy the right area.
- The status controls are hosted by the taskbar surface while retaining their
  existing D-Bus, tray and popup behavior. The top bar does not remain as a
  second visible surface; tray registrations and existing app windows remain
  intact.
- Hover enlargement, launch/attention bounce, the separate minimized-window
  area and user-selected auto-hide are disabled while Efficient is active.
- The app-menu registrar releases its D-Bus name so applications restore their
  local menu fallback. Normal mode takes the name again and apps re-register.
  Old asynchronous PID lookups cannot repopulate registrations after a switch.
- Open popups are closed on mode switch; the control center opens above a
  bottom taskbar. Notification Center remains inside the usable work area.
- The top exclusive zone is replaced by a bottom zone; output changes resize
  the taskbar and its status area. Normal Dock preferences remain stored and
  are restored on exit.

The taskbar background, application icons and status controls share one
compositor-owned surface. The status-bar backend continues to own D-Bus
requests, tray registrations and popup behavior, with popup anchors in the
taskbar's coordinates. Normal mode retains the existing separate top bar and
floating Dock. This is an icon-based taskbar, not a copy of all Windows
taskbar features. Existing Dock context menus, pinning and application-window
grouping remain in use.

Tests: Dock settings and D-Bus persistence, registrar release/reacquisition,
repeated layout switching, work-area restoration, output resize, and bottom
control-center popup placement; Settings has a standalone private-bus test at
`tests/desktop-mode`. Full real-session testing should additionally verify
multiple displays, fullscreen windows, tray overflow, live notifications and
third-party applications that do not implement a local menu fallback.

Validation for this change: the `lingmo-aurora` target builds, and
`dock_layout_test`, `statusbar_test`, `statusbar_view_test`, and the dedicated
bottom-control-center popup test pass. `dock_test` reports one default-pins
expectation failure (12/13 pass); the full `controlcenter_test` reports three
other failures, while its bottom-taskbar popup case passes. No local SDK build
or real Lingmo session / multi-monitor acceptance was performed.
