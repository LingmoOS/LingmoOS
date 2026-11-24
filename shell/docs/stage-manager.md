# Stage Manager implementation

## Decisions and scope

The implementation follows the independent StageGroup recommendation in
`docs/architecture/STAGE-MANAGER-REVIEW.md` in the workspace and the subsequently
supplied product reference. The earlier design's one-Stage-per-Desk path is not
used: groups live inside a Desk and have one logical output owner. Window
families have exclusive membership. The existing global Desk and Seat activation
remain authoritative. Switching preserves free window geometry and stacking;
there is no automatic primary/auxiliary tiling.

This is an internal Aurora feature, not an external window manager. WM owns
membership and effective visibility; Shell owns the recent-groups strip and
bounded still images. No new Wayland protocol, D-Bus service, SDK API or external
framework is required. A missing thumbnail must not delay switching. A Shell
component teardown is distinct from a fatal Aurora process crash.

Before editing, the compositor worktree contained a modified `debian/rules` and
untracked Debian/build output; these are preserved. Build and tests use a separate
directory. The former research-only restriction is superseded by the request to
start development; the original three design documents and independent review
remain historical inputs rather than being silently rewritten.

## Validation

Implementation and validation results are recorded below as work progresses.
Real Wayland/Xwayland desktop, GPU, touch/drag, lock-screen and multi-monitor
acceptance have not yet been performed. Static review or headless tests do not
prove those behaviours.

Current implementation checks:

- `lingmo-wm` target: passed.
- `lingmo-aurora` target: passed.
- `wm_test`: 152/152 passed, including Stage group independence, geometry
  preservation, membership move/separation, per-Desk selection and existing WM
  regressions.
- `accelerator_test` Stage toggle case: passed. The default toggle is
  `Meta+Shift+S` under the PC convention and `Control+Meta+Shift+S` under the
  Command convention; it is disabled while the session is blocked.
- The Stage strip visibility transition uses the existing SDK frame animation
  and can reverse direction from its current opacity when state changes again.
- Aurora's Stage strip follows the left-side reference layout: the active group
  is omitted from the recent list, background groups show bounded still
  previews with stacked-card depth, an application icon and a title, and the
  strip follows the current work area. It requests previews through the
  existing compositor ScreenGrabber and does not refresh hidden group previews
  continuously.
- Launcher Dock toggles now have a short activation-settling guard so pointer
  movement away from the Dock icon cannot dismiss the newly opened launcher;
  explicit background clicks, Escape and application launch still dismiss it.
- The full `accelerator_test` suite reports 43/49 passing. Six failures are in
  shortcut replacement/conflict assertions, client key delivery, or service
  tests that cannot register because this run has no session bus. The
  Stage-specific case passes.
- Real desktop, GPU, multi-monitor hotplug, touch drag and visual thumbnail
  fidelity: not yet verified.
