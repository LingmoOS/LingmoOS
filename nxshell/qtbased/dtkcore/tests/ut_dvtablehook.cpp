// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "testso.h"
#include <DVtableHook>

class ut_DVtableHook : public testing::Test
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
    virtual ~ut_DVtableHook() {}
};
void ut_DVtableHook::SetUp()
{
}
void ut_DVtableHook::TearDown()
{
}

using namespace TestClass;
DCORE_USE_NAMESPACE

static char test(A *obj, int v)
{
    qDebug() << Q_FUNC_INFO << obj << v;
    return 'x';
}

static char test2(A *obj, int v, bool v2)
{
    qDebug() << Q_FUNC_INFO << obj << v << v2;
    return 'y';
}

TEST_F(ut_DVtableHook, objectFun2ObjectFun)
{
    A *a = new A();
    B *b = new B();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, b, &B::test));
    ASSERT_TRUE(DVtableHook::hasVtable(a));
    ASSERT_EQ(a->test(0), 'b');
    DVtableHook::resetVfptrFun(a, &A::test);
    ASSERT_EQ(a->test(0), 'a');
    delete a;
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
    delete b;
}

TEST_F(ut_DVtableHook, objectFun2Fun)
{
    A *a = new A();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, &test));
    ASSERT_EQ(a->test(1), 'x');
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));

    delete a;
}

TEST_F(ut_DVtableHook, objectFun2StdFun)
{
    A *a = new A();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, std::bind(&test2, std::placeholders::_1, std::placeholders::_2, true)));
    ASSERT_EQ(a->test(2), 'y');
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
    // not support
    //    A *a2 = new A();
    //    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a2, &A::test, std::bind(&test2, std::placeholders::_1, std::placeholders::_2, false)));
    //    ASSERT_TRUE(!a2->test(2));
    //    DVtableHook::resetVtable(a2);
    //    ASSERT_TRUE(!DVtableHook::hasVtable(a2));
    delete a;
}

TEST_F(ut_DVtableHook, objectFun2LambdaFun)
{
    A *a = new A();
    auto lambda1 = [](A *obj, int v) {
        qDebug() << Q_FUNC_INFO << obj << v;
        return '1';
    };
    auto lambda2 = [](A *obj, int v) {
        qDebug() << Q_FUNC_INFO << obj << v;
        return '2';
    };
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, lambda1));
    ASSERT_EQ(a->test(3), '1');
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, lambda2));
    ASSERT_EQ(a->test(3), '2');
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
    delete a;
}

TEST_F(ut_DVtableHook, fun2ObjectFun)
{
    B *b = new B();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, b, &B::test));
    A *a = new A();
    ASSERT_TRUE(DVtableHook::getVtableOfObject(a) == DVtableHook::getVtableOfClass<A>());
    ASSERT_EQ(a->test(4), 'b');
    delete a;
    delete b;
}

TEST_F(ut_DVtableHook, fun2Fun)
{
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, &test));
    A *a = new A();
    ASSERT_EQ(a->test(5), 'x');
    delete a;
}

TEST_F(ut_DVtableHook, fun2StdFun)
{
    A *a = new A();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, std::bind(&test2, std::placeholders::_1, std::placeholders::_2, true)));
    ASSERT_EQ(a->test(6), 'y');
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
    delete a;
}

TEST_F(ut_DVtableHook, fun2LambdaFun)
{
    A *a = new A();
    auto lambda = [](A *obj, int v) {
        qDebug() << Q_FUNC_INFO << obj << v;
        return 'z';
    };
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, lambda));
    ASSERT_EQ(a->test(7), 'z');
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
    delete a;
}

TEST_F(ut_DVtableHook, testRTTI)
{
    A *c1 = new C();
    auto original = typeid(*c1).name();
    auto lambda3 = [](C *obj, int v) {
        qDebug() << Q_FUNC_INFO << obj << v;
        return '3';
    };
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(dynamic_cast<C *>(c1), &C::test, lambda3));
    ASSERT_EQ(c1->test(0), '3');
    ASSERT_EQ(typeid(*c1).name(), original);
    delete c1;
}
