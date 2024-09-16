// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "searchlineedit.h"
#undef private

#include <gtest/gtest.h>

class Tst_Searchlineedit : public testing::Test
{};

TEST_F(Tst_Searchlineedit, searchLineEdit_test)
{
    SearchLineEdit edit;
    edit.editMode();
    edit.onTextChanged();
    edit.moveFloatWidget();
}
