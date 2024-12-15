// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionmanager.h"
#include "common/opsstateinterface.h"

#include <DApplication>
#include <DLog>

#include <QSignalMapper>

#define MenuId "_menuid_"

/**
 * @brief ActionManager::ActionManager
 */
ActionManager::ActionManager()
{
    initMenu();
}

/**
 * @brief ActionManager::Instance
 * @return 单例对象
 */
ActionManager *ActionManager::Instance()
{
    static ActionManager _instance;
    return &_instance;
}

/**
 * @brief ActionManager::notebookContextMenu
 * @return 记事本右键菜单
 */
VNoteRightMenu *ActionManager::notebookContextMenu()
{
    return m_notebookContextMenu.get();
}

/**
 * @brief ActionManager::noteContextMenu
 * @return 记事项右键菜单
 */
VNoteRightMenu *ActionManager::noteContextMenu()
{
    return m_noteContextMenu.get();
}

/**
 * @brief ActionManager::saveNoteContextMenu
 * 获取记事项列表右键菜单的的二级菜单保存笔记右键菜单
 * @return 保存笔记二级菜单
 */
VNoteRightMenu *ActionManager::saveNoteContextMenu()
{
    return m_saveNoteContextMenu.get();
}

VNoteRightMenu *ActionManager::voiceContextMenu()
{
    return m_voiceContextMenu.get();
}

VNoteRightMenu *ActionManager::pictureContextMenu()
{
    return m_pictureContextMenu.get();
}

VNoteRightMenu *ActionManager::txtContextMenu()
{
    return m_txtContextMenu.get();
}

/**
 * @brief ActionManager::getActionKind
 * @param action 菜单项
 * @return id
 */
ActionManager::ActionKind ActionManager::getActionKind(QAction *action)
{
    return action->property(MenuId).value<ActionKind>();
}

/**
 * @brief ActionManager::getActionById
 * @param id
 * @return 菜单项
 */
QAction *ActionManager::getActionById(ActionManager::ActionKind id)
{
    QAction *menuAction = nullptr;

    QMap<ActionKind, QAction *>::iterator it = m_actionsMap.find(id);

    if (it != m_actionsMap.end()) {
        menuAction = *it;
    }

    return menuAction;
}

/**
 * @brief ActionManager::enableAction
 * @param actionId
 * @param enable true 可用
 */
void ActionManager::enableAction(ActionManager::ActionKind actionId, bool enable)
{
    QMap<ActionKind, QAction *>::iterator it = m_actionsMap.find(actionId);

    if (it != m_actionsMap.end()) {
        (*it)->setEnabled(enable);
    }
}

/**
 * @brief ActionManager::visibleAction
 * @param actionId
 * @param enable true显示
 */
void ActionManager::visibleAction(ActionManager::ActionKind actionId, bool enable)
{
    QMap<ActionKind, QAction *>::iterator it = m_actionsMap.find(actionId);

    if (it != m_actionsMap.end()) {
        (*it)->setVisible(enable);
    }
}

/**
 * @brief ActionManager::resetCtxMenu
 * @param type 右键菜单类型
 * @param enable true可用
 */
void ActionManager::resetCtxMenu(ActionManager::MenuType type, bool enable)
{
    int startMenuId = MenuMaxId;
    int endMenuId = MenuMaxId;

    if (MenuType::NotebookCtxMenu == type) {
        startMenuId = NotebookMenuBase;
        endMenuId = NotebookMenuMax;
    } else if (MenuType::NoteCtxMenu == type) {
        startMenuId = NoteMenuBase;
        endMenuId = NoteMenuMax;
    } else if (MenuType::VoiceCtxMenu == type) {
        startMenuId = VoiceMenuBase;
        endMenuId = VoiceMenuMax;
    } else if (MenuType::PictureCtxMenu == type) {
        startMenuId = PictureMenuBase;
        endMenuId = PictureMenuMax;
    } else if (MenuType::TxtCtxMenu == type) {
        startMenuId = TxtMenuBase;
        endMenuId = TxtMenuMax;
    } else if (MenuType::SaveNoteCtxMenu == type) {
        startMenuId = SaveNoteMenuBase;
        endMenuId = SaveNoteMax;
    }

    QAction *pAction = nullptr;

    for (; startMenuId < endMenuId; startMenuId++) {
        pAction = m_actionsMap[static_cast<ActionKind>(startMenuId)];

        if (nullptr != pAction) {
            pAction->setEnabled(enable);
        }
    }
}
/**
 * @brief ActionManager::initMenu
 */
