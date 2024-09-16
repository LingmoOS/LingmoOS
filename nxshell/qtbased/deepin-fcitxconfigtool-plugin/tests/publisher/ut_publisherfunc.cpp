// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include <QWidget>
#include <QJsonDocument>
#include <QKeyEvent>
#include "publisher/publisherfunc.h"

TEST(publisherFunc, pause)
{
    publisherFunc::pause(100);
}

TEST(publisherFunc, moveToWindowCenter)
{
    QWidget widget;
    publisherFunc::moveToWindowCenter(&widget);
}

TEST(publisherFunc, createDir)
{
    QString path = "/tmp/fcitx/test/";
    publisherFunc::createDir(path);
    publisherFunc::removeDir(path);
    publisherFunc::createDir(path);
}

TEST(publisherFunc, createFile)
{
    publisherFunc::createFile("/tmp/fcitx/test/testfile", "{\"firstName\":\"Brett\",\"lastName\":\"McLaughlin\"}");
}

TEST(publisherFunc, readJson)
{
    publisherFunc::createFile("/tmp/fcitx/test/testfile", "{\"firstName\":\"Brett\",\"lastName\":\"McLaughlin\"}");
    QJsonDocument temp = publisherFunc::readJson("/tmp/fcitx/test/testfile");
    temp = publisherFunc::readJson("/etc/os-release");
}

TEST(publisherFunc, readFile)
{
    QString temp = publisherFunc::readFile("/etc/shadow-");
    temp = publisherFunc::readFile("/tmp/fcitx/test/testfile");
}

TEST(publisherFunc, getImagePixel)
{
    publisherFunc::getImagePixel("");
}

TEST(publisherFunc, getDirFileNames)
{
    publisherFunc::getDirFileNames("/tmp/");
    publisherFunc::getDirFileNames("/uostestim/");
}

TEST(publisherFunc, startPopen)
{
    publisherFunc::startPopen("echo hello world");
}

TEST(publisherFunc, fontSize)
{
    publisherFunc::fontSize("echo hello world");
}

TEST(publisherFunc, getKeyValue)
{
    QKeyEvent* key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    publisherFunc::getKeyValue(key_event);
    delete key_event;
    key_event = nullptr;
    publisherFunc::getKeyValue(key_event);

    publisherFunc::getKeyValue(Qt::Key_Home);
    publisherFunc::getKeyValue(Qt::Key_End);
    publisherFunc::getKeyValue(Qt::Key_PageUp);
    publisherFunc::getKeyValue(Qt::Key_PageDown);
    publisherFunc::getKeyValue(Qt::Key_Insert);
    publisherFunc::getKeyValue(Qt::Key_Up);
    publisherFunc::getKeyValue(Qt::Key_Right);
    publisherFunc::getKeyValue(Qt::Key_Left);
    publisherFunc::getKeyValue(Qt::Key_Down);
    publisherFunc::getKeyValue(Qt::Key_Delete);
    publisherFunc::getKeyValue(Qt::Key_Space);
    publisherFunc::getKeyValue(Qt::Key_F1);
    publisherFunc::getKeyValue(Qt::Key_F2);
    publisherFunc::getKeyValue(Qt::Key_F3);
    publisherFunc::getKeyValue(Qt::Key_F4);
    publisherFunc::getKeyValue(Qt::Key_F5);
    publisherFunc::getKeyValue(Qt::Key_F6);
    publisherFunc::getKeyValue(Qt::Key_F7);
    publisherFunc::getKeyValue(Qt::Key_F8);
    publisherFunc::getKeyValue(Qt::Key_F9);
    publisherFunc::getKeyValue(Qt::Key_F10);
    publisherFunc::getKeyValue(Qt::Key_F11);
    publisherFunc::getKeyValue(Qt::Key_F12);
    publisherFunc::getKeyValue(Qt::Key_NumLock);
    publisherFunc::getKeyValue(Qt::Key_ScrollLock);
    publisherFunc::getKeyValue(Qt::Key_Pause);
    publisherFunc::getKeyValue(Qt::Key_Super_L);
    publisherFunc::getKeyValue(Qt::Key_Super_R);
    publisherFunc::getKeyValue(Qt::Key_Escape);
    publisherFunc::getKeyValue(Qt::Key_Tab);
    publisherFunc::getKeyValue(Qt::Key_CapsLock);
    publisherFunc::getKeyValue(Qt::Key_Shift);
    publisherFunc::getKeyValue(Qt::Key_Control);
    publisherFunc::getKeyValue(Qt::Key_Alt);
    publisherFunc::getKeyValue(Qt::Key_Backspace);
    publisherFunc::getKeyValue(Qt::Key_Meta);
    publisherFunc::getKeyValue(Qt::Key_Return);
    publisherFunc::getKeyValue(Qt::Key_Enter);
}
