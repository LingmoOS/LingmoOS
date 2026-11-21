# Desktop widget review

Builds the real widget sources and a headless interaction/rendering executable. The desktop host is compiled as an object to check the gallery integration; this is not a full file manager build.

Requires the installed Lingmo SDK, Lingmo Files headers, ICU and libcurl development packages, and CMake 3.22 or later.

	cmake -S tests/widgets -B build-widget-review
	cmake --build build-widget-review --parallel 1
	ctest --test-dir build-widget-review --output-on-failure

The optional output argument captures all widget cards and the widget gallery:

	./build-widget-review/widget-review ./build-widget-review/captures

Use WIDGET_REVIEW_LOCALE_DIR to load a widget-review gettext catalog from a temporary locale directory. The Chinese screenshot check uses the installed en_US.UTF-8 locale with LANGUAGE=zh_CN. WIDGET_REVIEW_FONT can select a font family installed in the verification environment; FONTCONFIG_FILE can point at an isolated font configuration. Neither font files nor private desktop settings belong in this repository.

The test uses temporary HOME and widget configuration, and leaves weather unconfigured to avoid network requests. It reads host hardware status and available application icons. Real compositor glass, multi-output dragging and online services require desktop validation.

Set COMPOSITOR_SOURCE_DIR to the compositor checkout when configuring to compile the real notification host and run the dynamic widget-provider test. The provider uses the same hidden-symbol contract as the packaged library; close/reopen and visibility checks verify that saved content and layout survive while live widget resources are released. The Settings widget page is also syntax-checked by the workspace verification script.
