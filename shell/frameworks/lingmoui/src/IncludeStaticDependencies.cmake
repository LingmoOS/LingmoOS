# This function is mainly a workaround for QTBUG-114949
#
# qt_add_qml_module will create a separate object target for each resource
# added to a module when doing a static build. These are then set as
# dependencies of the plugin target. This breaks installation since CMake
# requires these targets to be exported along with the rest of the library.
# However, just exporting them does not help as Qt includes these objects in
# a somewhat roundabout way that breaks for exported targets. The solution
# to all of this mess is to manually include all the target objects from the
# separate object targets, then wipe the link interface so there are no
# further references to these targets. We can then recreate the link
# interface.
function(include_static_dependencies ARG_TARGET)
    if (BUILD_SHARED_LIBS)
        return()
    endif()

    if (TARGET ${ARG_TARGET}_qmlcache)
        target_sources(${ARG_TARGET} PRIVATE $<TARGET_OBJECTS:${ARG_TARGET}_qmlcache>)
    endif()

    # This assumes the target uses the default naming for the plugin target.
    target_sources(${ARG_TARGET}plugin PRIVATE $<TARGET_OBJECTS:${ARG_TARGET}plugin_init>)

    # There is not really an easy way to get which resource targets the target
    # depends on. Luckily they follow a simple naming scheme so we can just try
    # and find these targets by iterating.
    foreach(_index RANGE 100)
        if (TARGET ${ARG_TARGET}_resources_${_index})
            target_sources(${ARG_TARGET} PRIVATE $<TARGET_OBJECTS:${ARG_TARGET}_resources_${_index}>)
        endif()
    endforeach()

    # Wipe the link interfaces of the target and the plugin target so they no
    # longer contain any references to the object targets.
    set_target_properties(${ARG_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "" INTERFACE_LINK_OPTIONS "" INTERFACE_SOURCES "")
    set_target_properties(${ARG_TARGET}plugin PROPERTIES INTERFACE_LINK_LIBRARIES "" INTERFACE_LINK_OPTIONS "" INTERFACE_SOURCES "" LINK_LIBRARIES "")

    # Recreate the link interface for the plugin target.
    # Note that we require WHOLE_ARCHIVE as otherwise the resources coming from
    # the object targets cannot be found by Qt as there is nothing using them
    # so the compiler discards them.
    target_link_libraries(${ARG_TARGET}plugin
        PUBLIC
        Qt6::Qml
        $<BUILD_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,${ARG_TARGET}>>
        $<INSTALL_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,KF6::${ARG_TARGET}>>
    )
endfunction()
