// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>
#include <DAppLoader>

DQUICK_USE_NAMESPACE

static int qArgc = 0;
static char **qArgv = nullptr;

class tst_DAppLoader : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        qDebug() << "*****************" << __FUNCTION__;
    }
    static void TearDownTestCase()
    {
        qDebug() << "*****************" << __FUNCTION__;
    }
    virtual void SetUp();
    virtual void TearDown();
};
void tst_DAppLoader::SetUp()
{
    qDebug() << "*****************" << __FUNCTION__;
}
void tst_DAppLoader::TearDown()
{
    qDebug() << "*****************" << __FUNCTION__;
}

TEST_F(tst_DAppLoader, DAppLoader_createApplication)
{
    qDebug() << "Lib Name: " << LIB_NAME
             << endl << "Lib Path: " << LIB_PATH;

    // 通过 libdtkdeclarative-dev 开发包中提供的 DAppLoader 类，解析本项目生成的 lib 文件。
    // 由于开发调试时，本项目生成的 lib 是被 exe 直接链接的，所以需要测试实际场景下(使用 DAppLoader 加载库)的使用方式是否正常。
    DAppLoader appLoader(LIB_NAME, LIB_PATH);
    QGuiApplication *app = appLoader.createApplication(qArgc , qArgv);

    ASSERT_TRUE(app);
}

int main(int argc, char *argv[])
{
    qArgc = argc;
    qArgv = argv;

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
