// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "ddciiconpalette.h"
#include <QDebug>
#include <QMetaProperty>
#include <QPalette>

DGUI_USE_NAMESPACE

static QVariant property(const DDciIconPalette &pa, const char *name)
{
    const QMetaObject *meta = &pa.staticMetaObject;
    int id = meta->indexOfProperty(name);
    if (id < 0)
        return QVariant();

    QMetaProperty p = meta->property(id);
    if (!p.isReadable())
        qWarning("%s::property: Property \"%s\" invalid or does not exist",
                 meta->className(), name);

    return p.readOnGadget(&pa);

}

static bool setProperty(DDciIconPalette &pa, const char *name, const QVariant &var)
{
    const QMetaObject *meta = &pa.staticMetaObject;
    int id = meta->indexOfProperty(name);
    if (id < 0) {
        qWarning("%s::setProperty: Property \"%s\" invalid,"
                 "not support dynamic property..", meta->className(), name);
        return false;
    }

    QMetaProperty p = meta->property(id);
    if (!p.isWritable())
        qWarning("%s::setProperty: Property \"%s\" invalid,"
                 " read-only or does not exist", meta->className(), name);

    return p.writeOnGadget(&pa, var);
}

TEST(ut_DDciIconPalette, color)
{
    DDciIconPalette pa(Qt::red, Qt::green, Qt::black, Qt::white);

#define testColor(role, setFunc,color1, color2)  do { \
    Q_ASSERT(color1!=color2); \
    ASSERT_EQ(pa.role(), color1); \
    ASSERT_EQ(property(pa, QT_STRINGIFY(role)), color1); \
    pa.setFunc(color2); \
    ASSERT_EQ(pa.role(), color2); \
    ASSERT_EQ(property(pa, QT_STRINGIFY(role)), color2); \
    setProperty(pa, #role, QColor(color1)); \
    ASSERT_EQ(pa.role(), color1); \
    ASSERT_EQ(property(pa, QT_STRINGIFY(role)), color1); \
} while(false);

    testColor(foreground, setForeground, QColor(Qt::red), QColor(Qt::green));
    testColor(background, setBackground, QColor(Qt::green), QColor(Qt::red));

    testColor(highlight, setHighlight,QColor(Qt::black), QColor(Qt::white));
    testColor(highlightForeground, setHighlightForeground, QColor(Qt::white), QColor(Qt::black));
}

TEST(ut_DDciIconPalette, convertToString_convertFromString_operator)
{
    DDciIconPalette pa(Qt::red, Qt::green, Qt::black, Qt::white);

    // convertToString
    QString str = DDciIconPalette::convertToString(pa);

    // convertFromString
    auto ap = DDciIconPalette::convertFromString(str);

    // operator != && ==
    ASSERT_FALSE(pa != ap);
}

TEST(ut_DDciIconPalette, fromQPalette)
{
    DDciIconPalette pa(Qt::red, Qt::green, Qt::black, Qt::white);

    QPalette qpa;
    qpa.setColor(QPalette::WindowText, Qt::red);
    qpa.setColor(QPalette::Window, Qt::green);
    qpa.setColor(QPalette::Highlight, Qt::black);
    qpa.setColor(QPalette::HighlightedText, Qt::white);

     ASSERT_EQ(pa, DDciIconPalette::fromQPalette(qpa));
}
