# The terminal's emulator, session and pty, derived from Konsole.

pkg_check_modules(PCRE2 REQUIRED IMPORTED_TARGET libpcre2-16)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

add_library(terminal-core STATIC
    lib/BlockArray.cpp
    lib/ColorScheme.cpp
    lib/Emulation.cpp
    lib/Filter.cpp
    lib/History.cpp
    lib/HistorySearch.cpp
    lib/KeyboardTranslator.cpp
    lib/KeyCodes.cpp
    lib/konsole_wcwidth.cpp
    lib/ProxyEnvironment.cpp
    lib/PtyProcess.cpp
    lib/RegExp.cpp
    lib/Screen.cpp
    lib/ScreenWindow.cpp
    lib/TerminalCharacterDecoder.cpp
    lib/TerminalSession.cpp
    lib/tools.cpp
    lib/Vt102Emulation.cpp
)

target_include_directories(terminal-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# Where the colour schemes and key layouts are read; a build tree run can point it at lib/.
set(TERMINAL_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/lingmo-terminal" CACHE PATH
    "Directory holding color-schemes/ and kb-layouts/")

target_compile_definitions(terminal-core PRIVATE
    HAVE_POSIX_OPENPT
    HAVE_SYS_TIME_H
    TERMINAL_DATA_DIR="${TERMINAL_DATA_DIR}"
)
target_link_libraries(terminal-core
    PUBLIC
        Lingmo::SDKBase
    PRIVATE
        PkgConfig::ICU
        PkgConfig::PCRE2
        PkgConfig::XKBCOMMON
)

# The display, on the toolkit.
add_library(terminal-view STATIC
    lib/TerminalFont.cpp
    lib/TerminalView.cpp
)
target_link_libraries(terminal-view
    PUBLIC
        terminal-core
        Lingmo::SDK
    PRIVATE
        PkgConfig::ICU
        PkgConfig::XKBCOMMON
)

install(DIRECTORY lib/color-schemes DESTINATION ${CMAKE_INSTALL_DATADIR}/lingmo-terminal)
install(DIRECTORY lib/kb-layouts DESTINATION ${CMAKE_INSTALL_DATADIR}/lingmo-terminal)
