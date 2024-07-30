add_definitions(-DTRANSLATION_DOMAIN=\"balooengine6\")

set(BALOO_ENGINE_SRCS
    andpostingiterator.cpp
    database.cpp
    document.cpp
    documentdb.cpp
    documentdatadb.cpp
    documenturldb.cpp
    documenttimedb.cpp
    documentiddb.cpp
    enginequery.cpp
    idtreedb.cpp
    idfilenamedb.cpp
    indexerstate.cpp
    mtimedb.cpp
    orpostingiterator.cpp
    phraseanditerator.cpp
    positiondb.cpp
    postingdb.cpp
    postingiterator.cpp
    termgenerator.cpp
    transaction.cpp
    vectorpostingiterator.cpp
    vectorpositioninfoiterator.cpp
    writetransaction.cpp
    global.cpp
    fsutils.cpp
)

ecm_qt_declare_logging_category(
    BALOO_ENGINE_SRCS
    HEADER "enginedebug.h"
    IDENTIFIER "Baloo::ENGINE"
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.baloo.engine
    OLD_CATEGORY_NAMES org.kde.baloo.engine
    DESCRIPTION "Baloo Engine"
    EXPORT BALOO
)

if(${BUILD_EXPERIMENTAL})
  set(BALOO_ENGINE_SRCS ${BALOO_ENGINE_SRCS} experimental/databasesanitizer.cpp)
endif()

add_library(KF6BalooEngine ${BALOO_ENGINE_SRCS})
add_library(KF6::BalooEngine ALIAS KF6BalooEngine)

qt_extract_metatypes(KF6BalooEngine)

target_link_libraries(KF6BalooEngine
    PUBLIC
    Qt6::Core
    KF6::CoreAddons
    ${LMDB_LIBRARIES}
    PRIVATE
    KF6BalooCodecs
    KF6::I18n
)

set_target_properties(KF6BalooEngine PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME BalooEngine
)

##target_include_directories(KF6BalooEngine INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Baloo>")

generate_export_header(KF6BalooEngine BASE_NAME BALOO_ENGINE EXPORT_FILE_NAME engine_export.h)

install(TARGETS KF6BalooEngine EXPORT KF6BalooTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
