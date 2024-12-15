// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYDEFINE_H
#define KEYDEFINE_H
// Virtual button codes that are not defined by X11.
#define Button1         1
#define Button2         2
#define Button3         3
#define WheelUp         4
#define WheelDown       5
#define WheelLeft       6
#define WheelRight      7
#define XButton1        8
#define XButton2        9

#pragma once

// Upper side keys
#define KEY_ESCAPE 0x9
#define KEY_F1 0x43
#define KEY_F2 0x44
#define KEY_F3 0x45
#define KEY_F4 0x46
#define KEY_F5 0x47
#define KEY_F6 0x48
#define KEY_F7 0x49
#define KEY_F8 0x4a
#define KEY_F9 0x4b
#define KEY_F10 0x4c
#define KEY_F11 0x5f
#define KEY_F12 0x60
#define KEY_TILDE 0x31 // `~
#define KEY_1 0xa
#define KEY_2 0xb
#define KEY_3 0xc
#define KEY_4 0xd
#define KEY_5 0xe
#define KEY_6 0xf
#define KEY_7 0x10
#define KEY_8 0x11
#define KEY_9 0x12
#define KEY_0 0x13
#define KEY_MINUS 0x14 // -_
#define KEY_PLUS 0x15 // =+
#define KEY_EQUAL 0x15 // =+

// Mid keys
#define KEY_A 0x26
#define KEY_B 0x38
#define KEY_C 0x36
#define KEY_D 0x28
#define KEY_E 0x1a
#define KEY_F 0x29
#define KEY_G 0x2a
#define KEY_H 0x2b
#define KEY_I 0x1f
#define KEY_J 0x2c
#define KEY_K 0x2d
#define KEY_L 0x2e
#define KEY_M 0x3a
#define KEY_N 0x39
#define KEY_O 0x20
#define KEY_P 0x21
#define KEY_Q 0x18
#define KEY_R 0x1b
#define KEY_S 0x27
#define KEY_T 0x1c
#define KEY_U 0x1e
#define KEY_V 0x37
#define KEY_W 0x19
#define KEY_X 0x35
#define KEY_Y 0x1d
#define KEY_Z 0x34
#define KEY_BRACKET_OPEN 0x22 // [{
#define KEY_BRACKET_CLOSE 0x23 // ]}
#define KEY_SEMICOLON 0x2f // ;:
#define KEY_QUOTE 0x30 // '"
#define KEY_BACKSLASH 0x33 // \|
#define KEY_VBAR 0x33 // \|
#define KEY_COMMA 0x3b // ,<
#define KEY_PERIOD 0x3c // .>
#define KEY_SLASH 0x3d // /?
#define KEY_QUESTION 0x3d // /?

// Control keys
#define KEY_TAB 0x17
#define KEY_CAPSLOCK 0x42
#define KEY_LSHIFT 0x32
#define KEY_LCONTROL 0x25
#define KEY_LCTRL 0x25
#define KEY_LALT 0x40
#define KEY_SPACE 0x41
#define KEY_RALT 0x6c
// TODO: Somehow catch this one
//#define KEY_LWIN 0x85 // Probably, I'm not sure
//#define KEY_LSUPER 0x85
#define KEY_RWIN 0x86
#define KEY_RSUPER 0x86
#define KEY_APPS 0x87
#define KEY_MENU 0x87
#define KEY_RCTRL 0x69
#define KEY_RSHIFT 0x3e
#define KEY_ENTER 0x24
#define KEY_RETURN 0x24
#define KEY_BACKSPACE 0x16

// Side keys
#define KEY_SCROLLLOCK 0x4e
#define KEY_PAUSE 0x7f
#define KEY_INSERT 0x76
#define KEY_HOME 0x6e
#define KEY_PAGEUP 0x70
#define KEY_DELETE 0x77
#define KEY_END 0x73
#define KEY_PAGEDOWN 0x75
#define KEY_LEFT 0x71
#define KEY_UP 0x6f
#define KEY_RIGHT 0x72
#define KEY_DOWN 0x74

// Num keys
#define KEY_NUMLOCK 0x4d
#define KEY_DIVIDE 0x6a
#define KEY_DIV 0x6a
#define KEY_NUMSLASH 0x6a
#define KEY_MULTIPLY 0x3f
#define KEY_MUL 0x3f
#define KEY_STAR 0x3f
#define KEY_NUMSTAR 0x3f
#define KEY_NUM7 0x4f
#define KEY_NUM8 0x50
#define KEY_NUM9 0x51
#define KEY_NUMMINUS 0x52
#define KEY_NUM4 0x53
#define KEY_NUM5 0x54
#define KEY_NUM6 0x55
#define KEY_NUMPLUS 0x56
#define KEY_NUM1 0x57
#define KEY_NUM2 0x58
#define KEY_NUM3 0x59
#define KEY_NUM0 0x5a
#define KEY_NUMDELETE 0x5b
#define KEY_NUMDEL 0x5b
#define KEY_NUMENTER 0x68
#define KEY_NUMRETURN 0x68
#endif // KEYDEFINE_H
