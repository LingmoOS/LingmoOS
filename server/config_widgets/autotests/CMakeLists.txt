
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(kstandardactiontest.cpp LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)

set(kconfigdialog_unittest_SRCS kconfigdialog_unittest.cpp)
kconfig_add_kcfg_files(kconfigdialog_unittest_SRCS GENERATE_MOC signaltest.kcfgc)
ecm_add_test(${kconfigdialog_unittest_SRCS} TEST_NAME "kconfigdialog_unittest" LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)

set(lang_entries
  ca
  de
  en_US
  es # must not have file!
  fr
  pt
)

# scripty would mangle all *.desktop files, since that'd have potential of
# breaking the test we'll need to bypass scripty by not having our files called
# .desktop!
# Do note that we pop these into CMAKE_LIBRARY_OUTPUT_DIRECTORY so QFINDTESTDATA
# is able to find the fixtures in the bin dir as KDECMakeSettings sets a special
# output dir.
foreach(lang ${lang_entries})
    set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/kf5_entry_data.cmake/locale/${lang}")
    set(src_file "${src_dir}/kf5_entry.cmake")
    set(bin_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/kf5_entry_data/locale/${lang}")
    set(bin_file "${bin_dir}/kf5_entry.desktop")
    file(MAKE_DIRECTORY ${bin_dir})
    if(EXISTS ${src_file}) # not all languages have entries
        configure_file(${src_file} ${bin_file} COPYONLY)
    endif()
endforeach()

ecm_add_test(klanguagenametest.cpp LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)
ecm_add_test(kcolorschemetest.cpp LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)
ecm_add_test(kcmdbartest.cpp LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)

ecm_add_test(krecentfilesactiontest.cpp TEST_NAME "krecentfilesaction_test" LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)
