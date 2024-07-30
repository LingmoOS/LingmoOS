add_executable(kdesu_executable kdesu.cpp sudlg.cpp sudlg.h)
target_compile_definitions(kdesu_executable PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

# in KDELibsDependencies.cmake installed by kdelibs there is a dependency to "kdesu"
# which is then recognized here as the target name for this executable
# so give the target here a different name and use the OUTPUT_NAME property to 
# give it the name kdesu
set_target_properties(kdesu_executable PROPERTIES OUTPUT_NAME kdesu)

target_link_libraries(kdesu_executable Qt::GuiPrivate KF6::WidgetsAddons KF6::I18n KF6::ConfigCore KF6::WindowSystem KF6::Su)

install(TARGETS kdesu_executable DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})





