// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "service/ocrinterface.h"

TEST_F(gtestview, ocr_openImage)
{
    OcrInterface *m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), nullptr);
    QImage img(QApplication::applicationDirPath() + "/test/jpg.jpg");
    m_ocrInterface->openImage(img);
    bool bRet = false;
    if (!img.isNull()) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);
    m_ocrInterface->deleteLater();
    m_ocrInterface = nullptr;
}

TEST_F(gtestview, ocr_openImageAndName)
{
    OcrInterface *m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), nullptr);
    QImage img(QApplication::applicationDirPath() + "/test/jpg.jpg");
    m_ocrInterface->openImageAndName(img, QApplication::applicationDirPath() + "/test/jpg.jpg");
    bool bRet = false;
    if (!img.isNull()) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);
    m_ocrInterface->deleteLater();
    m_ocrInterface = nullptr;
}
