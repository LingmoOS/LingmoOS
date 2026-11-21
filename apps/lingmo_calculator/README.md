# Lingmo Calculator

A C++20 calculator built with Lingmo SDK controls. It supports arithmetic with operator precedence, parentheses, unary signs, decimal and scientific notation, percentage, clipboard operations, and the last 50 calculations in a scrollable session history. Percent is postfix division by 100: 200*10% = 20; 100+10% = 100.1. Results display up to 15 significant digits; this is not an arbitrary-precision decimal calculator.

## Desktop global menus

The application exports com.canonical.dbusmenu at /org/lingmo/Calculator/Menu and registers a nonzero application window ID with com.canonical.AppMenu.Registrar. Lingmo's status bar resolves this registration by the focused window's process ID. File, Edit, Mode, Calculation and Help menus appear while Calculator is active. Actions share the same handlers as the SDK buttons and keyboard shortcuts. The exporter re-registers when the registrar restarts and unregisters on exit. A compact title-bar menu button is shown until global-menu registration succeeds, and is hidden after the acknowledgement. If registration fails or the registrar disappears, that button remains or reappears. There is no separate in-window mode/navigation row. The history page keeps its own Back button.

The exporter is adapted from filemanager/src/desktop/desktopmenuexporter with its original copyright notices retained. It intentionally lives in the application until a shared SDK menu-export API is established. No compositor change is required.

## Advanced modes

- Scientific: sin, cos, tan, asin, acos, atan, sqrt, ln (natural logarithm), log (base 10), exp, abs, pi, e, powers and factorials. RAD/DEG selects radians or degrees. Power is right-associative; -2^2 means -(2^2). Factorials require integers from 0 through 170. Domain errors preserve the input. Use explicit multiplication, e.g. 2*pi.
- Function Graph: enter an expression in x, press Plot or Enter, drag to pan and scroll or use +/- to zoom. Reset View restores x=[-10,10], y=[-5,5]. RAD/DEG affects trigonometry. Invalid samples and detected discontinuities break the curve. Sampling is bounded and is not a symbolic proof of continuity; very narrow features can be missed. One function is plotted at a time.
- Programmer: exact unsigned 64-bit integer calculations, HEX/DEC/OCT/BIN conversion, signed two's-complement readout, &, |, ^ (XOR), ~, <<, >>, arithmetic and remainder. Arithmetic wraps modulo 2^64, right shifts are logical, and shift counts must be 0–63. Numeric literals beyond 64 bits are rejected. Explicit 0x/0b/0o prefixes override the selected input base. Changing base converts the current valid value; an invalid expression must be corrected first.
- History: a standalone page containing the last 50 scientific/standard and programmer results. Selecting a record restores its value and associated angle unit or integer base. Programmer history never passes through floating-point conversion. Graph plots are not added to numeric history.

The title is centered on the complete window, with symmetric space reserved for window controls. All modes use SDK controls; graph rendering uses SDK Painter. The global Mode menu includes the current-mode radio state.

## Glass surface

The title, display and toolbar above the keypad use the SDK's popupGlass material through Window::setBackgroundGlass. Standard and scientific modes keep every keypad row opaque; programmer mode also exposes glass behind the base conversions above its keypad. History uses glass only in its header, and graph mode only in its title bar. The material follows the light/dark theme and uses grayscale text over transparency.

The whole-window blur bounds preserve the compositor's window outline and shadow; the opaque lower body covers the material below the header region. When backgroundGlassAvailable is false, the upper area uses a solid theme color instead. Headless captures show that fallback; alpha tests verify material requests and transparency, but the actual blurred desktop must be checked in a Lingmo compositor session.

## Build on Linux

Install LingmoSDK development files, CMake 3.22+, a C++20 compiler, pkg-config and gettext. For tests, install dbus-daemon (dbus-run-session).

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
sudo cmake --install build
~~~

When the SDK is under a custom prefix, pass -DCMAKE_PREFIX_PATH=/path/to/prefix. Run lingmo-calculator in a Lingmo Wayland session to check the status bar menus. Confirm menu actions, focus switching to another app and back, and single-instance activation.

SDK-free arithmetic tests:

~~~sh
cmake -S . -B build-model -DCALCULATOR_BUILD_UI=OFF
cmake --build build-model
ctest --test-dir build-model --output-on-failure
~~~

Headless screenshots (light/dark, 100%/150%, all modes and the standalone history page):

~~~sh
build/tests/calculator-capture captures
~~~

## Shortcuts

- Digits, decimal point, + - * / ( ) %: expression input
- Enter or =: calculate
- Escape / Delete / Ctrl+L: clear
- Backspace: delete last character
- Ctrl+C / Ctrl+V: copy / paste an expression
- Ctrl+1 / Ctrl+2 / Ctrl+3 / Ctrl+4: standard / scientific / graph / programmer
- Ctrl+H: open history / return
- Ctrl+Q: quit

Completed expressions show a live result preview without adding history. History rows can be clicked to reuse a result; typing an operator continues it and a digit starts a new calculation. History is a separate page: the keypad and display are hidden, and the window is not resized. Back returns to the previous mode. Each mode retains its own window size. Clipboard input accepts ×, ÷ and − as well as ASCII operators. Repeated decimal points are ignored and a pending binary operator can be corrected.

History is session-only. The sign button negates the entire current expression. Input and nesting are bounded; invalid expressions and division by zero preserve the expression for correction. Paste accepts arithmetic text only, never executes code.

GPL-3.0-or-later; see LICENSE.
