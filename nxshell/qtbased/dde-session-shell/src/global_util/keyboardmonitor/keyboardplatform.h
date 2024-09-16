// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDPLATFORM_H
#define KEYBOARDPLATFORM_H

#include <QObject>

class KeyBoardPlatform : public QObject
{
    Q_OBJECT
public:
    KeyBoardPlatform(QObject *parent = nullptr);
    virtual ~KeyBoardPlatform();

    virtual bool isCapslockOn() = 0;
    virtual bool isNumlockOn() = 0;
    virtual bool setNumlockStatus(const bool &on) = 0;
    virtual void run() = 0;

signals:
    void capslockStatusChanged(bool on);
    void numlockStatusChanged(bool on);
};

#endif // KEYBOARDPLATFORM_H
