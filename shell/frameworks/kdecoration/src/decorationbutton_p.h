/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decorationbutton.h"

#include <QPointer>

class QElapsedTimer;
class QTimer;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration2 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace KDecoration2
{
class Q_DECL_HIDDEN DecorationButton::Private
{
public:
    explicit Private(DecorationButtonType type, const QPointer<Decoration> &decoration, DecorationButton *parent);
    ~Private();

    bool isPressed() const
    {
        return m_pressed != Qt::NoButton;
    }
    bool isPressed(Qt::MouseButton button) const
    {
        return m_pressed.testFlag(button);
    }

    void setHovered(bool hovered);
    void setPressed(Qt::MouseButton, bool pressed);
    void setAcceptedButtons(Qt::MouseButtons buttons);
    void setEnabled(bool enabled);
    void setChecked(bool checked);
    void setCheckable(bool checkable);
    void setVisible(bool visible);
    void startDoubleClickTimer();
    void invalidateDoubleClickTimer();
    bool wasDoubleClick() const;
    void setPressAndHold(bool enable);
    void startPressAndHold();
    void stopPressAndHold();

    QString typeToString(DecorationButtonType type);

    QPointer<Decoration> decoration;
    DecorationButtonType type;
    QRectF geometry;
    bool hovered;
    bool enabled;
    bool checkable;
    bool checked;
    bool visible;
    Qt::MouseButtons acceptedButtons;
    bool doubleClickEnabled;
    bool pressAndHold;

private:
    void init();
    DecorationButton *q;
    Qt::MouseButtons m_pressed;
    std::unique_ptr<QElapsedTimer> m_doubleClickTimer;
    std::unique_ptr<QTimer> m_pressAndHoldTimer;
};

}
