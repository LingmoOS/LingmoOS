// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dockpanel.h"
#include "dsglobal.h"

#include <QObject>


namespace dock {
class DockHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(HideState hideState READ hideState NOTIFY hideStateChanged FINAL)

public:
    [[nodiscard]] DockHelper* getHelper(DockPanel* parent);

    virtual HideState hideState() = 0;

Q_SIGNALS:
    void hideStateChanged();

public Q_SLOTS:
    virtual void updateDockTriggerArea() = 0;

protected:
    DockHelper(DockPanel* parent);
    DockPanel* parent();

private:
    DockPanel* m_panel;
};
}

