// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_settingsdialog.h"

test_settingsdialog::test_settingsdialog()
{

}


extern void GenerateSettingTranslate();
TEST_F(test_settingsdialog, GenerateSettingTranslate)
{

    GenerateSettingTranslate();
    
}
