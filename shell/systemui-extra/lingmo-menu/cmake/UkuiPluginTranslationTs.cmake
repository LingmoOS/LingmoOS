macro(lingmo_plugin_translate_ts PLUGIN)
set(TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translation/${PLUGIN}_zh_CN.ts)
set(BO_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translation/${PLUGIN}_bo_CN.ts)
set(B_QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/translation/)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/translation/)
else()
    execute_process(
        COMMAND mkdir ${CMAKE_CURRENT_SOURCE_DIR}/translation/
        )
endif()

if(EXISTS ${TS_FILES})
    message(STATUS "${TS_FILES} is EXISTS")
    execute_process(
        COMMAND lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -target-language zh_CN -ts ${TS_FILES}
        )
    execute_process(
        COMMAND lrelease ${TS_FILES}
        )
else()
    execute_process(
        COMMAND lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -target-language zh_CN -ts ${TS_FILES}
        )
    execute_process(
        COMMAND lrelease ${TS_FILES}
        )
endif()

if(EXISTS ${BO_TS_FILES})
    message(STATUS "${BO_TS_FILES} is EXISTS")
    execute_process(
        COMMAND lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -target-language bo_CN -ts ${BO_TS_FILES}
        )
    execute_process(
        COMMAND lrelease ${BO_TS_FILES}
        )
else()
    execute_process(
        COMMAND lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -target-language bo_CN -ts ${BO_TS_FILES}
        )
    execute_process(
        COMMAND lrelease ${BO_TS_FILES}
        )
endif()


if(EXISTS ${B_QM_FILES})
   message(STATUS "${PLUGIN} buildQM dir is EXISTS")
else()
   message(STATUS "${PLUGIN} buildQM dir is not EXISTS")
   execute_process(
       COMMAND mkdir ${B_QM_FILES}
       )
   message(STATUS "${PLUGIN} buildQM dir is created")
endif()

set(P_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translation/${PLUGIN}_zh_CN.qm)
set(BO_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translation/${PLUGIN}_bo_CN.qm)

if(EXISTS ${P_QM_FILES})
   message(STATUS "${PLUGIN} proQM file is EXISTS")
   execute_process(
       COMMAND cp -f ${P_QM_FILES} ${B_QM_FILES}
       )
   execute_process(
       COMMAND rm -f ${P_QM_FILES}
       )
   message(STATUS "${PLUGIN} buildQM file is created")
else()
   message(STATUS "${PLUGIN} buildQM file is not EXISTS")
endif()

if(EXISTS ${BO_QM_FILES})
   message(STATUS "${PLUGIN} proQM file is EXISTS")
   execute_process(
       COMMAND cp -f ${BO_QM_FILES} ${B_QM_FILES}
       )
   execute_process(
       COMMAND rm -f ${BO_QM_FILES}
       )
   message(STATUS "${PLUGIN} buildQM file is created")
else()
   message(STATUS "${PLUGIN} buildQM file is not EXISTS")
endif()

if(${PLUGIN} STREQUAL "panel")
    set(P_QM_INSTALL ${PACKAGE_DATA_DIR}/${PLUGIN}/translation)
    message(STATUS " panel translation install   : ${P_QM_INSTALL}")
else()
    set(P_QM_INSTALL ${PACKAGE_DATA_DIR}/plugin-${PLUGIN}/translation)
    message(STATUS " plugin ${PLUGIN} translation install   : ${P_QM_INSTALL}")
endif()

install(DIRECTORY ${B_QM_FILES}
        DESTINATION ${P_QM_INSTALL})

ADD_DEFINITIONS(-DQM_INSTALL=\"${P_QM_INSTALL}/${PLUGIN}_zh_CN.qm\")
ADD_DEFINITIONS(-DBO_QM_INSTALL=\"${P_QM_INSTALL}/${PLUGIN}_bo_CN.qm\")
ADD_DEFINITIONS(-DPLUGINNAME=\"${PLUGIN}\")
endmacro()
