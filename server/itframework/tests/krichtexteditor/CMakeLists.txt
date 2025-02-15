add_executable(krichtexteditor)

target_sources(krichtexteditor PRIVATE
    main.cpp
    krichtexteditor.cpp
)

target_include_directories(krichtexteditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
ecm_mark_as_test(krichtexteditor)
target_link_libraries(krichtexteditor Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets KF5::TextWidgets KF5::XmlGui)

#install(TARGETS krichtexteditor DESTINATION ${KDE_INSTALL_BINDIR})
#install(FILES krichtexteditorui.rc DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/krichtexteditor)

