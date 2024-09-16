// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_animationstackedwidget.h"

test_animationstackedwidget::test_animationstackedwidget()
{

}

test_animationstackedwidget::~test_animationstackedwidget()
{

}

//void AnimationStackedWidget::setDuration(int duration)
TEST_F(test_animationstackedwidget, setDuration)
{
    mAnimationStackedWidget->setDuration(400);
}

//void AnimationStackedWidget::animationFinished()
TEST_F(test_animationstackedwidget, animationfinished)
{
    mAnimationStackedWidget->animationFinished();
}

//void AnimationStackedWidget::setCurrent(int index)
TEST_F(test_animationstackedwidget, setCurrent)
{
    mAnimationStackedWidget->setCurrent(4);
}

//void AnimationStackedWidget::setPre()
TEST_F(test_animationstackedwidget, setPre)
{
    mAnimationStackedWidget->setPre();
}

//void AnimationStackedWidget::setNext()
TEST_F(test_animationstackedwidget, setNext)
{
    mAnimationStackedWidget->setNext();
}

//void AnimationStackedWidget::setCurrentWidget(int &index, int beginWidth)
TEST_F(test_animationstackedwidget, setCurrentWidget)
{
    int index = 2;
    mAnimationStackedWidget->setCurrentWidget(index, 8);
}

//int AnimationStackedWidget::getBeginValue()
TEST_F(test_animationstackedwidget, getBeginValue)
{
    mAnimationStackedWidget->getBeginValue();
}

//AnimationStackedWidget::MoveOrientation AnimationStackedWidget::getMoveOrientation(const int currIndex, const int nextIndex)
TEST_F(test_animationstackedwidget, getMoveOrientation)
{
    mAnimationStackedWidget->getMoveOrientation(2, 4);
}

//double AnimationStackedWidget::offset() const
TEST_F(test_animationstackedwidget, offset)
{
    mAnimationStackedWidget->offset();
}

//void AnimationStackedWidget::setOffset(double offset)
TEST_F(test_animationstackedwidget, setOffset)
{
    mAnimationStackedWidget->setOffset(2.0);
}

TEST_F(test_animationstackedwidget,animationtest)
{
    mAnimationStackedWidget->setCurrentIndex(0);
    mAnimationStackedWidget->setDuration(250);
    mAnimationStackedWidget->setCurrent(1);
    mAnimationStackedWidget->setFixedSize(500, 800);
    QPixmap pixmap(mAnimationStackedWidget->size());
    for (int i = 0; i <10; ++i) {
        mAnimationStackedWidget->setOffset(i);
        pixmap = mAnimationStackedWidget->grab();
    }

    mAnimationStackedWidget->setCurrentIndex(1);
    mAnimationStackedWidget->setCurrent(0);
    for (int i = 0; i <10; ++i) {
        mAnimationStackedWidget->setOffset(i);
        pixmap = mAnimationStackedWidget->grab();
    }
}
