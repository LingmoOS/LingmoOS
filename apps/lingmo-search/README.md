# Lingmo Search

A native focused search overlay inspired by the center search control in the supplied reference: a glass input pill and four round category buttons, centered above the middle of the primary display. Typing expands a solid results surface below. This follows the project material standard (glass above, opaque results below); it is a desktop search overlay without a normal application title/menu bar.

## Use

- Super+Space opens/closes the overlay with the compositor integration in this workspace. The standalone executable and desktop entry can also toggle it.
- The four buttons filter Applications, Files, Settings and Calculator. Click the active category again to return to All.
- Type to search; Up/Down select, Enter opens the selected result or copies a calculation, Escape closes. Clicking elsewhere closes after the search window loses focus.
- Application results use the SDK application registry and localized desktop-entry names, descriptions and keywords.
- Settings results launch documented Settings page IDs through lingmo-settings -m. Availability of a hardware-specific setting depends on the machine.
- Files are matched by name in XDG Desktop, Documents, Downloads, Pictures, Music and Videos directories. Hidden entries, symbolic links and common generated directories are excluded. No file contents are read, no search history or filename index is written to disk, and there is no network provider.
- File scanning stops at 30,000 visited entries, 300 ms or 12 levels of depth. The footer reports a truncated scan. This first version is a bounded on-demand search, not a complete home-directory index. Slow filesystem operations themselves cannot be preempted by the time budget.
- Matching is Unicode case-insensitive exact/prefix/substring ranking, with names preferred over descriptions. Chinese names and translated keywords work; pinyin transliteration and fuzzy typo correction are not implemented.
- Calculator results support arithmetic, parentheses, powers, common scientific functions and pi/e using the current Calculator expression evaluator. Radians, up to 15 displayed significant digits. Invalid/domain-error expressions produce no result; no shell expressions are executed.

## Integration

The session D-Bus activates /usr/bin/lingmo-search --service via org.lingmo.Search.service. The object /org/lingmo/Search implements toggle(string activationToken), show(string activationToken) and hide(). The compositor forwards a user-activation token and does not wait for searches. The action is named toggle-search and is available in Settings shortcut customization; it is excluded from lock-screen and modal-dialog allowlists.

The desktop metapackage includes lingmo-search. Install the search package and rebuild/install the compositor and Settings changes to use the default binding. Existing user shortcuts which override Super+Space must be changed explicitly; the new default does not erase user configuration.

Selected application/setting/file actions use org.lingmo.AppRuntime at /AppRuntime. Commands are passed as argument arrays, never shell strings. Missing runtime or failed launches produce an error in the overlay. Files are rechecked before opening via xdg-open and are not read by a previewer. Calculation copying uses the SDK clipboard. Dismissing destroys the Wayland window after the current input callback; reopening creates a new one. The app stays alive after first activation so clipboard ownership survives dismissal; worker requests are canceled on dismissal and there is no background file crawling while idle.

Positioning uses the SDK placement-anchor protocol on the primary output. Other Wayland compositors may choose placement themselves. Actual focus, multi-monitor placement, click-away dismissal and backdrop blur need live Lingmo validation.

## Build

Requires Linux, CMake >= 3.22, a C++20 compiler, LingmoSDK development files, ICU development files, pkg-config, gettext and dbus-run-session for tests.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
sudo cmake --install build
~~~

Optionally set COMPOSITOR_SOURCE_DIR to the adjacent compositor checkout to compile the default-shortcut and D-Bus delegate contract test. The compositor also has an end-to-end accelerator test for Super+Space and lock-screen suppression.

~~~sh
build/tests/search-capture captures
~~~

Headless captures use deterministic sample app data and the solid material fallback, not actual desktop blur. The arithmetic implementation was adapted from this workspace's Calculator (GPL-3.0-or-later). This new component is not committed or published automatically.

## Validation status (2026-11-07)

The search executable built successfully against the packaged SDK in a native WSL temporary directory. All five CTest groups passed: translations, provider search, SDK UI/lifecycle, AppRuntime integration, and compositor shortcut/delegate contract. Tests include Unicode filenames/keywords, exact-title ranking, cancellation, private-path exclusion, result caps, repeated window destruction/recreation, focus-loss dismissal, runtime launch failure, clipboard copy and glass/solid boundaries. Light/dark captures at 100% and 150% were rendered and visually inspected.

Staged installation verified the binary, Chinese catalog, desktop entry and D-Bus activation service (Exec=/usr/bin/lingmo-search --service). Debian native source packaging also passed. No system installation or push was performed. WSL was shut down after verification.

The local SDK package links ICU 76 while the test host's ICU development package is version 78; the linker emitted coexistence warnings. Tests passed, but release builds should use a distribution-matched SDK/dependency stack. The entire compositor was not rebuilt: its action-name/default-binding/delegate sources were compiled by the contract test, while the updated end-to-end accelerator test remains for the full compositor suite. Live Lingmo multi-monitor placement, input methods, activation focus and actual glass blur remain to be verified.
