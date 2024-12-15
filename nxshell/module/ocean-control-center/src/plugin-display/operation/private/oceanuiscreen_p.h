// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCSCREEN_P_H
#define DCCSCREEN_P_H

#include "monitor.h"

#include <QObject>

namespace oceanuiV25 {
class DisplayWorker;
class OceanUIScreen;

class OceanUIScreenPrivate
{
public:
    static OceanUIScreen *New(QList<Monitor *> monitors, DisplayWorker *worker, QObject *parent = nullptr);
    static OceanUIScreenPrivate *Private(OceanUIScreen *screen);

    explicit OceanUIScreenPrivate(OceanUIScreen *screen);
    virtual ~OceanUIScreenPrivate();
    void setMonitors(QList<Monitor *> monitors);
    Monitor *monitor();
    QList<Monitor *> monitors();

    void setMode(QSize resolution, double rate);
    void setRotate(uint rotate);
    void setFillMode(const QString &fileMode);

    void updateResolutionList();
    void updateRateList();
    void updateScreen();

private:
    OceanUIScreen *q_ptr;
    QList<Monitor *> m_monitors;
    QList<QSize> m_resolutionList;
    QList<double> m_rateList;
    QString m_name;
    DisplayWorker *m_worker;
    QScreen *m_screen;
    Q_DECLARE_PUBLIC(OceanUIScreen)
};
} // namespace oceanuiV25
#endif // DCCSCREEN_P_H
