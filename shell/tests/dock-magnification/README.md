# Dock magnification regression tests

Build the real magnification and presentation code against the installed SDK,
without GoogleTest, a display server or the compositor's DRM development dependencies:

```sh
cmake -S tests/dock-magnification -B build-dock-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build-dock-tests
ctest --test-dir build-dock-tests --output-on-failure
```

The tests cover the unchanged ordered layout, unsorted presentation storage,
launcher drops followed by pointer sweeps, repeated insertion and reordering,
overlapping positions during transitions, and animation back to rest.
They are also registered by the full compositor test build.

`tests/shell/shell_components_test.cpp` additionally exercises a launcher MIME
drag into the actual DockWindow, then sweeps the dock in both directions and
checks the icon anchors and panel bounds. Run its `shell_components_test`
target in a full compositor test build for input and view integration.
