include(ECMMarkAsTest)

find_package(Qt6Widgets ${REQUIRED_QT_VERSION} CONFIG REQUIRED)
set_package_properties(Qt6Widgets PROPERTIES PURPOSE "Required for tests")

add_executable(projecttest)
ecm_mark_as_test(projecttest)

set(editproject_UI_H)
qt_wrap_ui(editproject_UI_H editproject.ui)

target_sources(projecttest PRIVATE
    projecttest.cpp
    main.cpp
    ${editproject_UI_H}
)

target_link_libraries(projecttest Qt6::Widgets KF6::Attica)
