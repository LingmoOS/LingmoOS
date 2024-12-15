// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gzplugin.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_LibGzipInterfaceFactory : public QObject, public ::testing::Test
{
public:
    UT_LibGzipInterfaceFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibGzipInterfaceFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibGzipInterfaceFactory *m_tester;
};

class UT_LibGzipInterface : public QObject, public ::testing::Test
{
public:
    UT_LibGzipInterface(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/gz/test.gz";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibGzipInterface(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibGzipInterface *m_tester;
};


TEST_F(UT_LibGzipInterfaceFactory, initTest)
{

}

TEST_F(UT_LibGzipInterface, initTest)
{

}
