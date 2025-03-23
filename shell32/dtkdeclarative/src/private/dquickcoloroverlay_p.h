// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKCOLOROVERLAY_P_H
#define DQUICKCOLOROVERLAY_P_H

#include <dtkdeclarative_global.h>

#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickColorOverlayPrivate;
class DQuickColorOverlay : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(DQuickColorOverlay)
    Q_DECLARE_PRIVATE(DQuickColorOverlay)
    Q_PROPERTY(QQuickItem *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool cached READ cached WRITE setCached NOTIFY cachedChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(SoftwareColorOverlay)
#endif

public:
    explicit DQuickColorOverlay(QQuickItem *parent = nullptr);

    QColor color() const;
    void setColor(const QColor &color);

    QQuickItem *source() const;
    void setSource(QQuickItem *item);

    void setCached(bool cached);
    bool cached() const;

Q_SIGNALS:
    void colorChanged();
    void sourceChanged();
    void cachedChanged();

protected:
    DQuickColorOverlay(DQuickColorOverlayPrivate &dd, QQuickItem *parent = nullptr);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
};

DQUICK_END_NAMESPACE

#endif // DQUICKCOLOROVERLAY_P_H
