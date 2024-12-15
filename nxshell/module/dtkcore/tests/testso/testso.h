// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>

namespace TestClass {  // test case use multiple inheritance
struct A
{
    int aa;
    virtual char test(int v);
    virtual ~A() {}
};

struct B
{
    int bx;
    virtual char test(int v)
    {
        qDebug() << Q_FUNC_INFO << v;

        return 'b';
    }

    virtual ~B() {}
};

struct C : public A, public B
{
    int cx;
    char test(int v) override
    {
        qDebug() << Q_FUNC_INFO << v;
        return 'c';
    }
};
}  // namespace TestClass
