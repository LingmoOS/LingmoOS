add_definitions(-DTRANSLATION_DOMAIN=\"balooshow5\")

set(SRCS main.cpp)

add_executable(balooshow6 ${SRCS})
ecm_mark_nongui_executable(balooshow6)
target_compile_definitions(balooshow6 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(balooshow6
    KF6::Baloo
    KF6::BalooEngine
    KF6::FileMetaData
    KF6::CoreAddons
    KF6::I18n
)

install(TARGETS balooshow6 DESTINATION ${KDE_INSTALL_BINDIR})
