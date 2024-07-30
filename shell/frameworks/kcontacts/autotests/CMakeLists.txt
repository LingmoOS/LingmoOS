include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG REQUIRED)

########### next target ###############

add_executable(testroundtrip)
target_sources(testroundtrip PRIVATE testroundtrip.cpp testroundtrip.qrc)
add_test(NAME kcontacts-testroundtrip COMMAND testroundtrip)
target_link_libraries(testroundtrip KF6Contacts Qt6::Test)

ecm_add_tests(birthdaytest.cpp
    addresseetest.cpp
    impptest.cpp
    emailtest.cpp
    addresstest.cpp
    geotest.cpp
    keytest.cpp
    phonenumbertest.cpp
    secrecytest.cpp
    soundtest.cpp
    timezonetest.cpp
    contactgrouptest.cpp
    vcarddragtest.cpp
    langtest.cpp gendertest.cpp
    calendarurltest.cpp
    ldifconvertertest.cpp
    relatedtest.cpp
    resourcelocatorurltest.cpp
    customidentifiertest.cpp
    vcardlinetest.cpp
    fieldgrouptest.cpp
    importexportvcardtest.cpp
    titletest.cpp
    nicknametest.cpp
    roletest.cpp
    orgtest.cpp
    datetimetest.cpp
    clientpidmaptest.cpp
    NAME_PREFIX "kcontacts-" LINK_LIBRARIES KF6Contacts Qt6::Test KF6::ConfigCore)

ecm_add_test(addressformattest.cpp ../src/kcontacts.qrc
    TEST_NAME addressformattest
    NAME_PREFIX "kcontacts-" LINK_LIBRARIES KF6Contacts KF6::I18nLocaleData KF6::ConfigCore Qt6::Test)

ecm_add_tests(picturetest.cpp
    NAME_PREFIX "kcontacts-" LINK_LIBRARIES KF6Contacts Qt6::Test KF6::ConfigCore Qt6::Gui)

# Benchmark, compiled, but not run automatically with ctest
add_executable(vcardtool_benchmark vcardtool_benchmark.cpp)
target_link_libraries(vcardtool_benchmark KF6Contacts Qt6::Test KF6::ConfigCore)
