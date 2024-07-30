include(CMakePackageConfigHelpers)
if(TARGET Dmtx::Dmtx)
    set(HAVE_DMTX 1)
endif()
if (TARGET ZXing::ZXing)
    set(HAVE_ZXING 1)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-prison.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prison.h)

add_library(KF6Prison)
add_library(KF6::Prison ALIAS KF6Prison)

qt_extract_metatypes(KF6Prison)

set_target_properties(KF6Prison PROPERTIES
    VERSION     ${PRISON_VERSION}
    SOVERSION   ${PRISON_SOVERSION}
    EXPORT_NAME Prison
)

target_sources(KF6Prison PRIVATE
    abstractbarcode_p.cpp
    aztecbarcode.cpp
    aztecbarcode_p.h
    barcode.cpp
    barcodeutil.cpp
    barcodeutil_p.h
    bitvector.cpp
    bitvector_p.h
    code128barcode.cpp
    code128barcode_p.h
    code39barcode.cpp
    code39barcode_p.h
    code93barcode.cpp
    code93barcode_p.h
    mecard.cpp
    mecard.h
    prison.h
    qrcodebarcode.cpp
    qrcodebarcode_p.h
    reedsolomon.cpp
    reedsolomon_p.h
)
if(TARGET Dmtx::Dmtx)
    target_sources(KF6Prison PRIVATE datamatrixbarcode.cpp datamatrixbarcode_p.h)
endif()
if(TARGET ZXing::ZXing)
    target_sources(KF6Prison PRIVATE
        pdf417barcode.cpp
        pdf417barcode_p.h
        zxingutil.cpp
    )
endif()
kde_source_files_enable_exceptions(
    barcode.cpp
    pdf417barcode.cpp
    prison.cpp
    zxingutil.cpp
)

ecm_qt_declare_logging_category(KF6Prison
    HEADER prison_debug.h
    IDENTIFIER Prison::Log
    CATEGORY_NAME kf.prison
    OLD_CATEGORY_NAMES kf5.prison
    DESCRIPTION "Prison (lib)"
    EXPORT PRISON
)

ecm_generate_export_header(KF6Prison
    BASE_NAME Prison
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Prison
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Prison>"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # module version header
)

target_link_libraries(KF6Prison
PUBLIC
   Qt6::Gui
PRIVATE
   QRencode::QRencode
)
if(TARGET Dmtx::Dmtx)
    target_link_libraries(KF6Prison PRIVATE Dmtx::Dmtx)
endif()
if(TARGET ZXing::ZXing)
    target_link_libraries(KF6Prison PRIVATE ZXing::ZXing)
endif()

install(TARGETS KF6Prison EXPORT KF6PrisonTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(Prison_CamelCase_HEADERS
    HEADER_NAMES
    Barcode
    Prison
    MeCard
    REQUIRED_HEADERS Prison_HEADERS
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Prison
)

set(_all_headers
    ${Prison_HEADERS}
    ${Prison_CamelCase_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/prison_export.h
)

install(
    FILES ${_all_headers}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Prison/Prison
    COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Prison_QCH
        NAME Prison
        BASE_NAME KF6Prison
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Prison_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            PRISON_EXPORT
            PRISON_DEPRECATED
            PRISON_DEPRECATED_EXPORT
            "PRISON_DEPRECATED_VERSION(x, y, t)"
            "PRISON_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

