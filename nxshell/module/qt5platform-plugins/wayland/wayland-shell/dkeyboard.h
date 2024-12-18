// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DKEYBOARD_H
#define DKEYBOARD_H

#include <QObject>
#ifndef D_LINGMO_IS_DWAYLAND
#include <KWayland/Client/oceankeyboard.h>
#else
#include <DWayland/Client/oceankeyboard.h>
#endif
using namespace KWayland::Client;

namespace QtWaylandClient {

class DKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit DKeyboard(QObject *parent = nullptr);
    virtual ~DKeyboard();

public slots:
    void handleKeyEvent(quint32 key, OCEANKeyboard::KeyState state, quint32 time);
    void handleModifiersChanged(quint32 depressed, quint32 latched, quint32 locked, quint32 group);
};

}

#endif // DKEYBOARD_H
