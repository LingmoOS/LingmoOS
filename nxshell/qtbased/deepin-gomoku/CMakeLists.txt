cmake_minimum_required(VERSION 3.7)
project(deepin-gomoku)

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
# Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,--gc-sections")
endif()

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

ADD_SUBDIRECTORY(gomoku)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
ADD_SUBDIRECTORY(tests)
endif()

find_package(Qt5LinguistTools REQUIRED)
#lupdate start
#此处其实只有当没有自动翻译需要手动翻译.ts文件才有意义可以创建不同语言名称的ts文件，下同
set(TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/deepin-gomoku.ts"
)
find_program(LUPDATE_EXECUTABLE lupdate)
foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file})
endforeach()
#lupdate end

file (GLOB DTNG_TS_FILES translations/*.ts)

#lrelease start
#发布qm文件
find_program(LRELEASE_EXECUTABLE lrelease)
foreach(_ts_file ${DTNG_TS_FILES})
        execute_process(COMMAND ${LRELEASE_EXECUTABLE}   ${_ts_file})
endforeach()
#lrelease end

qt5_create_translation(DTNG_QM_FILES
    ${DTNG_TS_FILES}
     ${DTNG_QM_FILES}
     )

 #根据环境修改/usr 目录
 if(DEFINED ENV{PREFIX})
    set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
 else()
    set(CMAKE_INSTALL_PREFIX /usr)
 endif()

 file(GLOB APP_QM_FILES "translations/*.qm")
 install(FILES ${APP_QM_FILES} DESTINATION share/deepin-gomoku/translations)
