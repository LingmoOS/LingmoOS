// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmemorywarningdialogadaptor.h"
#include "dmemorywarningdialog.h"

#include <gtest/gtest.h>

class UT_DMemoryWarningDialogAdaptor : public testing::Test
{

};

TEST_F(UT_DMemoryWarningDialogAdaptor, coverageTest)
{
    DMemoryWarningDialog dialog;
    DMemoryWarningDialogAdaptor adaptor(&dialog);

    adaptor.Show({"google-chrome"});
    adaptor.Hide();
}
