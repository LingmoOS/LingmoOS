# Writes po/<DOMAIN>.pot from the strings of SOURCES and DESKTOP_FILES and
# merges it into every po/<language>.po; see LingmoTranslations.cmake.
string(REPLACE "|" ";" SOURCES "${SOURCES}")
string(REPLACE "|" ";" DESKTOP_FILES "${DESKTOP_FILES}")

set(files "")
foreach(source ${SOURCES})
    if(IS_DIRECTORY ${SOURCE_DIRECTORY}/${source})
        file(GLOB_RECURSE found RELATIVE ${SOURCE_DIRECTORY}
            ${SOURCE_DIRECTORY}/${source}/*.cpp ${SOURCE_DIRECTORY}/${source}/*.h)
        list(APPEND files ${found})
    else()
        list(APPEND files ${source})
    endif()
endforeach()
list(SORT files)
list(APPEND files ${DESKTOP_FILES})
list(REMOVE_DUPLICATES files)

set(po_directory ${SOURCE_DIRECTORY}/po)
set(template ${po_directory}/${DOMAIN}.pot)
set(extracted ${CMAKE_CURRENT_BINARY_DIR}/${DOMAIN}.pot)
file(MAKE_DIRECTORY ${po_directory})
execute_process(
    COMMAND ${XGETTEXT} --from-code=UTF-8 --add-comments=TRANSLATORS: --add-location=file --no-wrap
            --keyword=tr --keyword=trc:1c,2 --keyword=trn:1,2 --keyword=trNoop
            --keyword=dtr:2 --keyword=dtrc:2c,3 --keyword=dtrn:2,3
            --package-name=${DOMAIN} --msgid-bugs-address=https://github.com/LingmoOS
            -o ${extracted} ${files}
    WORKING_DIRECTORY ${SOURCE_DIRECTORY}
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "xgettext failed")
endif()

# Translations come as pull requests: the history names the translators, and
# there are no language teams.
file(READ ${extracted} text)
string(REGEX REPLACE "^(# [^\n]*\n)+#\n" "" text "${text}")
string(REPLACE "charset=CHARSET" "charset=UTF-8" text "${text}")
string(REGEX REPLACE "\"Last-Translator: [^\n]*\n" "\"Last-Translator: \\\\n\"\n" text "${text}")
string(REGEX REPLACE "\"Language-Team: [^\n]*\n" "\"Language-Team: \\\\n\"\n" text "${text}")

# No text goes through printf: a "%" followed by a letter is not a directive.
string(REGEX REPLACE "#, (possible-)?c-format\n" "" text "${text}")
string(REGEX REPLACE ", (possible-)?c-format" "" text "${text}")
string(REGEX REPLACE "#, (possible-)?c-format, " "#, " text "${text}")

# Placeholders are "{name}", which msgfmt checks as python-brace-format.
set(placeholder "\\{[A-Za-z_][A-Za-z0-9_]*\\}")
string(REGEX REPLACE "\n((msgctxt [^\n]*\n)?msgid \"[^\n]*${placeholder}[^\n]*\"\n)"
       "\n#, python-brace-format\n\\1" text "${text}")
string(REGEX REPLACE "\n((msgctxt [^\n]*\n)?msgid \"[^\n]*\"\nmsgid_plural \"[^\n]*${placeholder}[^\n]*\"\n)"
       "\n#, python-brace-format\n\\1" text "${text}")
string(REPLACE "#, python-brace-format\n#, python-brace-format\n" "#, python-brace-format\n" text "${text}")

# Only a change of the strings rewrites the template, not a new date.
set(date_line "\"POT-Creation-Date: [^\n]*\n")
set(previous "")
if(EXISTS ${template})
    file(READ ${template} previous)
endif()
string(REGEX REPLACE "${date_line}" "" new_strings "${text}")
string(REGEX REPLACE "${date_line}" "" old_strings "${previous}")
if(NOT new_strings STREQUAL old_strings)
    file(WRITE ${template} "${text}")
    message(STATUS "Updated ${template}")
endif()

file(GLOB po_files ${po_directory}/*.po)
foreach(po_file ${po_files})
    execute_process(
        COMMAND ${MSGMERGE} --quiet --update --backup=none --no-wrap --add-location=file ${po_file} ${template}
        RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "msgmerge failed on ${po_file}")
    endif()
    file(READ ${po_file} catalog)
    string(REGEX REPLACE "\"Last-Translator: [^\n]*\n" "\"Last-Translator: \\\\n\"\n" cleaned "${catalog}")
    string(REGEX REPLACE "\"Language-Team: [^\n]*\n" "\"Language-Team: \\\\n\"\n" cleaned "${cleaned}")
    if(NOT cleaned STREQUAL catalog)
        file(WRITE ${po_file} "${cleaned}")
    endif()
endforeach()
