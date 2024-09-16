// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xzplugin.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_LibXzInterfaceFactory : public QObject, public ::testing::Test
{
public:
    UT_LibXzInterfaceFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibXzInterfaceFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibXzInterfaceFactory *m_tester;
};

class UT_LibXzInterface : public QObject, public ::testing::Test
{
public:
    UT_LibXzInterface(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/xz/test.xz";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibXzInterface(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibXzInterface *m_tester;
};


TEST_F(UT_LibXzInterfaceFactory, initTest)
{

}

TEST_F(UT_LibXzInterface, initTest)
{

}
