/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#ifndef GRABBINGINHIBITION_H
#define GRABBINGINHIBITION_H

#include "shortcutinhibition_p.h"

class KeyboardGrabber : public ShortcutInhibition
{
public:
    explicit KeyboardGrabber(QWindow *window);
    ~KeyboardGrabber() override;
    void enableInhibition() override;
    void disableInhibition() override;
    bool shortcutsAreInhibited() const override;

private:
    QWindow *m_grabbedWindow;
    bool m_grabbingKeyboard;
};

#endif
