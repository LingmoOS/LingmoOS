
set(Formatter_SOVERSION 2)

set(formatter_SRCS
    Formatter.cpp
    Unit.cpp
    FormatterWrapper.cpp
)

set(formatter_HDRS
    Formatter.h
    Unit.h
    FormatterWrapper.h
)

ecm_qt_declare_logging_category(formatter_SRCS
    HEADER formatter_debug.h
    IDENTIFIER FORMATTER
    CATEGORY_NAME org.kde.libksysguard.formatter
    DESCRIPTION "libksysguard (formatter)" EXPORT LIBKSYSGUARD
)

add_library(Formatter ${formatter_SRCS} ${formatter_HDRS})
add_library(KSysGuard::Formatter ALIAS Formatter)

ecm_add_qml_module(Formatter URI "org.kde.ksysguard.formatter" VERSION 1.0 GENERATE_PLUGIN_SOURCE)

ecm_finalize_qml_module(Formatter)

target_link_libraries(Formatter
    PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    PRIVATE
    KF6::CoreAddons
    KF6::I18n
)

generate_export_header(Formatter)

target_include_directories(Formatter
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

set_target_properties(Formatter PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardFormatter
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${Formatter_SOVERSION}
)

target_compile_definitions(Formatter PRIVATE -DTRANSLATION_DOMAIN=\"KSysGuardFormatter\")

install(TARGETS Formatter EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${formatter_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/formatter_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/formatter
)
