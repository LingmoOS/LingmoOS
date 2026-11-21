# Shake pointer to locate

Settings → Mouse → Shake pointer to locate enables the effect (default on).
The compositor exposes `shakeToLocate` and `setShakeToLocate(b)` on
`org.lingmo.Mouse1`, and persists `Mouse/shakeToLocate` in compositor.conf.
Cursor preferences follow that file, so changing the switch takes effect
without changing pointer speed, base size or theme. The control is separate
from mouse-device-only settings, so it is usable with a touchpad too.

Detection uses logical pointer coordinates and monotonic time. Four opposing
excursions, each at least 24 logical pixels, within 650 ms and with sufficient
travel trigger the locator. A long gap, a >300px jump, or invalid coordinates
reset detection. Small vibration and a fast one-way sweep do not trigger it.
Button-held dragging and touch-hidden/held cursors do not start it.

The cursor grows to 2.5 times its base size over 100 ms, holds until 900 ms
after the last trigger, then returns over 300 ms. Repeated shaking extends the
hold. A 16ms timer runs only during the effect. Textures are reused at varying
draw sizes; animation does not reload cursor themes. Hardware cursor planes
are temporarily hidden and the cursor is composited instead. At normal size,
the existing plane path is restored. Capture bounds and capture drawing use
the same scale. The pointer location and input hotspot are never changed.

Standard cursor shapes retain their artwork. A client-owned bitmap cursor is
temporarily replaced visually by the system pointer for locating; the client
surface is preserved and restored afterward. An explicit hidden cursor is
never forced visible. This first implementation does not resample arbitrary
client cursor surfaces. Large display-coordinate jumps are ignored, though
small programmatic pointer warps cannot be distinguished from device motion
at this observer layer.

Validation:
- Standalone detector/animation tests: direction, event rate, jitter, slow
  motion, warps, repeated gestures, smooth growth/decay and disable reset.
- Cursor tests: hotspot invariance, size, hardware return, dragging, touch,
  disable, custom client restoration and explicit hide.
- Mouse service and Settings UI tests: property/method, persistence, switch
  state and service restart. Tests use private D-Bus and temporary preferences.
- Full compositor build. Live DPI/multi-monitor movement and perceived gesture
  sensitivity still need desktop acceptance; no running session was replaced.