void ActionManager::initMenu()
{
    //保存笔记二级菜单项文案
    QStringList saveNoteMenuTexts;
    saveNoteMenuTexts << DApplication::translate("NotesContextMenu", "Save as HTML")
                      << DApplication::translate("NotesContextMenu", "Save as TXT");

    //初始化笔记右键菜单
    VNoteRightMenu *saveNoteMenu = new VNoteRightMenu();
    saveNoteMenu->setTitle(DApplication::translate("NotesContextMenu", "Save note"));
    m_saveNoteContextMenu.reset(saveNoteMenu);
    int saveNoteMenuIdStart = ActionKind::SaveNoteMenuBase;

    //添加菜单选项
    for (auto it : saveNoteMenuTexts) {
        QAction *pAction = new QAction(it, m_saveNoteContextMenu.get());
        pAction->setProperty(MenuId, saveNoteMenuIdStart);

        m_saveNoteContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(saveNoteMenuIdStart), pAction);

        saveNoteMenuIdStart++;
        if (SaveNoteMax == saveNoteMenuIdStart) {
            break;
        }
    }

    //Notebook context menu
    QStringList notebookMenuTexts;
    notebookMenuTexts << DApplication::translate("NotebookContextMenu", "Rename")
                      << DApplication::translate("NotebookContextMenu", "Delete")
                      << DApplication::translate("NotebookContextMenu", "New note");
    //初始化记事本右键菜单
    m_notebookContextMenu.reset(new VNoteRightMenu());

    int notebookMenuIdStart = ActionKind::NotebookMenuBase;

    for (auto it : notebookMenuTexts) {
        QAction *pAction = new QAction(it, m_notebookContextMenu.get());
        pAction->setProperty(MenuId, notebookMenuIdStart);

        m_notebookContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(notebookMenuIdStart), pAction);

        notebookMenuIdStart++;
    }

    //Note context menu
    QStringList noteMenuTexts;
    noteMenuTexts << DApplication::translate("NotesContextMenu", "Rename")
                  << DApplication::translate("NotesContextMenu", "")
                  << DApplication::translate("NotesContextMenu", "Move")
                  << DApplication::translate("NotesContextMenu", "Delete")
                  << DApplication::translate("NotesContextMenu", "")
                  << DApplication::translate("NotesContextMenu", "Save voice recording")
                  << DApplication::translate("NotesContextMenu", "New note");

    //初始化笔记右键菜单
    m_noteContextMenu.reset(new VNoteRightMenu());

    int noteMenuIdStart = ActionKind::NoteMenuBase;

    for (auto it : noteMenuTexts) {
        //添加二级菜单
        if (NoteSave == noteMenuIdStart) {
            m_noteContextMenu->addMenu(m_saveNoteContextMenu.get());
            noteMenuIdStart++;
            continue;
        }

        QAction *pAction = new QAction(it, m_noteContextMenu.get());
        pAction->setProperty(MenuId, noteMenuIdStart);

        m_noteContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(noteMenuIdStart), pAction);

        noteMenuIdStart++;

        if (noteMenuIdStart == NoteAddNew) {
            m_noteContextMenu->addSeparator();
        }
    }

    //Voice context menu
    QStringList voiceMenuTexts;
    voiceMenuTexts << DApplication::translate("NoteDetailContextMenu", "Save as MP3")
                   << DApplication::translate("NoteDetailContextMenu", "Voice to Text")
                   << DApplication::translate("NoteDetailContextMenu", "Delete")
                   << DApplication::translate("NoteDetailContextMenu", "Select all")
                   << DApplication::translate("NoteDetailContextMenu", "Copy")
                   << DApplication::translate("NoteDetailContextMenu", "Cut")
                   << DApplication::translate("NoteDetailContextMenu", "Paste");

    //初始化语音文本右键菜单
    m_voiceContextMenu.reset(new VNoteRightMenu());

    int voiceMenuIdStart = ActionKind::VoiceMenuBase;

    for (auto it : voiceMenuTexts) {
        QAction *pAction = new QAction(it, m_voiceContextMenu.get());
        pAction->setProperty(MenuId, voiceMenuIdStart);

        m_voiceContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(voiceMenuIdStart), pAction);
        voiceMenuIdStart++;

        if (VoiceMenuMax == voiceMenuIdStart) {
            break;
        }
    }

    //picture context menu
    QStringList pcitureMenuTexts;
    pcitureMenuTexts << DApplication::translate("NoteDetailContextMenu", "View")
                     << DApplication::translate("NoteDetailContextMenu", "Delete")
                     << DApplication::translate("NoteDetailContextMenu", "Select all")
                     << DApplication::translate("NoteDetailContextMenu", "Copy")
                     << DApplication::translate("NoteDetailContextMenu", "Cut")
                     << DApplication::translate("NoteDetailContextMenu", "Paste")
                     << DApplication::translate("NoteDetailContextMenu", "Save as");

    //初始化图片文本右键菜单
    m_pictureContextMenu.reset(new VNoteRightMenu());

    int pictureMenuIdStart = ActionKind::PictureMenuBase;

    for (auto it : pcitureMenuTexts) {
        QAction *pAction = new QAction(it, m_pictureContextMenu.get());
        pAction->setProperty(MenuId, pictureMenuIdStart);

        m_pictureContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(pictureMenuIdStart), pAction);
        pictureMenuIdStart++;
        if (PictureMenuMax == pictureMenuIdStart) {
            break;
        }
    }

    //txt context menu
    QStringList txtMenuTexts;
    txtMenuTexts << DApplication::translate("NoteDetailContextMenu", "Delete")
                 << DApplication::translate("NoteDetailContextMenu", "Select all")
                 << DApplication::translate("NoteDetailContextMenu", "Copy")
                 << DApplication::translate("NoteDetailContextMenu", "Cut")
                 << DApplication::translate("NoteDetailContextMenu", "Paste")
                 << DApplication::translate("NoteDetailContextMenu", "Text to Speech")
                 << DApplication::translate("NoteDetailContextMenu", "Stop reading")
                 << DApplication::translate("NoteDetailContextMenu", "Speech to Text")
                 << DApplication::translate("NoteDetailContextMenu", "Translate");

    //初始化文字文本右键菜单
    m_txtContextMenu.reset(new VNoteRightMenu());

    int txtMenuIdStart = ActionKind::TxtMenuBase;

    for (auto it : txtMenuTexts) {
        QAction *pAction = new QAction(it, m_txtContextMenu.get());
        pAction->setProperty(MenuId, txtMenuIdStart);

        m_txtContextMenu->addAction(pAction);
        m_actionsMap.insert(static_cast<ActionKind>(txtMenuIdStart), pAction);
        if (TxtPaste == txtMenuIdStart) {
            m_aiSeparator = m_txtContextMenu->addSeparator();
        }
        txtMenuIdStart++;
        if (TxtMenuMax == txtMenuIdStart) {
            break;
        }
    }
}

void ActionManager::visibleAiActions(bool visible)
{
    visibleAction(VoiceToText, visible);
    visibleAction(TxtSpeech, visible);
    visibleAction(TxtStopreading, visible);
    visibleAction(TxtDictation, visible);
    visibleAction(TxtTranslate, visible);
    m_aiSeparator->setVisible(visible);
}
