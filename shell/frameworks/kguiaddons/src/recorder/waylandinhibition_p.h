/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#ifndef WAYLANDSHORTCUTINHIBITOR_H
#define WAYLANDSHORTCUTINHIBITOR_H

#include "shortcutinhibition_p.h"

#include <memory>

class ShortcutsInhibitManager;
class ShortcutsInhibitor;

class WaylandInhibition : public ShortcutInhibition
{
public:
    explicit WaylandInhibition(QWindow *window);
    ~WaylandInhibition() override;
    bool shortcutsAreInhibited() const override;
    void enableInhibition() override;
    void disableInhibition() override;

private:
    QWindow *const m_window;
    std::shared_ptr<ShortcutsInhibitManager> m_manager;
};

#endif
