if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_filetypes\")

set(libfiletypes_SRCS
 filetypedetails.cpp
 filegroupdetails.cpp
 kservicelistwidget.cpp
 typeslistitem.cpp
 typeslistitembase.cpp
 typeslistproxyitem.cpp
 typeslisttreewidget.cpp
 mimetypedata.cpp
 mimetypewriter.cpp
 multiapplydialog.cpp
 newtypedlg.cpp
 kserviceselectdlg.cpp
 filetypedetails.h
 filegroupdetails.h
 kservicelistwidget.h
 typeslistitem.h
 typeslistitembase.h
 typeslistproxyitem.h
 typeslisttreewidget.h
 mimetypedata.h
 mimetypewriter.h
 multiapplydialog.h
 newtypedlg.h
 kserviceselectdlg.h
)

########### next target ###############

set(kcm_filetypes_SRCS filetypesview.cpp ${libfiletypes_SRCS})

kcoreaddons_add_plugin(kcm_filetypes SOURCES ${kcm_filetypes_SRCS} INSTALL_NAMESPACE "lingmo/kcms/systemsettings_qwidgets")

kcmutils_generate_desktop_file(kcm_filetypes)
target_link_libraries(kcm_filetypes
    KF6::ConfigWidgets
    KF6::IconThemes
    KF6::IconWidgets
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    KF6::Parts
    Qt::DBus
)

########### next target ###############

set(keditfiletype_SRCS keditfiletype.cpp keditfiletype.h ${libfiletypes_SRCS})

add_executable(keditfiletype ${keditfiletype_SRCS})
target_compile_definitions(keditfiletype PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(keditfiletype
    KF6::ConfigCore
    KF6::IconThemes
    KF6::IconWidgets
    KF6::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    KF6::WindowSystem
    KF6::I18n
    KF6::Service
    KF6::Parts
    Qt::DBus
)

install_compat_symlink(keditfiletype)
install(TARGETS keditfiletype DESTINATION ${KDE_INSTALL_FULL_BINDIR})

########### install files ###############

install( PROGRAMS org.kde.keditfiletype.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
