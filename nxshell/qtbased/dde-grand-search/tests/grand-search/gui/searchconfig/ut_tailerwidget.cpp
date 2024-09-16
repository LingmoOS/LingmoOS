// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/tailerwidget.h"
#include "business/config/searchconfig.h"
#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(TailerWidgetTest, onCheckBoxStateChanged)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchConfig::setConfig, []() { return; });

    TailerWidget w;
    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxStateChanged(0, true));
    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxStateChanged(1, true));
}
