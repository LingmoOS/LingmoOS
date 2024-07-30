#
# The main documentation is written using asciidoc
# So, the files *.adoc are the documentation sources
# These are converted to docbook format using the a2x
# tool that commes with the asciidoc package
#

if(BUILD_KWALLET_QUERY)
    set(m kwallet-query)
    kdoctools_create_manpage(man-${m}.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
endif()
