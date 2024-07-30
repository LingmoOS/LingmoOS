# check presence of installed files
set(MANPATH ${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_MANDIR})
set(HTMLPATH ${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_DOCBUNDLEDIR})
set(FILES
# from kdoctools_install
    ${MANPATH}/man1/checkXML6.1
    ${MANPATH}/man1/meinproc6.1
    ${MANPATH}/man7/kf6options.7
    ${MANPATH}/man7/qt6options.7
# from add_subdirectory
    ${MANPATH}/es/man1/kjscmd.1
    ${MANPATH}/fr/man1/kjscmd.1
    ${HTMLPATH}/fr/foobar/index.cache.bz2
    ${HTMLPATH}/es/foobar/index.cache.bz2
    ${HTMLPATH}/es/kioslave5/fooslave/index.cache.bz2
)

foreach(f ${FILES})
    if(NOT EXISTS ${f})
        message(SEND_ERROR "${f} was not found")
    else()
        message(STATUS "found installed file ${f}")
    endif()
endforeach()
