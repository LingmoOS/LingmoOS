
if (NOT TARGET KF6::KIOWidgets)
   set(KIOCORE_ONLY ON)
   add_definitions(-DKIOCORE_ONLY=1)
   set(kio_libs KF6::KIOCore)
else ()
   set(kio_libs KF6::KIOWidgets)
endif ()

function(add_kioclient_interface TARGET_NAME)
    add_executable(${TARGET_NAME} kioclient.cpp)
    string(TOUPPER "${TARGET_NAME}" UPPER_TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-DKIOCLIENT_AS_${UPPER_TARGET_NAME}")
    target_compile_definitions(${TARGET_NAME} PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(${TARGET_NAME} Qt::DBus Qt::Widgets KF6::CoreAddons ${kio_libs} KF6::I18n)
    install_compat_symlink(${TARGET_NAME})
    install(TARGETS ${TARGET_NAME} DESTINATION ${KDE_INSTALL_FULL_BINDIR})
endfunction()

#we compile every file with different definitions, so it will behave slightly different
add_kioclient_interface(kioclient)
add_kioclient_interface(kdecp)
add_kioclient_interface(kdemv)

#kioclient is not marked as nongui since download and openProperties do graphical things
ecm_mark_nongui_executable(kdemv)
ecm_mark_nongui_executable(kdecp)

if (NOT KIOCORE_ONLY)
add_executable(kde-open kioclient.cpp)
target_compile_definitions(kde-open PRIVATE "-DKIOCLIENT_AS_KDEOPEN")
target_compile_definitions(kde-open PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kde-open Qt::DBus KF6::CoreAddons KF6::KIOWidgets KF6::I18n)
install_compat_symlink(kde-open)
install(TARGETS kde-open DESTINATION ${KDE_INSTALL_FULL_BINDIR})
endif()
