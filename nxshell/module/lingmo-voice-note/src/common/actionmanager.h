// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H

#include "widgets/vnoterightmenu.h"

#include <DMenu>

#include <QObject>
#include <QMap>

DWIDGET_USE_NAMESPACE

class ActionManager : public QObject
{
    Q_OBJECT
public:
    static ActionManager *Instance();

    enum ActionKind {
        Invalid = 0,
        //Notepad
        NotebookMenuBase,
        NotebookRename = NotebookMenuBase,
        NotebookDelete,
        NotebookAddNew,
        //Add notebook menu item begin {

        //Add notebook menu item end }
        NotebookMenuMax,

        //notes
        NoteMenuBase,
        NoteRename = NoteMenuBase,
        NoteTop,
        NoteMove,
        NoteDelete,
        NoteSave,
        NoteSaveVoice,
        NoteAddNew,
        //Add note menu item begin {

        //Add note menu item end }
        NoteMenuMax,

        //Voice
        VoiceMenuBase,
        VoiceAsSave = VoiceMenuBase,
        VoiceToText,
        VoiceDelete,
        VoiceSelectAll,
        VoiceCopy,
        VoiceCut,
        VoicePaste,
        //Add voice menu item begin {

        //Add voice menu item end }
        VoiceMenuMax,

        //pciture
        PictureMenuBase,
        PictureView = PictureMenuBase,
        PictureDelete,
        PictureSelectAll,
        PictureCopy,
        PictureCut,
        PicturePaste,
        PictureSaveAs,
        //Add pciture menu item begin {

        //Add pciture menu item end }
        PictureMenuMax,

        //txt
        TxtMenuBase,
        TxtDelete = TxtMenuBase,
        TxtSelectAll,
        TxtCopy,
        TxtCut,
        TxtPaste,

        TxtSpeech,
        TxtStopreading,
        TxtDictation,
        TxtTranslate,
        //Add Txt menu item begin {

        //Add Txt menu item end }
        TxtMenuMax,

        SaveNoteMenuBase,
        SaveNoteAsHtml = SaveNoteMenuBase, //保存为HTML
        SaveNoteAsText, //保存为txt
        SaveNoteMax,

        MenuMaxId
    };

    Q_ENUM(ActionKind)

    //Menu types
    enum MenuType {
        NotebookCtxMenu,
        NoteCtxMenu,
        VoiceCtxMenu,
        PictureCtxMenu,
        TxtCtxMenu,
        SaveNoteCtxMenu, //保存笔记二级菜单
    };
    Q_ENUM(MenuType)
    //获取记事本列表右键菜单
    VNoteRightMenu *notebookContextMenu();
    //获取记事项列表右键菜单
    VNoteRightMenu *noteContextMenu();
    //获取记事项列表右键菜单的的二级菜单保存笔记菜单
    VNoteRightMenu *saveNoteContextMenu();
    //获取语音文本右键菜单
    VNoteRightMenu *voiceContextMenu();
    //获取图片文本右键菜单
    VNoteRightMenu *pictureContextMenu();
    //获取文字文本右键菜单
    VNoteRightMenu *txtContextMenu();
    //获取菜单项ID
    ActionKind getActionKind(QAction *action);
    //获取菜单项
    QAction *getActionById(ActionKind id);
    //设置菜单项是否可用
    void enableAction(ActionKind actionId, bool enable);
    //设置菜单项是否可见
    void visibleAction(ActionKind actionId, bool enable);
    //重置菜单项可用状态
    void resetCtxMenu(MenuType type, bool enable = true);
    //隐藏/显示语音助手相关功能
    void visibleAiActions(bool visible);

protected:
    ActionManager();
    //初始化
    void initMenu();
    //获取菜单指针
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_notebookContextMenu;
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_noteContextMenu;
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_voiceContextMenu;
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_pictureContextMenu;
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_txtContextMenu;
    QScopedPointer<VNoteRightMenu, QScopedPointerDeleteLater> m_saveNoteContextMenu;

    QMap<ActionKind, QAction *> m_actionsMap;

    //语音助手功能分割线
    QAction *m_aiSeparator {nullptr};
};

#endif // ACTIONFACTORY_H
