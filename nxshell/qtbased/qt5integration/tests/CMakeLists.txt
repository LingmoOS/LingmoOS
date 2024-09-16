# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
add_executable(unit-tests
    iconengines/svgiconengine/ut_qsvgiconengine.cpp
    iconengines/svgiconengine/icons.qrc
    imageformats/dci/ut_qdciiohandler.cpp
    imageformats/dci/dci_test_files.qrc
    imageformats/svg/ut_qsvgiohandler.cpp
    styleplugins/chameleon/ut_chameleonstyle.cpp
    styleplugins/chameleon/icons/theme-icons.qrc
    platformtheme/ut_dthemesettings.cpp
    platformtheme/ut_qdeepintheme.cpp
    platformtheme/ut_qdeepinfiledialoghelper.cpp
    platformtheme/test-files.qrc
    platformtheme/filedialogservice.h
    platformtheme/filedialogservice.cpp
    platformtheme/filedialogmanagerservice.h
    platformtheme/filedialogmanagerservice.cpp
    ut_pluginloading.cpp
    main.cpp
)

find_package(GTest REQUIRED)

target_compile_options(unit-tests PRIVATE
    -g
    -fno-access-control
    -Wall
    -fsanitize=address
    -fsanitize-recover=address
    -O0
)

target_link_options(unit-tests PRIVATE
    -fsanitize=address
    -fsanitize-recover=address
)

 target_link_libraries(unit-tests PRIVATE
    GTest::GTest
    GTest::gmock
    dicon
    dsvgicon
    dci
    dsvg
    qdeepin
    chameleon
    asan
)

target_compile_definitions(unit-tests PRIVATE
    DIALOG_SERVICE="org.deepin.fakefilemanager.filedialog"
    META_FILE_BASE_DIR="${CMAKE_SOURCE_DIR}"
    UT_PLATFORMPLUGIN_PATH="${PLUGIN_OUTPUT_BASE_DIR}"
)

target_include_directories(unit-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/iconengineplugins/builtinengine
    ${CMAKE_SOURCE_DIR}/iconengineplugins/dciiconengine
    ${CMAKE_SOURCE_DIR}/iconengineplugins/svgiconengine
    ${CMAKE_SOURCE_DIR}/imageformatplugins/svg
    ${CMAKE_SOURCE_DIR}/imageformatplugins/dci
    ${CMAKE_SOURCE_DIR}/styleplugins/chameleon
    ${CMAKE_SOURCE_DIR}/platformthemeplugin
)
