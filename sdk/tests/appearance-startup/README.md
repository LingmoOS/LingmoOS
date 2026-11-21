# Late appearance service regression

Build this directory with an installed Lingmo SDK and libsystemd. It compiles the current AppearanceWatcher source, lets its first request fail, then exports a real private-bus appearance service. The test must receive the configured icon theme and dark mode without a manual theme change.

Run the executable with dbus-run-session and an isolated session configuration without service activation. This focused test does not rebuild the whole SDK renderer.
