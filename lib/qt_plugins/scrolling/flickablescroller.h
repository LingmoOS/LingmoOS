// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_FLICKABLESCROLLER_H
#define LINGMO_FLICKABLESCROLLER_H

#include "scroller.h"

class QQuickFlickable;

namespace Lingmo {

// Touchpad scrolling of a Flickable, reported as Flickable movement.
class FlickableScroller : public Scroller
{
    Q_OBJECT

public:
    explicit FlickableScroller(QQuickFlickable *flickable);

    QQuickFlickable *flickable() const;

protected:
    Axis horizontalAxis() const override;
    Axis verticalAxis() const override;
    QPointF position() const override;
    void applyPosition(const QPointF &position) override;
    void reportMoving(bool moving, Qt::Orientations axes) override;
    void stopViewMotion() override;
    bool isViewAlive() const override;

private:
    QPointer<QQuickFlickable> m_flickable;
};

} // namespace Lingmo

#endif // LINGMO_FLICKABLESCROLLER_H
