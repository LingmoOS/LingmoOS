// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dtaskbarcontrol.h"

#include <QSignalSpy>
#include <gmock/gmock.h>

DGUI_USE_NAMESPACE

// 由于需要发送dbus接口 使用打桩类进行
class MockDTaskbarControl : public DTaskbarControl
{
public:
    using DTaskbarControl::DTaskbarControl;
    MOCK_METHOD1(sendMessage, void(const QVariantMap &));
};

class TDTaskbarControl : public DTest
{
protected:
    void SetUp();
    void TearDown();

    MockDTaskbarControl *control;
};

void TDTaskbarControl::SetUp()
{
    control = new MockDTaskbarControl;
}

void TDTaskbarControl::TearDown()
{
    delete control;
}

TEST_F(TDTaskbarControl, testFunction)
{
    enum { ProgressTestValue = 1 };
    EXPECT_CALL(*control, sendMessage(testing::_)).Times(testing::AtLeast(1));

    QSignalSpy valueChangedSpy(control, SIGNAL(progressChanged(double)));
    control->setProgress(true, ProgressTestValue);
    ASSERT_EQ(valueChangedSpy.count(), 1);

    QSignalSpy visibleChangedSpy(control, SIGNAL(progressVisibleChanged(bool)));
    control->setProgress(false, ProgressTestValue);
    ASSERT_EQ(visibleChangedSpy.count(), 1);

    QSignalSpy counterChangedSpy(control, SIGNAL(counterChanged(int)));
    control->setCounter(true, 1);
    ASSERT_EQ(control->counter(), 1);
    ASSERT_EQ(counterChangedSpy.count(), 1);

    QSignalSpy counterVisibleSpy(control, SIGNAL(counterVisibleChanged(bool)));
    control->setCounter(false, 1);
    ASSERT_EQ(counterVisibleSpy.count(), 1);

    control->setUrgency(true);
    control->setCounterVisible(true);
    ASSERT_EQ(control->counterVisible(), true);
}
