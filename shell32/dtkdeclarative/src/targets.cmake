# Add secure compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie")
# For mips64
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick DBus QuickControls2 LinguistTools)

find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR}Gui REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GL REQUIRED IMPORTED_TARGET gl)

include(${PROJECT_SOURCE_DIR}/src/src.cmake)

file(GLOB ASSETS_RCS ${PROJECT_SOURCE_DIR}/src/dtkdeclarative_assets.qrc)
file(GLOB TS_FILES "${PROJECT_SOURCE_DIR}/src/translations/*.ts")

add_library(${LIB_NAME}_properties INTERFACE)
add_library(${LIB_NAME}_sources INTERFACE)
target_sources(${LIB_NAME}_sources INTERFACE
    ${SRCS}
    ${HEADERS}
    ${D_HEADERS}
    ${ASSETS_RCS}
)

set(DTK_QML_APP_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/${LIB_NAME}/qml-app" CACHE STRING "dtk qml app plugin path")
set(DTK_QML_APP_PLUGIN_SUBPATH "dtkdeclarative/plugins" CACHE STRING "dtk qml app plugin subpath")
target_compile_definitions(${LIB_NAME}_properties INTERFACE
    DTK_QML_APP_PLUGIN_PATH="${DTK_QML_APP_PLUGIN_PATH}"
    DTK_QML_APP_PLUGIN_SUBPATH="${DTK_QML_APP_PLUGIN_SUBPATH}"
    DDECLARATIVE_TRANSLATIONS_DIR="${DDECLARATIVE_TRANSLATIONS_DIR}"
)

if(USE_QQuickStylePluginPrivate)
    target_link_libraries(${LIB_NAME}_properties INTERFACE
        $<BUILD_INTERFACE:Qt${QT_VERSION_MAJOR}::QuickControls2Private>
    )
endif()

target_link_libraries(${LIB_NAME}_properties INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    $<BUILD_INTERFACE:Qt${QT_VERSION_MAJOR}::QuickPrivate>
    $<BUILD_INTERFACE:Qt${QT_VERSION_MAJOR}::DBus>
    $<BUILD_INTERFACE:PkgConfig::GL>
)

target_include_directories(${LIB_NAME}_properties INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/private>
)

target_include_directories(${LIB_NAME}_properties INTERFACE
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_directories(${LIB_NAME}_properties INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${LIB_INSTALL_DIR}>
)

# gen dtkdeclarative_config.h
include(DtkBuildConfig)
gen_dtk_config_header(MODULE_NAME ${LIB_NAME} HEADERS ${D_HEADERS} DEST_DIR ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE CONFIG_PATH)
list(APPEND PUBLIC_HEADERS ${CONFIG_PATH})
# Install headers
install(FILES ${PUBLIC_HEADERS} ${D_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}")

# Install ${LIB_NAME}_properties
install(TARGETS ${LIB_NAME}_properties EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeProperties DESTINATION "${LIB_INSTALL_DIR}")
install(EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeProperties NAMESPACE Dtk${DTK_VERSION_MAJOR}:: FILE Dtk${DTK_VERSION_MAJOR}DeclarativePropertiesTargets.cmake DESTINATION "${CONFIG_INSTALL_DIR}")
