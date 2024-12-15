# generate qm
execute_process(COMMAND bash "misc/translations/translate_generation.sh"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)
