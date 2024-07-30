/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "types.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandConfig;
class WaylandOutputDevice;

class WaylandScreen : public QObject
{
    Q_OBJECT

public:
    explicit WaylandScreen(WaylandConfig *config);
    ~WaylandScreen() override = default;

    KScreen::ScreenPtr toKScreenScreen(KScreen::ConfigPtr &parent) const;
    void updateKScreenScreen(KScreen::ScreenPtr &screen) const;
    void setOutputs(const QList<WaylandOutputDevice *> &outputs);

private:
    QSize m_size;
    int m_outputCount;
};

}
