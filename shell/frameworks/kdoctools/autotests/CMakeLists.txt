add_test(
  NAME meinproc_template
  COMMAND meinproc6 --srcdir ${CMAKE_CURRENT_BINARY_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../src/template.docbook
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set(CTEST_ENVIRONMENT "QT_LOGGING_RULES=kf.doctools.core=true")

if (TARGET Qt6::Core)
    set(QT_VERSION_OPT "-DBUILD_WITH_QT6=ON")
else()
    set(QT_VERSION_OPT "-DBUILD_WITH_QT6=OFF")
endif()

add_test(kdoctools_install ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/kdoctools+install"
    "${CMAKE_CURRENT_BINARY_DIR}/kdoctools+install"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-target install
    --build-options
        "-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/kdoctools+install/destdir"
        "-DKF6DocTools_DIR=${CMAKE_BINARY_DIR}/cmake"
        "-DKDocTools_BINARY_DIR=${CMAKE_BINARY_DIR}"
        "-DKDOCTOOLS_CUSTOMIZATION_DIR=${KDOCTOOLS_CUSTOMIZATION_DIR}"
        "-DKDOCTOOLS_ENABLE_HTMLHANDBOOK=1"
        "-DCMAKE_VERBOSE_MAKEFILE=ON"
        "-D_kdoctoolsTestsRunning=1"
        ${QT_VERSION_OPT}
    --test-command ${CMAKE_COMMAND}
        "-DKDE_INSTALL_MANDIR=${KDE_INSTALL_MANDIR}"
        "-DKDE_INSTALL_DOCBUNDLEDIR=${KDE_INSTALL_DOCBUNDLEDIR}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/kdoctools+install/destdir"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/kdoctools+install/test.cmake")
