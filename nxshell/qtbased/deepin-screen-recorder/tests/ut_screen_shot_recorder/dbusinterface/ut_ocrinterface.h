// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QStringList>
#include <QVariant>
#include <gtest/gtest.h>
#include <QtDBus/QtDBus>

#include "../../src/dbusinterface/ocrinterface.h"


using namespace testing;

class OcrInterfaceTest:public testing::Test{

public:
    OcrInterface *m_ocrInterface;
    virtual void SetUp() override{
        std::cout << "start OcrInterfaceTest" << std::endl;
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus());
    }

    virtual void TearDown() override{
        if(m_ocrInterface){
            delete  m_ocrInterface;
            m_ocrInterface = nullptr;
        }
        std::cout << "end DBusNotifyTest" << std::endl;
    }
};

TEST_F(OcrInterfaceTest, openFile)
{
    QString filePath = "";
    m_ocrInterface->openFile(filePath);
}
TEST_F(OcrInterfaceTest, openImage)
{
    QImage image;
    m_ocrInterface->openImage(image);
}

TEST_F(OcrInterfaceTest, openImageAndName)
{
    QImage image;
    QString imageName = "";
    m_ocrInterface->openImageAndName(image,imageName);
}

