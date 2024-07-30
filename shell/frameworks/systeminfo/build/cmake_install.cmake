# Install script for directory: /home/lingmo/project/nx_pkg/systeminfo

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/appiumtests/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/categories/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/kcontrol/menus/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/kcms/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/src/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/lingmo/project/nx_pkg/systeminfo/build/doc/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/ca/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/ca/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/ca/docs/kinfocenter/index.docbook"
    "/home/lingmo/project/nx_pkg/systeminfo/po/ca/docs/kinfocenter/kinfocenter.png"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/es/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/es/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/es/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/fr/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/fr/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/fr/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/id/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/id/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/id/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/it/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/it/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/it/docs/kinfocenter/index.docbook"
    "/home/lingmo/project/nx_pkg/systeminfo/po/it/docs/kinfocenter/kinfocenter.png"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/nl/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/nl/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/nl/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/pt/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/pt/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/pt/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/pt_BR/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/pt_BR/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/pt_BR/docs/kinfocenter/index.docbook"
    "/home/lingmo/project/nx_pkg/systeminfo/po/pt_BR/docs/kinfocenter/kinfocenter.png"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/ru/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/ru/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/ru/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/sr/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/sr/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/sr/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/sr@latin/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/sr@latin/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/sr@latin/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/sv/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/sv/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/sv/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/tr/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/tr/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/tr/docs/kinfocenter/index.docbook"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/uk/kinfocenter" TYPE FILE FILES
    "/home/lingmo/project/nx_pkg/systeminfo/build/po/uk/docs/kinfocenter/index.cache.bz2"
    "/home/lingmo/project/nx_pkg/systeminfo/po/uk/docs/kinfocenter/index.docbook"
    "/home/lingmo/project/nx_pkg/systeminfo/po/uk/docs/kinfocenter/kinfocenter.png"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/metainfo" TYPE FILE FILES "/home/lingmo/project/nx_pkg/systeminfo/org.kde.kinfocenter.appdata.xml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" TYPE PROGRAM FILES "/home/lingmo/project/nx_pkg/systeminfo/org.kde.kinfocenter.desktop")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/bin/kinfocenter")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/bin" TYPE FILE FILES "/home/lingmo/project/nx_pkg/systeminfo/build/kinfocenter")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kinfocenter" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share" TYPE DIRECTORY FILES "/home/lingmo/project/nx_pkg/systeminfo/build/locale")
endif()

if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
  file(WRITE "/home/lingmo/project/nx_pkg/systeminfo/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
