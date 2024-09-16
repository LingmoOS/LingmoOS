// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnotemessagedialog.h"
#include "vnotemessagedialog.h"
#include <DApplication>

UT_VNoteMessageDialog::UT_VNoteMessageDialog()
{
}

TEST_F(UT_VNoteMessageDialog, UT_VNoteMessageDialog_initConnections_001)
{
    VNoteMessageDialog vnotemessagedialog(0);
    vnotemessagedialog.initConnections();
}

TEST_F(UT_VNoteMessageDialog, UT_VNoteMessageDialog_setSingleButton_001)
{
    VNoteMessageDialog vnotemessagedialog(0);
    vnotemessagedialog.setSingleButton();
    EXPECT_FALSE(vnotemessagedialog.m_buttonSpliter->isVisible())
        << "setSingleButton, m_buttonSpliter->isVisible()";
    EXPECT_FALSE(vnotemessagedialog.m_confirmBtn->isVisible())
        << "setSingleButton, m_confirmBtn->isVisible()";
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "OK", "button"),
              vnotemessagedialog.m_cancelBtn->text())
        << "setSingleButton, m_cancelBtn->text";
}

TEST_F(UT_VNoteMessageDialog, UT_VNoteMessageDialog_initMessage_001)
{
    VNoteMessageDialog vnotemessagedialog(0);
    vnotemessagedialog.m_msgType = vnotemessagedialog.DeleteFolder;

    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog",
                                      "Are you sure you want to delete this notebook?\nAll notes in it will be deleted"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is DeleteFolder, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.AbortRecord;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Do you want to stop the current recording?"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is AbortRecord, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.AsrTimeLimit;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog",
                                      "Cannot convert this voice note, as notes over 20 minutes are not supported at present."),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is AsrTimeLimit, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.AborteAsr;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Converting a voice note now. Do you want to stop it?"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is AborteAsr, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.VolumeTooLow;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog",
                                      "The low input volume may result in bad recordings. Do you want to continue?"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is VolumeTooLow, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.CutNote;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog",
                                      "The clipped recordings and converted text will not be pasted. Do you want to continue?"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is CutNote, m_pMessage->text";

    vnotemessagedialog.m_msgType = vnotemessagedialog.DeleteNote;

    vnotemessagedialog.m_notesCount = 1;
    vnotemessagedialog.initMessage();
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Are you sure you want to delete this note?"),
              vnotemessagedialog.m_pMessage->text())
        << "m_msgType is DeleteNote, m_notesCount = 1, m_pMessage->text";
    for (int i = 2; i < 100; i *= 3) {
        vnotemessagedialog.m_notesCount = i;
        vnotemessagedialog.initMessage();
        EXPECT_EQ(DApplication::translate("VNoteMessageDialog",
                                          "Are you sure you want to delete the selected %1 notes?")
                      .arg(i),
                  vnotemessagedialog.m_pMessage->text())
            << "m_msgType is DeleteNote, m_notesCount = " << i << ", m_pMessage->text";
    }

    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Cancel", "button"),
              vnotemessagedialog.m_cancelBtn->text())
        << "m_cancelBtn->text";

    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Confirm", "button"),
              vnotemessagedialog.m_confirmBtn->text())
        << "m_confirmBtn->text";
}

TEST_F(UT_VNoteMessageDialog, UT_VNoteMessageDialog_initMessage_002)
{
    VNoteMessageDialog vnotemessagedialog1(7);
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "Save failed"),
              vnotemessagedialog1.m_pMessage->text())
        << "m_msgType is SaveFailed";

    VNoteMessageDialog vnotemessagedialog2(8);
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "You do not have permission to save files there"),
              vnotemessagedialog2.m_pMessage->text())
        << "m_msgType is NoPermission";

    VNoteMessageDialog vnotemessagedialog3(9);
    EXPECT_EQ(DApplication::translate("VNoteMessageDialog", "The voice note has been deleted"),
              vnotemessagedialog3.m_pMessage->text())
        << "m_msgType is VoicePathNoAvail";
}
