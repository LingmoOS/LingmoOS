/*
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <config-powerdevil.h>

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QScopedPointer>
#include <QTimer>

#if HAVE_XCB
#include <xcb/xcb.h>
#endif

#include "powerdevilcore_export.h"

namespace PowerDevil
{
class POWERDEVILCORE_EXPORT KWinKScreenHelperEffect : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit KWinKScreenHelperEffect(QObject *parent = nullptr);
    ~KWinKScreenHelperEffect() override;

    enum State {
        NormalState,
        FadingOutState,
        FadedOutState,
        FadingInState,
    };

    bool start();

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

public Q_SLOTS:
    void stop();

Q_SIGNALS:
    void stateChanged(State state);
    void fadedOut();

private:
    bool checkValid();
    void setEffectProperty(long value);

    State m_state = NormalState;
    bool m_isValid = false;
    bool m_running = false;

    QTimer m_abortTimer;

#if HAVE_XCB
    xcb_atom_t m_atom = 0;
#endif
};

} // namespace PowerDevil
