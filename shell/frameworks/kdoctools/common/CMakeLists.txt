# install common files for all languages
foreach( _kdoctoolscommon_currentlang ${KDOCTOOLS_LANGUAGES_LIST})
    file( GLOB _kdoctoolscommon_lang_allfiles "${_kdoctoolscommon_currentlang}/*" )
    install(FILES ${_kdoctoolscommon_lang_allfiles} DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/${_kdoctoolscommon_currentlang}/kdoctools6-common)
endforeach()

# add English version of the -translated files, so that
# help:/kdoctools6-common/gpl-translated.html always works even
# if the translated file for the current language does not exist,
# thanks to kio_help's fallback on the English version.
set (_basetranslatedlicenses fdl gpl lgpl)
foreach (_licenseprefix ${_basetranslatedlicenses})
    install(FILES en/${_licenseprefix}-license.html DESTINATION
            ${KDE_INSTALL_DOCBUNDLEDIR}/en/kdoctools6-common/
            RENAME ${_licenseprefix}-translated.html)
endforeach()

