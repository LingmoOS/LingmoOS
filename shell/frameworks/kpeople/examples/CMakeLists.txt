add_executable(contactlistwidgets contactlistwidgets.cpp)
target_link_libraries(contactlistwidgets Qt6::Core Qt6::Widgets KF6::People)

find_package(Qt6Quick)
if (Qt6Quick_FOUND)
    qt_add_resources(contactlistqml_SRCS contactlistqml.qrc)
    add_executable(contactlistqml contactlistqml.cpp ${contactlistqml_SRCS})
    target_link_libraries(contactlistqml Qt6::Quick)
endif()

add_executable(personwidget personwidget.cpp)
target_link_libraries(personwidget Qt6::Core Qt6::Widgets KF6::People KF6::PeopleWidgets)


# add_executable(loadcontact loadcontact.cpp)
# target_link_libraries(loadcontact Qt6::Core Qt6::Gui KF6::People)

add_executable(duplicates duplicates.cpp)
target_link_libraries(duplicates KF6::People KF6::CoreAddons KF6::PeopleBackend)

add_executable(duplicatesdialog duplicatesdialog.cpp)
target_link_libraries(duplicatesdialog KF6::PeopleWidgets Qt6::Widgets)
