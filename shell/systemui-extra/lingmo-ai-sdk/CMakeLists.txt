cmake_minimum_required(VERSION 3.5)

project(lingmo-ai-base LANGUAGES CXX C)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

include_directories(include)
include_directories(src)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)

find_package(jsoncpp REQUIRED)

include_directories(${GIO_INCLUDE_DIRS})
pkg_check_modules(JSON REQUIRED json-glib-1.0)
include_directories(${GIO_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})
include_directories(include/lingmo-ai/ai-base)
include_directories(include/lingmo-ai)

set(SERVICE_SOURCES
    src/services/textprocessorproxy.cpp
    src/services/textprocessorglue.c
    src/services/speechprocessorproxy.cpp
    src/services/speechprocessorglue.c
    src/services/modelmanagerserviceproxy.cpp
    src/services/modelmanagerserviceglue.c
    src/services/serverproxy.cpp
    src/services/visionprocessorproxy.cpp
    src/services/visionprocessorglue.c
    src/services/datamanagementprocessorglue.c
    src/services/datamanagementprocessorglue.h
    src/services/datamanagementprocessorproxy.cpp
    src/services/datamanagementprocessorproxy.h
    src/services/keywordrecognizerproxy.cpp
    src/services/keywordrecognizerglue.c
    src/services/ocrprocessorproxy.cpp
)

add_library(lingmo-ai-base SHARED
    include/lingmo-ai/ai-base/modelconfig.h
    include/lingmo-ai/ai-base/remotemodelvendor.h
    include/lingmo-ai/ai-base/vision.h
    include/lingmo-ai/ai-base/nlp.h
    include/lingmo-ai/ai-base/speech.h
    include/lingmo-ai/ai-base/ocr.h
    include/lingmo-ai/ai-base/private/datamanagement.h
    include/lingmo-ai/config.h
    src/modelconfig.cpp
    src/nlp.cpp
    src/vision.cpp
    src/visionsession.h
    src/visionsession.cpp
    src/nlpsession.h
    src/nlpsession.cpp
    src/speech.cpp
    src/config.cpp
    src/ocr.cpp
    src/ocrsession.h
    src/ocrsession.cpp
    ${SERVICE_SOURCES}
    src/speechsession.h src/speechsession.cpp
    src/datamanagementsession.h src/datamanagementsession.cpp
    include/lingmo-ai/ai-base/datamanagement.h src/datamanagement.cpp
    src/settings/settings.h src/settings/settings.cpp
    src/settings/globalsettings.h src/settings/globalsettings.cpp
    src/settings/childsettings.h src/settings/childsettings.cpp
    include/lingmo-ai/base.h
    include/lingmo-ai/common.h
)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

set_target_properties(lingmo-ai-base PROPERTIES VERSION 2.0.0 SOVERSION 2)

target_link_libraries(
    lingmo-ai-base
    ${GIO_LIBRARIES}
    jsoncpp
    ${JSON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    tesseract
    leptonica
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS lingmo-ai-base
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/lingmo-ai
    DESTINATION include)

install(FILES pkg-configs/lingmo-ai-base.pc DESTINATION share/pkgconfig/)
install(FILES data/org.lingmo.aisdk.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
