# SPDX-FileCopyrightText: 2017-2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

set(aztecbarcodetest_srcs
    aztecbarcodetest.cpp
    aztec/aztec.qrc
    ../src/lib/aztecbarcode.cpp
    ../src/lib/abstractbarcode_p.cpp
    ../src/lib/barcodeutil.cpp
    ../src/lib/bitvector.cpp
    ../src/lib/reedsolomon.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../src/lib/prison_debug.cpp
)

ecm_add_test(${aztecbarcodetest_srcs} TEST_NAME prison-aztecbarcodetest LINK_LIBRARIES Qt6::Test KF6::Prison)

ecm_add_test(
    reedsolomontest.cpp
    ../src/lib/bitvector.cpp
    ../src/lib/reedsolomon.cpp
    TEST_NAME prison-reedsolomontest
    LINK_LIBRARIES Qt6::Test KF6::Prison
)

set(code128barcodetest_srcs
    code128barcodetest.cpp
    code128/code128.qrc
    ../src/lib/abstractbarcode_p.cpp
    ../src/lib/barcodeutil.cpp
    ../src/lib/code128barcode.cpp
    ../src/lib/bitvector.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../src/lib/prison_debug.cpp
)

ecm_add_test(${code128barcodetest_srcs} TEST_NAME prison-code128barcodetest LINK_LIBRARIES Qt6::Test KF6::Prison)

if(TARGET Dmtx::Dmtx)
    ecm_add_test(datamatrixtest.cpp datamatrix/datamatrix.qrc TEST_NAME prison-datamatrixtest LINK_LIBRARIES Qt6::Test KF6::Prison)
endif()
ecm_add_test(qrtest.cpp qr/qr.qrc TEST_NAME prison-qrtest LINK_LIBRARIES Qt6::Test KF6::Prison)

if (TARGET ZXing::ZXing)
    ecm_add_test(zxingutiltest.cpp ${PROJECT_SOURCE_DIR}/src/lib/zxingutil.cpp TEST_NAME prison-zxingutiltest LINK_LIBRARIES Qt6::Test ZXing::ZXing KF6::Prison)
    kde_source_files_enable_exceptions(${PROJECT_SOURCE_DIR}/src/lib/zxingutil.cpp )
    ecm_add_test(imagescannertest.cpp qr/qr.qrc TEST_NAME prison-imagescannertest LINK_LIBRARIES Qt6::Test Qt6::Concurrent KF6::PrisonScanner)
endif()

ecm_add_test(
    mecardtest.cpp
    TEST_NAME prison-mecardtest
    LINK_LIBRARIES Qt6::Test KF6::Prison
)
