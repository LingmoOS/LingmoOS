add_library(KF6ThreadWeaver)
add_library(KF6::ThreadWeaver ALIAS KF6ThreadWeaver)

set_target_properties(KF6ThreadWeaver PROPERTIES
    VERSION     ${THREADWEAVER_VERSION}
    SOVERSION   ${THREADWEAVER_SOVERSION}
    EXPORT_NAME ThreadWeaver
)

target_sources(KF6ThreadWeaver PRIVATE
    collection.cpp
    collection.h
    collection_p.cpp
    collection_p.h
    debuggingaids.cpp
    debuggingaids.h
    dependency.cpp
    dependency.h
    dependencypolicy.cpp
    dependencypolicy.h
    destructedstate.cpp
    destructedstate.h
    exception.cpp
    exception.h
    executewrapper.cpp
    executewrapper_p.h
    executor.cpp
    executor_p.h
    iddecorator.cpp
    iddecorator.h
    inconstructionstate.cpp
    inconstructionstate.h
    job.cpp
    job.h
    jobinterface.h
    job_p.cpp
    job_p.h
    jobpointer.h
    lambda.h
    managedjobpointer.h
    qobjectdecorator.cpp
    qobjectdecorator.h
    queueapi.cpp
    queueapi.h
    queue.cpp
    queue.h
    queueing.h
    queueinterface.h
    queuepolicy.h
    queuesignals.cpp
    queuesignals.h
    queuesignals_p.cpp
    queuesignals_p.h
    queuestream.cpp
    queuestream.h
    resourcerestrictionpolicy.cpp
    resourcerestrictionpolicy.h
    sequence.cpp
    sequence.h
    sequence_p.cpp
    sequence_p.h
    shuttingdownstate.cpp
    shuttingdownstate.h
    state.cpp
    state.h
    suspendedstate.cpp
    suspendedstate.h
    suspendingstate.cpp
    suspendingstate.h
    thread.cpp
    thread.h
    threadweaver.cpp
    threadweaver.h
    weaver.cpp
    weaver.h
    weaverimplstate.cpp
    weaverimplstate.h
    weaverinterface.h
    weaver_p.cpp
    weaver_p.h
    workinghardstate.cpp
    workinghardstate.h

)

ecm_generate_export_header(KF6ThreadWeaver
    BASE_NAME ThreadWeaver
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6ThreadWeaver PUBLIC Qt6::Core)
set(threadweaver_BUILD_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(KF6ThreadWeaver PUBLIC "$<BUILD_INTERFACE:${threadweaver_BUILD_INCLUDE_DIRS}>")
target_include_directories(KF6ThreadWeaver INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/ThreadWeaver>")
#FIXME: make this PUBLIC, so that it applies to anything that links against
kde_target_enable_exceptions(KF6ThreadWeaver PRIVATE)

install(TARGETS KF6ThreadWeaver EXPORT KF6ThreadWeaverTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(ThreadWeaver_CamelCase_HEADERS
  HEADER_NAMES
    ThreadWeaver
    Weaver
    WeaverInterface
    QueueAPI
    QueueStream
    Queueing
    Exception
    QueueInterface
    Queue
    DebuggingAids
    Thread
    JobInterface
    Job
    IdDecorator
    QObjectDecorator
    Lambda
    State
    WeaverImplState
    InConstructionState
    WorkingHardState
    SuspendingState
    SuspendedState
    ShuttingDownState
    DestructedState
    Collection
    Sequence
    Dependency
    DependencyPolicy
    ResourceRestrictionPolicy
    QueueSignals
    QueuePolicy
    JobPointer
    ManagedJobPointer

    PREFIX ThreadWeaver
    REQUIRED_HEADERS ThreadWeaver_HEADERS
)
install(FILES ${ThreadWeaver_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/ThreadWeaver/ThreadWeaver COMPONENT Devel)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_export.h
    ${ThreadWeaver_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/ThreadWeaver/threadweaver COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6ThreadWeaver_QCH
        NAME ThreadWeaver
        BASE_NAME KF6ThreadWeaver
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${ThreadWeaver_HEADERS}
            "${CMAKE_SOURCE_DIR}/docs/use-cases.md"
            "${CMAKE_SOURCE_DIR}/docs/whymultithreading.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${threadweaver_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            THREADWEAVER_EXPORT
            THREADWEAVER_DEPRECATED
            THREADWEAVER_DEPRECATED_EXPORT
            "THREADWEAVER_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

