// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "homescreen.h"

#include <QObject>

#include <Lingmo/Applet>

// keeps a list of all of instances of Lingmo::Applet that are loaded into the containment
// allows for FolioWidgets to find their corresponding Lingmo::Applet
class WidgetsManager : public QObject
{
    Q_OBJECT
public:
    WidgetsManager(QObject *parent = nullptr);

    Lingmo::Applet *getWidget(int id);

    void addWidget(Lingmo::Applet *applet);
    void removeWidget(Lingmo::Applet *applet);

Q_SIGNALS:
    void widgetAdded(Lingmo::Applet *applet);
    void widgetRemoved(Lingmo::Applet *applet);

private:
    QList<Lingmo::Applet *> m_widgets;
};
