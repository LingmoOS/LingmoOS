# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
add_executable(gallery
    norwegianwoodstyle.h
    widgetgallery.h
    main.cpp
    norwegianwoodstyle.cpp
    widgetgallery.cpp
    styles.qrc
)

target_link_libraries(gallery PRIVATE Dtk${VERSION_SUFFIX}::Widget)
set_target_properties(gallery PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
