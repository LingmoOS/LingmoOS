# mingw can't handle exported explicit template instantiations in a DLL
if (MINGW)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols ${CMAKE_SHARED_LINKER_FLAGS}")
endif (MINGW)

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

add_library(KF6Attica)
add_library(KF6::Attica ALIAS KF6Attica)

set_target_properties(KF6Attica PROPERTIES
    VERSION     ${ATTICA_VERSION}
    SOVERSION   ${ATTICA_SOVERSION}
    EXPORT_NAME "Attica"
)

target_sources(KF6Attica PRIVATE
   accountbalance.cpp
   accountbalanceparser.cpp
   achievement.cpp
   achievementparser.cpp
   buildservice.cpp
   buildserviceparser.cpp
   buildservicejob.cpp
   buildservicejobparser.cpp
   buildservicejoboutput.cpp
   buildservicejoboutputparser.cpp
   activity.cpp
   activityparser.cpp
   atticabasejob.cpp
   atticautils.cpp
   privatedata.cpp
   privatedataparser.cpp
   category.cpp
   categoryparser.cpp
   comment.cpp
   commentparser.cpp
   config.cpp
   configparser.cpp
   content.cpp
   contentparser.cpp
   deletejob.cpp
   distribution.cpp
   distributionparser.cpp
   downloaddescription.cpp
   downloaditem.cpp
   downloaditemparser.cpp
   event.cpp
   eventparser.cpp
   folder.cpp
   folderparser.cpp
   forum.cpp
   forumparser.cpp
   getjob.cpp
   homepageentry.cpp
   homepagetype.cpp
   homepagetypeparser.cpp
   icon.cpp
   itemjob.cpp
   knowledgebaseentry.cpp
   knowledgebaseentryparser.cpp
   license.cpp
   licenseparser.cpp
   listjob_inst.cpp
   message.cpp
   messageparser.cpp
   metadata.cpp
   parser.cpp
   person.cpp
   personparser.cpp
   platformdependent_v2.cpp
   postfiledata.cpp
   postjob.cpp
   project.cpp
   projectparser.cpp
   putjob.cpp
   remoteaccount.cpp
   remoteaccountparser.cpp
   provider.cpp
   providermanager.cpp
   publisher.cpp
   publisherparser.cpp
   publisherfield.cpp
   publisherfieldparser.cpp
   qtplatformdependent.cpp
   topic.cpp
   topicparser.cpp
   platformdependent_v3.cpp
)

ecm_qt_declare_logging_category(KF6Attica
    HEADER attica_debug.h
    IDENTIFIER ATTICA
    CATEGORY_NAME kf.attica
    OLD_CATEGORY_NAMES org.kde.attica
    DESCRIPTION "Attica"
    EXPORT ATTICA
)

file(GLOB TEST_FILES "*.cpp" "*.h")
ecm_check_outbound_license(
    LICENSES LGPL-2.1-only LGPL-3.0-only
    TEST_NAME KF6Attica
    FILES ${TEST_FILES}
)

ecm_generate_export_header(KF6Attica
    BASE_NAME Attica
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6Attica
PUBLIC
  Qt6::Core
  Qt6::Network
)
target_include_directories(KF6Attica INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Attica>")


ecm_generate_headers(Attica_CamelCase_HEADERS
  HEADER_NAMES
  Activity
  AccountBalance
  Achievement
  Comment
  Config
  Content
  Category
  DeleteJob
  Distribution
  DownloadDescription
  DownloadItem
  Event
  Folder
  Forum
  GetJob
  HomePageEntry
  HomePageType
  Icon
  ItemJob
  KnowledgeBaseEntry
  ListJob
  License
  Message
  Metadata
  Person
  PrivateData
  PostJob
  Project
  PutJob
  RemoteAccount
  BuildService
  BuildServiceJob
  BuildServiceJobOutput
  ProviderManager
  Provider
  Publisher
  PublisherField
  Topic

  PREFIX Attica
  REQUIRED_HEADERS Attica_HEADERS
)

set(Attica_HEADERS
    ${Attica_HEADERS}
    atticabasejob.h # TODO: rename to basejob.h, add atticabasejob.h forwarding to basejob.h for compat
    atticautils.h   # TODO: not exported, this should probably NOT be installed at all?
    #interface for external platform plugins
    platformdependent.h
    platformdependent_v2.h
    platformdependent_v3.h
)

install(FILES ${Attica_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Attica/Attica COMPONENT Devel)

install(TARGETS KF6Attica
        EXPORT KF6AtticaTargets
        ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${Attica_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    ${CMAKE_CURRENT_BINARY_DIR}/attica_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Attica/attica
    COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT ATTICA
    FILE attica.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Attica_QCH
        NAME Attica
        BASE_NAME KF6Attica
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Attica_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Network_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            ATTICA_EXPORT
            ATTICA_DEPRECATED
            ATTICA_DEPRECATED_EXPORT
            "ATTICA_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

# Install package config file
if(NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6Attica
        LIB_NAME KF6Attica
        INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/Attica
        DEPS Qt6Core Qt6Network
        DESCRIPTION "Qt library to access Open Collaboration Services"
        INSTALL
    )
endif(NOT WIN32)

