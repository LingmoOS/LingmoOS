find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
kde_enable_exceptions()

include(ECMAddTests)

########### next target ###############

ecm_add_test(
    JobTests.cpp
    AppendCharacterAndVerifyJob.cpp
    WaitForIdleAndFinished.cpp
    JobLoggingWeaver.cpp
    JobLoggingDecorator.cpp
    TEST_NAME ThreadWeaverJobTests LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver
)
set_target_properties(ThreadWeaverJobTests PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(SequencesTests.cpp WaitForIdleAndFinished.cpp TEST_NAME ThreadWeaverSequencesTests LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
set_target_properties(ThreadWeaverSequencesTests PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(QueueTests.cpp TEST_NAME ThreadWeaverQueueTests LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
set_target_properties(ThreadWeaverQueueTests PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(DeleteTest.cpp TEST_NAME ThreadWeaverDeleteTest LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
set_target_properties(ThreadWeaverDeleteTest PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(LifecycleTests.cpp TEST_NAME ThreadWeaverLifecycleTest LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
set_target_properties(ThreadWeaverLifecycleTest PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(ShutdownOnQApplicationQuitTests.cpp TEST_NAME ThreadWeaverShutdownOnQApplicationQuitTest LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
set_target_properties(ThreadWeaverShutdownOnQApplicationQuitTest PROPERTIES MACOSX_BUNDLE FALSE)

########### next target ###############

ecm_add_test(QueueFactoryTests.cpp TEST_NAME ThreadWeaverQueueFactoryTest LINK_LIBRARIES Qt6::Test KF6::ThreadWeaver)
Set_target_properties(ThreadWeaverQueueFactoryTest PROPERTIES MACOSX_BUNDLE FALSE)
