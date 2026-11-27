# Desktop modes

Settings → Desktop & Dock → Desktop mode selects Normal or Efficient without
logging out. `org.lingmo.Dock` at `/Dock` exposes `setEfficientMode(b)` and the
read-only `efficientMode` property, with PropertiesChanged notifications.
`Desktop/Mode` in shell.conf persists 0 (Normal, default) or 1 (Efficient).
The settings client waits for confirmed service state, disables the control
while unavailable/pending, and refreshes after a service restart.

Efficient mode applies a bottom taskbar on the primary logical output:

- A Dock application area, full-width across its allocation, with fixed 44px
  base icons and a 64px bar height. Icons shrink when space is limited.
- The launcher sits at the bar's left edge; the application icons after it are
  centred in the room the launcher leaves.
- The bar is a full-width strip flush with the screen edge, drawn on the same
  glass as the floating dock: the panel's blur is the only background, no
  theme color is painted over it, and its corners use a 28px radius.
- As on the ChromeOS shelf, only the corners facing the desktop round: the top
  corners take the 28px radius and the bottom corners stand square against the
  screen edge. The radius masks the glass through a path, and the input region
  follows the same radii.
- The launcher entry is the shelf's start button: a rounded plate with a ring
  inside, drawn in the theme's text color (light on dark, dark on light). This
  drawing replaces the launchpad artwork in efficient mode only; the floating
  dock keeps its artwork, and switching modes swaps it back.
- The existing tray, custom status items, volume/network/battery, control
  center, notification button and two-line clock/date occupy the right area.
- The top bar is moved, not duplicated; tray registrations and existing app
  windows remain intact. Both areas use the surface's glass rather than an
  opaque theme background.
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

## Visual restyle (2026-11-27)

Efficient mode was re-enabled after the temporary disable in 5.0.37 and
restyled after the ChromeOS shelf: 44px icons, a 64px bar, and a 28px radius
on the top corners only, with the bottom corners square against the screen
edge; no opaque theme fill covers the panel's blur. The launcher sits at the
left edge, the application icons are centred after it and stand vertically
centred, and the launcher is drawn as the shelf's plate with a ring inside, in
efficient mode only. The status bar keeps its own
surface; the two windows stand flush on the bottom edge and the glass is what
shows through both. `roundedRegion` and `updateGlass` take separate top and
bottom radii; the glass mask is a path because one radius cannot draw two
different edges. The launcher's drawing is a `DockIcon::LauncherFace` kept
hidden outside efficient mode, so the floating dock's SVG artwork is untouched.

Validation: `dock_test` 13/14 (the default-pin test fails as it did before the
change), `statusbar_view_test` and `statusbar_test` pass. `controlcenter_test`
reports the same three failures it had before the change. Headless captures
show the 64px strip, the launcher at the left edge, the centred application
icons and the launcher's ring. Glass, blur and the real popup placement show
only under a real compositor; no live session, GPU or multi-monitor
acceptance was performed.
