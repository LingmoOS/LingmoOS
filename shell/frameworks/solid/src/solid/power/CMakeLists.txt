set(solid_LIB_SRCS
    ${solid_LIB_SRCS}
    power/job.cpp
)
if(WITH_NEW_POWER_ASYNC_API)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/power)
    set(solid_LIB_SRCS
        ${solid_LIB_SRCS}
        power/power.cpp
        power/acpluggedjob.cpp
        power/inhibitionjob.cpp
        power/inhibition.cpp
        power/statesjob.cpp
        power/requeststatejob.cpp
        power/powerbackendloader.cpp

        power/backends/abstractacpluggedjob.h
        power/backends/abstractinhibitionjob.h
        power/backends/abstractinhibition.h
        power/backends/abstractstatesjob.h
        power/backends/abstractrequeststatejob.h
        power/backends/powernotifier.h
        power/backends/dummy/dummyacpluggedjob.cpp
        power/backends/dummy/dummypowernotifier.cpp
        power/backends/dummy/dummyinhibitionjob.cpp
        power/backends/dummy/dummyinhibition.cpp
        power/backends/dummy/dummystatesjob.cpp
        power/backends/dummy/dummyrequeststatejob.cpp
    )
endif()

if(WITH_NEW_POWER_ASYNC_API AND WITH_NEW_POWER_ASYNC_FREEDESKTOP)
    include(power/backends/freedesktop/CMakeLists.txt)
endif()
