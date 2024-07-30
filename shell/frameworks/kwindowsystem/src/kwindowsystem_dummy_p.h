/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KWINDOWSYSTEM_DUMMY_P_H
#define KWINDOWSYSTEM_DUMMY_P_H

#include "kwindowsystem_p.h"

class KWindowSystemPrivateDummy : public KWindowSystemPrivate
{
public:
    void activateWindow(QWindow *win, long time) override;
    bool showingDesktop() override;
    void setShowingDesktop(bool showing) override;
};

#endif
