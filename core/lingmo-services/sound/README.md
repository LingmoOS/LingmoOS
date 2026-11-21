# System event sounds

`lingmo-services` owns `org.lingmo.Services.Sound` at
`/org/lingmo/Services/Sound` on the session bus, under `org.lingmo.Services`.
It is published before the session starts the desktop. It has no independent
autostart process.

Methods:

- `playEvent(s event) -> b accepted`: request a themed event. True means the
  player started, not that the audio device successfully rendered the sound.
- `setEnabled(b enabled)`: persist the global switch; disabling also stops the
  current event sound.
- `setTheme(s theme)`: select a theme directory name.

Read-only properties `enabled` and `theme` emit PropertiesChanged. Settings
are stored in `[Sound]` in `~/.config/lingmo/services.conf`, as `Enabled=true`
and `Theme=freedesktop` by default. Settings → Sound provides the switch and
a preview button; unavailable services disable these controls.

Clients send event names, never file paths or shell commands. Names contain
letters, digits, hyphens and underscores, up to 80 characters. The service
looks in the configured theme, then freedesktop, under the SDK's user and
system data directories. Within a theme it checks `stereo/` then the root,
and `.oga`, `.ogg`, `.wav`. `screen-capture` falls back to `camera-shutter`.
This is a fixed-directory lookup, not a full index.theme inheritance resolver.
Missing events are silent.

Playback uses an owned `pw-play` process, one sound at a time, without a queue.
Requests within 150 ms of an accepted event are dropped, and playback is
limited to five seconds. PipeWire uses the default output and its existing
volume/mute settings. The service cleans up its player on exit. Debian runtime
dependencies include `pipewire-bin` and `sound-theme-freedesktop`.

Current callers:

- Session desktop startup: `service-login`, once per session, not on a services restart.
- Screenshot: `screen-capture`; successful capture notifications suppress a second sound.
- Notification banners: `sound-name` or `message-new-instant`; obey `suppress-sound`.
  Replacements, deduplicated notifications, closed pending notifications, DND,
  center-only/off applications and banners suppressed by Notification Center
  do not play. Even critical banners stay silent under DND/center-only policy.

Arbitrary `sound-file` notification hints are not supported. Logout, device
plug/unplug and volume-change events are not wired yet; callers can use the
same playEvent interface when their lifecycle policies are implemented.

Build and test independently:

```sh
cmake -S tests/sound -B build-sound
cmake --build build-sound
ctest --test-dir build-sound --output-on-failure
```

The test uses a private bus and temporary theme fixtures, never the user's
settings or speakers. Shell notification policy coverage is in the compositor's
`notification_service_test` target.
