remove_definitions(-DTRANSLATION_DOMAIN=\"baloo_file6\")
add_definitions(-DTRANSLATION_DOMAIN=\"baloo_file_extractor6\")

set(EXTRACTOR_SRCS
  main.cpp
  app.cpp
  result.cpp
  commandpipe.cpp
  ../priority.cpp
  ../basicindexingjob.cpp
  ../fileindexerconfig.cpp
  ../storagedevices.cpp
  ../regexpcache.cpp
  ../fileexcludefilters.cpp
  ../propertydata.cpp
)

ecm_qt_declare_logging_category(EXTRACTOR_SRCS
    HEADER baloodebug.h
    IDENTIFIER BALOO
    CATEGORY_NAME kf.baloo
)
kconfig_add_kcfg_files(EXTRACTOR_SRCS ../baloosettings.kcfgc GENERATE_MOC)

add_executable(baloo_file_extractor ${EXTRACTOR_SRCS})
ecm_mark_nongui_executable(baloo_file_extractor)
target_compile_definitions(baloo_file_extractor PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloo_file_extractor
  Qt6::Gui
  KF6::FileMetaData
  KF6::I18n
  KF6::ConfigCore
  KF6::Solid
  KF6::BalooEngine
  KF6::Crash
  KF6::IdleTime
)

install(TARGETS baloo_file_extractor DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR_KF})
