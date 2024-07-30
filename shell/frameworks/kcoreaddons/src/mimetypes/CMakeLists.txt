# always install the mime-types
install(FILES kde6.xml DESTINATION ${KDE_INSTALL_MIMEDIR})

# for KDE frameworks 5 we require at least version 1.8
find_package(SharedMimeInfo 1.8)
set_package_properties(SharedMimeInfo PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Allows KDE applications to determine file types"
)

# update XDG mime-types if shared mime info is around
if(SharedMimeInfo_FOUND)
    update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
endif()
