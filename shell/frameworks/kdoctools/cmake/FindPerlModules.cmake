#.rst:
# FindPerlModules
# ---------------
#
# Check that Perl has certain modules available.
#
# If PERL_EXECUTABLE is set, uses that, otherwise calls the Perl find module.
#
# To use, pass the perl module names (in the form you would use in a Perl
# ``use`` statement) as components.
#
# This will define the following variables:
#
# ``Perl_<module>_FOUND``
#     True if the given Perl module could be loaded by Perl
#
# where ``<module>`` is either the name passed as a component, or a version
# with ``::`` replaced by ``_``.

# SPDX-FileCopyrightText: 2015 Alex Merry <alex.merry@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

if (NOT PERL_EXECUTABLE)
    find_package(Perl)
endif()

include(FindPackageHandleStandardArgs)

if (PERL_EXECUTABLE)
    set(PerlModules_all_modules_found TRUE)
    foreach(_comp ${PerlModules_FIND_COMPONENTS})
        execute_process(
            COMMAND ${PERL_EXECUTABLE} -e "use ${_comp}"
            RESULT_VARIABLE _result
            OUTPUT_QUIET
            ERROR_QUIET
        )
        string(REPLACE "::" "_" _comp_sanitised "${_comp}")
        if (_result EQUAL 0)
            set(PerlModules_${_comp_sanitised}_FOUND TRUE)
        else()
            set(PerlModules_${_comp_sanitised}_FOUND FALSE)
            set(PerlModules_all_modules_found FALSE)
        endif()
        find_package_handle_standard_args(PerlModules_${_comp}
            FOUND_VAR
                PerlModules_${_comp}_FOUND
            REQUIRED_VARS
                PerlModules_${_comp_sanitised}_FOUND
            )
    endforeach()
endif()

find_package_handle_standard_args(PerlModules
    FOUND_VAR
        PerlModules_FOUND
    REQUIRED_VARS
        PerlModules_all_modules_found
    HANDLE_COMPONENTS
)

include(FeatureSummary)
set_package_properties(PerlModules PROPERTIES
    URL "https://www.cpan.org"
)

