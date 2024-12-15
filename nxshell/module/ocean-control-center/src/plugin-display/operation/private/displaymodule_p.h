// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DISPLAYMODULE_P_H
#define DISPLAYMODULE_P_H

#include <QObject>

namespace oceanuiV25 {
class DisplayWorker;
class DisplayModel;
class DisplayModule;
class OceanUIScreen;

class DisplayModulePrivate
{

public:
    explicit DisplayModulePrivate(DisplayModule *parent);

    virtual ~DisplayModulePrivate() { }

    void init();
    void updateVirtualScreens();
    void updateMonitorList();
    void updatePrimary();
    void updateDisplayMode();
    OceanUIScreen *primary() const;
    QString displayMode() const;

public:
    DisplayModule *q_ptr;
    DisplayModel *m_model;
    DisplayWorker *m_worker;
    QList<OceanUIScreen *> m_screens;
    QList<OceanUIScreen *> m_virtualScreens;
    OceanUIScreen *m_primary;
    QString m_displayMode;

    Q_DECLARE_PUBLIC(DisplayModule)
};
} // namespace oceanuiV25
#endif // DISPLAYMODULE_P_H
