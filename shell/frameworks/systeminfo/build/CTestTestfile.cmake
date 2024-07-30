# CMake generated Testfile for 
# Source directory: /home/lingmo/project/nx_pkg/systeminfo
# Build directory: /home/lingmo/project/nx_pkg/systeminfo/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(appstreamtest "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/home/lingmo/project/nx_pkg/systeminfo/build/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties(appstreamtest PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;168;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;187;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/home/lingmo/project/nx_pkg/systeminfo/CMakeLists.txt;15;include;/home/lingmo/project/nx_pkg/systeminfo/CMakeLists.txt;0;")
subdirs("appiumtests")
subdirs("categories")
subdirs("kcontrol/menus")
subdirs("kcms")
subdirs("src")
subdirs("doc")
