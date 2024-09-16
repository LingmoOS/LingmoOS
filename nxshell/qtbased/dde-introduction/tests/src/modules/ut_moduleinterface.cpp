// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_moduleinterface.h"

#define protected public
#include "moduleinterface.h"
#undef protected

ut_moduleinterface_test::ut_moduleinterface_test()
{

}

TEST_F(ut_moduleinterface_test, ModuleInterface)
{
    ModuleInterface* test_module = new ModuleInterface();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_model);
    ASSERT_TRUE(test_module->m_worker);
    ASSERT_TRUE(test_module->m_selectBtn);
    ASSERT_TRUE(test_module->m_updateSelectBtnTimer);
    test_module->deleteLater();
}

TEST_F(ut_moduleinterface_test, setIconType)
{
    ModuleInterface* test_module = new ModuleInterface();
    test_module->setIconType(Model::Big);
    test_module->setIconType(Model::Small);
    test_module->deleteLater();
}

TEST_F(ut_moduleinterface_test, resizeEvent)
{
    ModuleInterface* test_module = new ModuleInterface();
    QResizeEvent event(QSize(test_module->width(),test_module->height()),QSize(120,120));
    test_module->resizeEvent(&event);
    EXPECT_TRUE(test_module->m_updateSelectBtnTimer->isActive());
    test_module->m_updateSelectBtnTimer->stop();
    test_module->deleteLater();
}
