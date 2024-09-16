# common src
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE SRCS   "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB D_HEADERS
    ${PROJECT_SOURCE_DIR}/src/DAppLoader
    ${PROJECT_SOURCE_DIR}/src/DQmlAppMainWindowInterface
    ${PROJECT_SOURCE_DIR}/src/DQmlAppPreloadInterface
    ${PROJECT_SOURCE_DIR}/src/DQuickBlitFramebuffer
    ${PROJECT_SOURCE_DIR}/src/DQuickItemViewport
    ${PROJECT_SOURCE_DIR}/src/DQuickWindow
)
file(GLOB PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/src/dapploader.h
    ${PROJECT_SOURCE_DIR}/src/dqmlappmainwindowinterface.h
    ${PROJECT_SOURCE_DIR}/src/dqmlapppreloadinterface.h
    ${PROJECT_SOURCE_DIR}/src/dquickblitframebuffer.h
    ${PROJECT_SOURCE_DIR}/src/dquickitemviewport.h
    ${PROJECT_SOURCE_DIR}/src/dquickwindow.h
    ${PROJECT_SOURCE_DIR}/src/dtkdeclarative_global.h
)

# exclusive to dtk5
if (EnableDtk5)
    list(APPEND D_HEADERS
        ${PROJECT_SOURCE_DIR}/src/DPlatformThemeProxy
        ${PROJECT_SOURCE_DIR}/src/DQuickSystemPalette
    )
    list(APPEND PUBLIC_HEADERS
        ${PROJECT_SOURCE_DIR}/src/dplatformthemeproxy.h
        ${PROJECT_SOURCE_DIR}/src/dquicksystempalette.h
    )

    list(REMOVE_ITEM HEADERS
        ${PROJECT_SOURCE_DIR}/src/private/dbackdropnode_p.h
        ${PROJECT_SOURCE_DIR}/src/private/dquickbackdropblitter_p.h
    )
    list(REMOVE_ITEM SRCS
        ${PROJECT_SOURCE_DIR}/src/private/dbackdropnode.cpp
        ${PROJECT_SOURCE_DIR}/src/private/dquickbackdropblitter.cpp
    )
endif()

# exclusive to dtk6
if (EnableDtk6)
    list(REMOVE_ITEM HEADERS
      ${PROJECT_SOURCE_DIR}/src/dquicksystempalette.h
      ${PROJECT_SOURCE_DIR}/src/private/dquicksystempalette_p.h
    )
    list(REMOVE_ITEM SRCS
      ${PROJECT_SOURCE_DIR}/src/dquicksystempalette.cpp
    )
endif()
