// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_actionmanager.h"
#include "actionmanager.h"
#include "opsstateinterface.h"

UT_ActionManager::UT_ActionManager()
{
}

TEST_F(UT_ActionManager, UT_ActionManager_notebookContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_notebookContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_notebookContextMenu.get(), actionmanager.notebookContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_noteContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_noteContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_noteContextMenu.get(), actionmanager.noteContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_saveNoteContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_saveNoteContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_saveNoteContextMenu.get(), actionmanager.saveNoteContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_voiceContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_voiceContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_voiceContextMenu.get(), actionmanager.voiceContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_pictureContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_pictureContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_pictureContextMenu.get(), actionmanager.pictureContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_txtContextMenu_001)
{
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_txtContextMenu.isNull()) << "bool";
    EXPECT_EQ(actionmanager.m_txtContextMenu.get(), actionmanager.txtContextMenu())
        << "eq";
}

TEST_F(UT_ActionManager, UT_ActionManager_getActionKind_001)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NotebookAddNew);
    EXPECT_EQ(actionmanager.getActionKind(tmpact), actionmanager.NotebookAddNew)
        << "NotebookAddNew";

    tmpact = actionmanager.getActionById(actionmanager.VoiceToText);
    EXPECT_EQ(actionmanager.getActionKind(tmpact), actionmanager.VoiceToText)
        << "VoiceToText";

    tmpact = actionmanager.getActionById(actionmanager.PictureView);
    EXPECT_EQ(actionmanager.getActionKind(tmpact), actionmanager.PictureView)
        << "PictureView";
}

TEST_F(UT_ActionManager, UT_ActionManager_getActionById_001)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NotebookRename);
    EXPECT_EQ(tmpact->text(), "Rename");
}

TEST_F(UT_ActionManager, UT_ActionManager_enableAction_001)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NotebookRename);
    actionmanager.enableAction(actionmanager.NotebookRename, false);
    EXPECT_FALSE(tmpact->isEnabled()) << "false";

    actionmanager.enableAction(actionmanager.NotebookRename, true);
    EXPECT_TRUE(tmpact->isEnabled()) << "true";
}

TEST_F(UT_ActionManager, UT_ActionManager_visibleAction_001)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NotebookRename);
    actionmanager.visibleAction(actionmanager.NotebookRename, false);
    EXPECT_FALSE(tmpact->isVisible());
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_001)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NotebookRename);
    actionmanager.resetCtxMenu(actionmanager.NotebookCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.NotebookCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_002)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.NoteMenuBase);
    actionmanager.resetCtxMenu(actionmanager.NoteCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.NoteCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_003)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.VoiceMenuBase);
    actionmanager.resetCtxMenu(actionmanager.VoiceCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.VoiceCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_004)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.PictureMenuBase);
    actionmanager.resetCtxMenu(actionmanager.PictureCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.PictureCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_005)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.TxtMenuBase);
    actionmanager.resetCtxMenu(actionmanager.TxtCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.TxtCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_resetCtxMenu_006)
{
    ActionManager actionmanager;
    QAction *tmpact = actionmanager.getActionById(actionmanager.SaveNoteMenuBase);
    actionmanager.resetCtxMenu(actionmanager.SaveNoteCtxMenu, false);
    EXPECT_FALSE(tmpact->isEnabled()) << false;
    actionmanager.resetCtxMenu(actionmanager.SaveNoteCtxMenu, true);
    EXPECT_TRUE(tmpact->isEnabled()) << true;
}

TEST_F(UT_ActionManager, UT_ActionManager_initMenu_001)
{
    OpsStateInterface::instance()->operState(OpsStateInterface::instance()->StateAISrvAvailable, false);
    ActionManager actionmanager;
    EXPECT_FALSE(actionmanager.m_saveNoteContextMenu.isNull()) << "m_saveNoteContextMenu";
    EXPECT_FALSE(actionmanager.m_notebookContextMenu.isNull()) << "m_notebookContextMenu";
    EXPECT_FALSE(actionmanager.m_noteContextMenu.isNull()) << "m_noteContextMenu";
    EXPECT_FALSE(actionmanager.m_pictureContextMenu.isNull()) << "m_pictureContextMenu";
    EXPECT_FALSE(actionmanager.m_txtContextMenu.isNull()) << "m_txtContextMenu";
    EXPECT_FALSE(actionmanager.m_saveNoteContextMenu.isNull()) << "m_saveNoteContextMenu";
    EXPECT_FALSE(actionmanager.m_actionsMap.isEmpty()) << "m_actionsMap";
    EXPECT_EQ(34, actionmanager.m_actionsMap.size()) << "m_actionsMap.size";
}

TEST_F(UT_ActionManager, UT_ActionManager_Instance_001)
{
    ActionManager *instanse;
    instanse = ActionManager::Instance();
    EXPECT_TRUE(instanse) << "bool";
    EXPECT_EQ(instanse, ActionManager::Instance()) << "eq";
}
