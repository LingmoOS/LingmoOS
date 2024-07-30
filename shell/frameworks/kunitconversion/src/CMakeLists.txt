add_library(KF6UnitConversion)
add_library(KF6::UnitConversion ALIAS KF6UnitConversion)

set_target_properties(KF6UnitConversion PROPERTIES
    VERSION     ${KUNITCONVERSION_VERSION}
    SOVERSION   ${KUNITCONVERSION_SOVERSION}
    EXPORT_NAME UnitConversion
)

ecm_qt_declare_logging_category(KF6UnitConversion
    HEADER kunitconversion_debug.h
    IDENTIFIER LOG_KUNITCONVERSION
    CATEGORY_NAME kf.kunitconversion
    DESCRIPTION "kunitconversion"
    EXPORT KUNITCONVERSION
)

target_sources(KF6UnitConversion PRIVATE
    converter.cpp
    value.cpp
    unit.cpp
    unitcategory.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    velocity.cpp
    mass.cpp
    pressure.cpp
    energy.cpp
    currency.cpp
    power.cpp
    timeunit.cpp
    fuel_efficiency.cpp
    density.cpp
    weight_per_area.cpp
    acceleration.cpp
    force.cpp
    angle.cpp
    frequency.cpp
    thermal_conductivity.cpp
    thermal_flux.cpp
    thermal_generation.cpp
    voltage.cpp
    electrical_current.cpp
    electrical_resistance.cpp
    permeability.cpp
    binary_data.cpp
)

if (WIN32)
    # As stated in http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx M_PI only gets defined
    # when using MSVC if _USE_MATH_DEFINES is defined (this is needed for angle.cpp)
    target_compile_definitions(KF6UnitConversion PRIVATE _USE_MATH_DEFINES)
endif()

ecm_generate_export_header(KF6UnitConversion
    BASE_NAME KUnitConversion
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/kunitconversion/kunitconversion_export.h
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

set(kunitconversion_BUILD_INCLUDE_DIRS ${KUnitConversion_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(KF6UnitConversion PUBLIC "$<BUILD_INTERFACE:${kunitconversion_BUILD_INCLUDE_DIRS}>")
target_include_directories(KF6UnitConversion INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KUnitConversion>")

target_link_libraries(KF6UnitConversion PUBLIC  Qt6::Core
                                        PRIVATE Qt6::Network
                                                KF6::I18n
)

ecm_generate_headers(KUnitConversion_CamelCase_HEADERS
    HEADER_NAMES
    Converter
    Value
    Unit
    UnitCategory

    PREFIX KUnitConversion
    REQUIRED_HEADERS KUnitConversion_HEADERS
)
install(FILES ${KUnitConversion_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUnitConversion/KUnitConversion COMPONENT Devel)

install(TARGETS KF6UnitConversion EXPORT KF6UnitConversionTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kunitconversion/kunitconversion_export.h
    ${KUnitConversion_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUnitConversion/kunitconversion COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KUNITCONVERSION
    FILE kunitconversion.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6UnitConversion_QCH
        NAME KUnitConversion
        BASE_NAME KF6UnitConversion
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KUnitConversion_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${kunitconversion_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KUNITCONVERSION_EXPORT
            KUNITCONVERSION_DEPRECATED
            KUNITCONVERSION_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

