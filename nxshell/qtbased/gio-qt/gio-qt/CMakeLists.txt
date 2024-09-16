# Populate a CMake variable with the sources
# TODO: portable headers?

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIOMM giomm-2.4) # look into FindPkgConfig.cmake

set (QGIO_PUBLIC_HEADER_FILES
    include/dglibutils.h
    include/dgiovolumemanager.h
    include/dgiomountoperation.h
    include/dgiomount.h
    include/dgiovolume.h
    include/dgiofile.h
    include/dgiofileinfo.h
    include/dgiofileiterator.h
    include/dgiodrive.h
    include/dgiosettings.h
)

set (QGIO_PRIVATE_HEADER_FILES
    private/dgiohelper.h
)

set (QGIO_PRIVATE_CPP_FILES
    source/dglibutils.cpp
    source/dgiovolumemanager.cpp
    source/dgiomountoperation.cpp
    source/dgiomount.cpp
    source/dgiovolume.cpp
    source/dgiofile.cpp
    source/dgiofileinfo.cpp
    source/dgiofileiterator.cpp
    source/dgiodrive.cpp
    source/dgiosettings.cpp
    private/dgiohelper.cpp
)

include(qt6.cmake)
include(qt5.cmake)

install (FILES ${QGIO_PUBLIC_HEADER_FILES}
    DESTINATION include/gio-qt
)
