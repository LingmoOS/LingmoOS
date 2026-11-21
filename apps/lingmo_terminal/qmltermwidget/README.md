# Terminal core

The terminal's emulator, session, pty and display. The emulation, history,
key translation and colour schemes are derived from Konsole by way of
[qmltermwidget](https://github.com/Swordfish90/qmltermwidget); the display
and font handling are rewritten on
[lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit).

It builds as two static libraries for the terminal:

- `terminal-core`: emulation, screen, history, sessions and the pty.
- `terminal-view`: `TerminalView` and `TerminalFont`, the toolkit display.

Colour schemes (`lib/color-schemes`, see `THEMES.md`) and keyboard layouts
(`lib/kb-layouts`) are installed under `share/lingmo-terminal`.

## Licensing

Released under GPL-2.0-or-later. Some files keep compatible licenses inherited
from upstream; see `LICENSE` and the file headers for details.
