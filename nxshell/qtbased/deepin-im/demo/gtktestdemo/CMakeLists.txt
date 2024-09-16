# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(GTK_TEST_DEMO_SOURCES
  main.cpp
)

add_executable(gtk_im_demo
  ${GTK_TEST_DEMO_SOURCES}
)

target_link_libraries(gtk_im_demo
  PkgConfig::Gtk4
)

install(TARGETS gtk_im_demo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


