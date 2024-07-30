find_package(Qt6Test)

set(libs Qt6::Sql Qt6::Test KF6::People KF6::PeopleBackend)

ecm_add_test(TEST_NAME persondatatest persondatatests.cpp fakecontactsource.cpp LINK_LIBRARIES ${libs})
ecm_add_test(TEST_NAME personsmodeltest personsmodeltest.cpp fakecontactsource.cpp LINK_LIBRARIES ${libs})
ecm_add_test(TEST_NAME personsproxymodeltest personsproxymodeltest.cpp fakecontactsource.cpp LINK_LIBRARIES ${libs})
