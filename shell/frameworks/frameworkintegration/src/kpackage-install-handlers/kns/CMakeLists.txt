configure_file(knshandlerversion.h.in knshandlerversion.h)
add_executable(knshandler main.cpp)
target_link_libraries(knshandler KF6::NewStuffCore KF6::I18n KF6::Notifications)

install(TARGETS knshandler DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF}/kpackagehandlers)

add_executable(knshandlertest main.cpp)
target_link_libraries(knshandlertest KF6::NewStuffCore KF6::I18n KF6::Notifications)
target_compile_definitions(knshandlertest PRIVATE -DTEST)

if(EXISTS "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_CONFDIR}/colorschemes.knsrc")
    add_test(NAME test_kns-kpackage COMMAND knshandlertest kns://colorschemes.knsrc/api.kde-look.org/1136471)

    add_test(NAME test_kns-kpackage-fail COMMAND knshandlertest kns://colorschemes.knsrc/xxx/1136471)
    set_tests_properties(test_kns-kpackage-fail PROPERTIES WILL_FAIL TRUE)
    message(STATUS "KNS-KPackage test enabled")
endif()
