remove_definitions(-DTRANSLATION_DOMAIN=\"liblingmo6\")  # Undo the global domain
add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_containmentactions_test\")

set(lingmo_containmentactions_test_SRCS
    test.cpp
    test.h
)
ki18n_wrap_ui(lingmo_containmentactions_test_SRCS config.ui)

kcoreaddons_add_plugin(lingmo_containmentactions_test
    SOURCES ${lingmo_containmentactions_test_SRCS}
    INSTALL_NAMESPACE "lingmo/containmentactions"
)

set_target_properties(lingmo_containmentactions_test PROPERTIES
    OUTPUT_NAME org.kde.test
)

target_link_libraries(lingmo_containmentactions_test
    Qt::Widgets
    Lingmo::Lingmo
    KF6::I18n
)
