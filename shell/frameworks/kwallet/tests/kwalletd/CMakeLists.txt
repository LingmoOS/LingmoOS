include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../src/api/KWallet ) # for kwallet.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../../src/api/KWallet )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../src/runtime/kwalletd ) # kwalletwizard.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../../src/runtime/kwalletd/backend ) # kwalletwizard.h

include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(Gpgmepp) # provided by GpgME

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_executable(kwalletwizardtest)
set(kwalletwizardtest_ui_SRCS)
qt_wrap_ui(kwalletwizardtest_ui_SRCS
   ../../src/runtime/kwalletd/kbetterthankdialogbase.ui
   ../../src/runtime/kwalletd/kwalletwizardpageexplanation.ui
   ../../src/runtime/kwalletd/kwalletwizardpageintro.ui
   ../../src/runtime/kwalletd/kwalletwizardpageoptions.ui
   ../../src/runtime/kwalletd/kwalletwizardpagepassword.ui
   ../../src/runtime/kwalletd/kwalletwizardpagegpgkey.ui
)

if (Gpgmepp_FOUND)
    qt_wrap_ui(kwalletwizardtest_ui_SRCS
        ../../src/runtime/kwalletd/kwalletwizardpagepasswordgpg.ui)
endif(Gpgmepp_FOUND)

target_sources(kwalletwizardtest PRIVATE
   kwalletwizardtest.cpp
   ../../src/runtime/kwalletd/kwalletwizard.cpp
   ${kwalletwizardtest_ui_SRCS}
)
ecm_mark_as_test(kwalletwizardtest)
target_link_libraries(kwalletwizardtest
    KF6Wallet
    Qt6::Test
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons)
if (Gpgmepp_FOUND)
    target_link_libraries(kwalletwizardtest Gpgmepp)
endif(Gpgmepp_FOUND)

add_library(kwallettestlib STATIC kwallettest.cpp)
target_link_libraries(kwallettestlib KF6Wallet Qt6::Widgets)

macro(kwalletd_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname}
        kwallettestlib
        KF6WalletBackend
        KF6Wallet
        Qt6::DBus
        Qt6::Test
        KF6::CoreAddons
        KF6::I18n)
  endforeach(_testname)
endmacro()

kwalletd_tests(
    kwalletasync
    kwalletsync
    kwalletpath
    kwalletboth
    kwalletmany
    kwalletcbc
)


