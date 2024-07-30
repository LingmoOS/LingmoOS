add_definitions(-DTRANSLATION_DOMAIN=\"baloosearch6\")

set(BALOO_SEARCH_SRCS main.cpp)

add_executable(baloosearch6 ${BALOO_SEARCH_SRCS})
ecm_mark_nongui_executable(baloosearch6)
target_compile_definitions(baloosearch6 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloosearch6
  KF6::I18n
  KF6::CoreAddons
  KF6::Baloo
)

install(TARGETS baloosearch6 DESTINATION ${KDE_INSTALL_BINDIR})
