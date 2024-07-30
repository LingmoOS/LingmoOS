# Encode an URI according to RFC 2396
# kdoctools_encode_uri takes a variable name and it encodes
# its value according to RFC 2396 (minus few reserved characters)
# overwriting the original value.
#
# SPDX-FileCopyrightText: 2014 Luigi Toscano <luigi.toscano@tiscali.it>
#
# SPDX-License-Identifier: BSD-3-Clause

function(kdoctools_encode_uri _original_uri)
    find_package(Perl REQUIRED)
    find_package(PerlModules COMPONENTS URI::Escape REQUIRED)
    # properly encode the URI
    string(REPLACE "\"" "\\\"" escaped_uri "${${_original_uri}}")
    execute_process(COMMAND ${PERL_EXECUTABLE} -MURI::Escape -e "print uri_escape_utf8(\"${escaped_uri}\", \"^A-Za-z0-9\\-\\._~\\/:\");"
                    OUTPUT_VARIABLE encoded_uri
                    RESULT_VARIABLE res_encoding
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT ${res_encoding} EQUAL 0)
        message(FATAL_ERROR "Problem while encoding the URI ${${_original_uri}}")
    endif()
    set(${_original_uri} "${encoded_uri}" PARENT_SCOPE)
endfunction()
