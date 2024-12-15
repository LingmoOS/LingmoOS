// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "backgroundocean.h"
#include "stubext.h"

#include <gtest/gtest.h>



DDP_BACKGROUND_BEGIN_NAMESPACE
class UT_backGroundgroundOCEAN : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ocean = new  BackgroundOCEAN;
    }
    virtual void TearDown() override
    {
        delete ocean;
        stub.clear();
    }
    BackgroundOCEAN *ocean = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_backGroundgroundOCEAN, getBackgroundFromOCEAN)
{
    bool call = true;
    ocean->getBackgroundFromOCEAN("temp");
    EXPECT_TRUE(call);
}

TEST_F(UT_backGroundgroundOCEAN, getBackgroundFromConfig)
{
    bool call = false;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open),
                       [] { __DBG_STUB_INVOKE__ return true;});
    QJsonDocument *qjson = new QJsonDocument;
    stub.set_lamda((QJsonDocument(*)(const QByteArray&,QJsonParseError *))(&QJsonDocument::fromJson),
                   [&call,&qjson]( const QByteArray &json, QJsonParseError *error = nullptr){
        __DBG_STUB_INVOKE__
        call = true;
        error->error= QJsonParseError::NoError;

        return *qjson; });

    stub.set_lamda(&QJsonDocument::isArray,[&call](){__DBG_STUB_INVOKE__ call = true;return true;});
    stub.set_lamda(&QJsonArray::size,[](){__DBG_STUB_INVOKE__ return 1;});
    stub.set_lamda(&QJsonValue::isObject,[&call](){__DBG_STUB_INVOKE__ call = true; return true;});
    typedef QString(*FuncType)(void);
    stub.set_lamda((FuncType)(QString(QJsonValue::*)(void)const)(&QJsonValue::toString),[](){__DBG_STUB_INVOKE__ return "index+monitorName";});
    stub.set_lamda(&QJsonValue::isArray,[&call](){__DBG_STUB_INVOKE__ call = true; return true;});
    ocean->getBackgroundFromConfig("temp");
    EXPECT_TRUE(call);
    delete qjson;
}

TEST_F(UT_backGroundgroundOCEAN, background)
{
    bool call = false;
    stub.set_lamda(&BackgroundOCEAN::getBackgroundFromOCEAN,[](){
       __DBG_STUB_INVOKE__
       return "file:/temp";
    });
    auto funType = static_cast<bool(*)(const QString &)>(&QFile::exists);
    stub.set_lamda(funType,[&call](const QString &){
                   __DBG_STUB_INVOKE__
                   call =true;
                   return false;
         });

    EXPECT_EQ(QString("/usr/share/backgrounds/default_background.jpg"), ocean->background("temp"));
    EXPECT_TRUE(call);
    call = false;
    ocean->background("");
    EXPECT_FALSE(call);
}

TEST_F(UT_backGroundgroundOCEAN, getDefaultBackground)
{
    EXPECT_EQ(QString("/usr/share/backgrounds/default_background.jpg"),ocean->getDefaultBackground());
}

DDP_BACKGROUND_END_NAMESPACE
