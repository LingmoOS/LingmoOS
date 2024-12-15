// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <DLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QThread>

#include "../deb-installer/view/widgets/packageselectitem.h"

#define private public
#include "../deb-installer/view/pages/packageselectview.h"

class ut_packageSelectView_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { w = new PackageSelectView; }
    void TearDown() { delete w; }

private:
    PackageSelectView *w;
};

TEST_F(ut_packageSelectView_TEST, PackageSelectView_UT_flushDebList)
{
    DebIr ir1;
    DebIr ir2;
    w->flushDebList({ir1, ir2});
    ASSERT_EQ(w->items.size(), 2);
}

TEST_F(ut_packageSelectView_TEST, PackageSelectView_UT_checkSelect)
{
    DebIr ir1;
    ir1.archMatched = true;
    DebIr ir2;
    ir2.archMatched = false;
    w->flushDebList({ir1, ir2});

    w->checkSelect();
    ASSERT_EQ(w->selectAllBox->isChecked(), true);

    w->items[0]->setChecked(false);
    w->checkSelect();
    ASSERT_EQ(w->selectAllBox->isChecked(), false);
}

TEST_F(ut_packageSelectView_TEST, PackageSelectView_UT_selectAll)
{
    DebIr ir1;
    ir1.archMatched = true;
    DebIr ir2;
    ir2.archMatched = false;
    w->flushDebList({ir1, ir2});

    w->selectAll(false);
    ASSERT_EQ(w->selectAllBox->isChecked(), false);

    w->selectAll(true);
    ASSERT_EQ(w->selectAllBox->isChecked(), true);
}

TEST_F(ut_packageSelectView_TEST, PackageSelectView_UT_setHaveMustInstallDeb)
{
    w->setHaveMustInstallDeb(true);
    ASSERT_EQ(w->installButton->isEnabled(), true);

    w->setHaveMustInstallDeb(false);
    ASSERT_EQ(w->installButton->isEnabled(), false);
}

TEST_F(ut_packageSelectView_TEST, PackageSelectView_UT_onInstallClicked)
{
    QObject::connect(
        w, &PackageSelectView::packageInstallConfim, [](const QList<int> &indexes) { ASSERT_EQ(indexes.size(), 2); });

    DebIr ir1;
    ir1.archMatched = true;
    DebIr ir2;
    ir2.archMatched = true;
    w->flushDebList({ir1, ir2});
    w->installButton->click();

    QThread::msleep(50);
}
