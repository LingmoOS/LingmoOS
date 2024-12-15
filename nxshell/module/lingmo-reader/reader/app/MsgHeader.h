// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MSGHEADER_H
#define MSGHEADER_H

namespace {

//  搜索框消息
enum E_FIND_WIDGET_MSG {
    E_FIND_PREV,
    E_FIND_NEXT,
    E_FIND_EXIT,
    E_FIND_CONTENT,
};

//  高亮&注释消息
enum E_NOTE_MSG {
    MSG_NOTE_ADD = 0,
    MSG_NOTE_MOVE,
    MSG_NOTE_DELETE,
    MSG_ALL_NOTE_DELETE
};

enum E_MENU_ACTION {
    E_HANDLE_SELECT_TEXT,
    E_HANDLE_HANDLE_TOOL,
    E_BOOKMARK_DELETE,
    E_BOOKMARK_DELETE_ALL,
    E_NOTE_COPY,
    E_NOTE_DELETE,
    E_NOTE_DELETE_ALL
};

}
#endif  // MSGHEADER_H
