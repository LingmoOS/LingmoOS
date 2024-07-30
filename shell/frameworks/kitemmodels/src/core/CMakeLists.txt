add_library(KF6ItemModels)
add_library(KF6::ItemModels ALIAS KF6ItemModels)

# needed for the QML module
qt_extract_metatypes(KF6ItemModels)

set_target_properties(KF6ItemModels PROPERTIES
    VERSION     ${KITEMMODELS_VERSION}
    SOVERSION   ${KITEMMODELS_SOVERSION}
    EXPORT_NAME ItemModels
)

target_sources(KF6ItemModels PRIVATE
    kbihash_p.h
    kbreadcrumbselectionmodel.cpp
    kbreadcrumbselectionmodel.h
    kcheckableproxymodel.cpp
    kcheckableproxymodel.h
    kcolumnheadersmodel.cpp
    kcolumnheadersmodel.h
    kdescendantsproxymodel.cpp
    kdescendantsproxymodel.h
    kextracolumnsproxymodel.cpp
    kextracolumnsproxymodel.h
    klinkitemselectionmodel.cpp
    klinkitemselectionmodel.h
    kmodelindexproxymapper.cpp
    kmodelindexproxymapper.h
    knumbermodel.cpp
    knumbermodel.h
    krearrangecolumnsproxymodel.cpp
    krearrangecolumnsproxymodel.h
    kselectionproxymodel.cpp
    kselectionproxymodel.h
    kvoidpointerfactory_p.h
)

ecm_qt_declare_logging_category(KF6ItemModels
    HEADER kitemmodels_debug.h
    IDENTIFIER KITEMMODELS_LOG
    CATEGORY_NAME kf.itemmodels.core
    OLD_CATEGORY_NAMES kf5.kitemmodels
    DESCRIPTION "KItemModels (Core)"
    EXPORT KITEMMODELS
)

ecm_generate_export_header(KF6ItemModels
    BASE_NAME KItemModels
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6ItemModels INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KItemModels>")

target_link_libraries(KF6ItemModels PUBLIC Qt6::Core)

ecm_generate_headers(KItemModels_HEADERS
    HEADER_NAMES
        KBreadcrumbSelectionModel
        KCheckableProxyModel
        KColumnHeadersModel
        KDescendantsProxyModel
        KExtraColumnsProxyModel
        KLinkItemSelectionModel
        KModelIndexProxyMapper
        KNumberModel
        KRearrangeColumnsProxyModel
        KSelectionProxyModel
    REQUIRED_HEADERS KItemModels_HEADERS
)

install(TARGETS KF6ItemModels EXPORT KF6ItemModelsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kitemmodels_export.h
    ${KItemModels_HEADERS}
    ${KItemModels_Legacy_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KItemModels
    COMPONENT Devel
)

if (BUILD_QCH)
    ecm_add_qch(
        KF6ItemModels_QCH
        NAME KItemModels
        BASE_NAME KF6ItemModels
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KItemModels_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KITEMMODELS_EXPORT
            "KITEMMODELS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
