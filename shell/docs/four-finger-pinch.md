# Four-finger pinch shortcuts

Four-finger libinput pinch gestures are claimed by the desktop gesture filter
before they can reach an application. Pinch inward opens the existing full-screen
Launcher; pinch outward performs the existing Show Desktop window slide. Both
map libinput's absolute gesture scale directly to progress, so direction changes
move the same animation backward/forward without a snap. A short settling
animation completes gestures released past 50%, or reverses them below that
threshold. A libinput-cancelled gesture always reverses. If the launcher is already open, spreading fades it out while windows move
aside; cancellation restores it. New pinch gestures are
ignored while a window overview is active or a drag owns input.

The launcher gesture reveals its existing launcher window while the opacity
and slight zoom follow the pinch. Its interactive views stay disabled until
settling completes. Show Desktop uses its existing per-window presentation
offsets and keyboard handoff. Ordinary three-finger swipe desktop switching
and overview gestures remain unchanged. Pinches with other finger counts,
applications' own gestures, and all non-touchpad input retain their normal path.

The scale thresholds are: ignore within 0.04 of 1; progress reaches its endpoint
at 0.55 when pinching inward or 1.55 when pinching outward. These thresholds
are in libinput's relative pinch scale and do not depend on pointer acceleration.

Tests cover scale-to-progress mapping, reversal, cancellation, one-finger-count
filtering, drag rejection, live launcher view transform/opacity and show-desktop
presentation. These are compositor/headless tests; verify the feel on physical
touchpads of different resolutions and at multiple display scales before
release. This feature has no per-gesture Settings toggle in this change.
