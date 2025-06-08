/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>

class JoyWidget;

/* on FreeBSD the header <sys/joystick.h> already has a struct joystick, so we can't use the same name here, Alex */
class Joystick : public KCModule
{
    Q_OBJECT

public:
    explicit Joystick(QWidget *parent = nullptr, const QVariantList &list = QVariantList());

    void load() override;
    void defaults() override;

private:
    JoyWidget *joyWidget;
};
