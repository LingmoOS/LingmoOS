// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_arrowrectangle.h"
#include "../../src/widgets/arrowrectangle.h"
#include "../../src/views/memorylistwidget.h"


Ut_ArrowRectangle::Ut_ArrowRectangle()
{

}

TEST_F(Ut_ArrowRectangle, eventFilter)
{
    ArrowRectangle *arrowrectangle = new ArrowRectangle(DArrowRectangle::ArrowTop, DArrowRectangle::FloatWidget);
    QFocusEvent *f = new QFocusEvent(QFocusEvent::Type::FocusOut);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::KeyboardModifier::NoModifier);
    arrowrectangle->eventFilter(arrowrectangle, static_cast <QEvent *>(k));
    EXPECT_TRUE(arrowrectangle->isHidden());
    arrowrectangle->eventFilter(arrowrectangle, static_cast <QEvent *>(f));
    EXPECT_TRUE(arrowrectangle->isHidden());
    delete k;
    delete f;
    delete arrowrectangle;
}

TEST_F(Ut_ArrowRectangle, focusInEvent)
{
    ArrowRectangle *arrowrectangle = new ArrowRectangle(DArrowRectangle::ArrowTop, DArrowRectangle::FloatWidget);
    MemoryListWidget *list = new MemoryListWidget();
    arrowrectangle->setContent(list);
    QFocusEvent *f = new QFocusEvent(QFocusEvent::Type::FocusIn, Qt::TabFocusReason);
    arrowrectangle->focusInEvent(f);
    EXPECT_FALSE(static_cast<MemoryListWidget *>(arrowrectangle->getContent())->hasFocus());
    delete f;
    delete list;
    delete arrowrectangle;
}

TEST_F(Ut_ArrowRectangle, keyPressEvent)
{
    ArrowRectangle *arrowrectangle = new ArrowRectangle(DArrowRectangle::ArrowTop, DArrowRectangle::FloatWidget);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    arrowrectangle->keyPressEvent(k);
    delete k;
    delete arrowrectangle;
    //屏蔽keyPressEvent，无assert
}
