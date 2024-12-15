// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/InfoCommandLinkButton.h"
#include "utils/utils.h"

#include <QKeyEvent>

#include <gtest/gtest.h>

TEST(InfoCommandLinkButton_TEST, InfoCommandLinkButton_UT_setFamily)
{
    InfoCommandLinkButton *btn = new InfoCommandLinkButton("");
    btn->setFocusPolicy(Qt::TabFocus);
    EXPECT_EQ(Qt::TabFocus, btn->focusPolicy());
    delete btn;
}

TEST(InfoCommandLinkButton_TEST, InfoCommandLinkButton_UT_keyPressEvent)
{
    InfoCommandLinkButton *btn = new InfoCommandLinkButton("");
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(btn, &keyPressEvent);
    EXPECT_FALSE(btn->hasFocus());
    delete btn;
}
