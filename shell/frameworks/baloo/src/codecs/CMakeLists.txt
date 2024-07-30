set(BALOO_CODECS_SRCS
    doctermscodec.cpp
    positioncodec.cpp
    postingcodec.cpp

    coding.cpp
)

add_library(KF6BalooCodecs STATIC ${BALOO_CODECS_SRCS})
add_library(KF6::BalooCodecs ALIAS KF6BalooCodecs)

set_property(TARGET KF6BalooCodecs PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KF6BalooCodecs
    Qt6::Core
    KF6::CoreAddons
)
