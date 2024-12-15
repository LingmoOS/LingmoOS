// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "manager.h"
#include "kblayoutprovider.h"
#undef private

#include <gtest/gtest.h>

class UT_Manager : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new Manager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    Manager *obj = nullptr;
};

TEST_F(UT_Manager, coverageTest)
{

    QStringList argList;
    argList << "AudioUp" << "AudioDown" << "AudioMute";
    argList << "BrightnessUp" << "BrightnessDown";
    argList << "CapsLockOn" << "CapsLockOff";
    argList << "NumLockOn" << "NumLockOff";
    argList << "TouchpadOn" << "TouchpadOff"<< "TouchpadToggle";
    argList << "SwitchWM3D" << "SwitchWM2D"<< "SwitchWMError";
    argList << "AudioMicMuteOn" << "AudioMicMuteOff";
    argList << "WLANOn" << "WLANOff";
    argList << "DirectSwitchLayout";
    argList << "SwitchLayoutDone";

    obj->m_currentProvider = obj->m_kbLayoutProvider;
    KBLayoutProvider *provide = qobject_cast<KBLayoutProvider *>(obj->m_kbLayoutProvider);
    if (provide) {
        QStringList layouts({"en"});
        provide->userLayoutListChanged(layouts);
    }

    foreach(QString arg , argList) {
        obj->ShowOSD(arg);
    }
    obj->updateUI();
}
