// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_ANIMATIONSTACKEDWIDGET_H
#define TEST_ANIMATIONSTACKEDWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/animationstackedwidget.h"

class test_animationstackedwidget: public::QObject, public::testing::Test
{
public:
    test_animationstackedwidget();
    ~test_animationstackedwidget();

    virtual void SetUp()
    {
        mAnimationStackedWidget = new AnimationStackedWidget();
        widget1 = new QWidget(mAnimationStackedWidget);
        widget2 = new QWidget(mAnimationStackedWidget);
        mAnimationStackedWidget->addWidget(widget1);
        mAnimationStackedWidget->addWidget(widget2);
    }

    virtual void TearDown()
    {
        delete mAnimationStackedWidget;
        mAnimationStackedWidget = nullptr;
    }
protected:
    AnimationStackedWidget *mAnimationStackedWidget = nullptr;
    QWidget *widget1 = nullptr;
    QWidget *widget2 = nullptr;
};

#endif // TEST_ANIMATIONSTACKEDWIDGET_H
