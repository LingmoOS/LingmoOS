// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "clipboardmodel.h"
#include "constants.h"

#include <QApplication>

class TstConstants : public testing::Test
{
};

TEST_F(TstConstants, coverageTest)
{
    Globals::GetScale(QSize(1000,1000), 200, 300);
    Globals::GetScale(QSize(200,300), 1000, 1000);

    QPixmap pix;
    QStyle *style = QApplication::style();
    const QPixmap &testPix = style->standardPixmap(QStyle::SP_DialogYesButton);

    Globals::pixmapScaled(pix);
    Globals::GetRoundPixmap(pix, QColor(Qt::red));

    Globals::pixmapScaled(testPix);
    Globals::GetRoundPixmap(testPix, QColor(Qt::red));
}
