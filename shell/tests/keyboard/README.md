# Screen keyboard verification

Build this standalone CMake project against an installed SDK to test the keyboard model, SDK pointer interactions, XKB conversion and global shortcut D-Bus contract. It also compiles the real Shell controller and full-shell test source as object targets to catch API errors.

~~~sh
cmake -S aurora/tests/keyboard -B keyboard-build -DCMAKE_PREFIX_PATH=/path/to/sdk
cmake --build keyboard-build
ctest --test-dir keyboard-build --output-on-failure
keyboard-build/screen-keyboard-capture captures
~~~

The object-only shell test check does not execute Seat/WindowManager integration. Run screen_keyboard_test and AcceleratorTest.ScreenKeyboardShortcutIsBlockedOnLockScreen in a complete compositor build for that integration. Settings keyboardpage.cpp should also be compiled against the SDK. No test sends events to the desktop session; tests use fake or headless input.
