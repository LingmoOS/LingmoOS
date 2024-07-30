find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for tests")

include(ECMAddTests)

########### a KParts ###############

# don't use kcoreaddons_add_plugin here since we don't want to install it
add_library(notepadpart MODULE)
target_sources(notepadpart PRIVATE notepad.cpp notepad.qrc)
# so we have to do the INSTALL_NAMESPACE thing by hand:
set_target_properties(notepadpart PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/kf6/parts")
target_link_libraries(notepadpart KF6::Parts KF6::I18n)

########### tests ###############

ecm_add_tests(
  parttest.cpp
  partloadertest.cpp
  LINK_LIBRARIES KF6::Parts Qt6::Test KF6::XmlGui
)
