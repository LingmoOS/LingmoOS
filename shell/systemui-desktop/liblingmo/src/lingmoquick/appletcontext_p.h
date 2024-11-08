/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "sharedqmlengine.h"

#include <QQmlContext>
#include <qstringlistmodel.h>

namespace Lingmo
{
class Applet;
}

namespace LingmoQuick
{

class AppletContext : public QQmlContext
{
    Q_OBJECT
public:
    AppletContext(QQmlEngine *engine, Lingmo::Applet *applet, SharedQmlEngine *parent);
    ~AppletContext();

    Lingmo::Applet *applet() const;
    SharedQmlEngine *sharedQmlEngine() const;

private:
    Lingmo::Applet *m_applet;
    SharedQmlEngine *m_sharedEngine;
};

}
