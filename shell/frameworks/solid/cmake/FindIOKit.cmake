# - Find IOKit on Mac
#
#  IOKIT_LIBRARY - the library to use IOKit
#  IOKIT_FOUND - true if IOKit has been found

# SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
#
# SPDX-License-Identifier: BSD-3-Clause

include(CMakeFindFrameworks)

cmake_find_frameworks(IOKit)
cmake_find_frameworks(CoreFoundation)

if (IOKit_FRAMEWORKS)
   set(IOKIT_LIBRARY "-framework IOKit -framework CoreFoundation" CACHE FILEPATH "IOKit framework" FORCE)
   set(IOKIT_FOUND 1)
endif (IOKit_FRAMEWORKS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IOKit DEFAULT_MSG IOKIT_LIBRARY)
