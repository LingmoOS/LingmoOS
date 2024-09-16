// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKRECTANGLE_P_H
#define DQUICKRECTANGLE_P_H

#include <dtkdeclarative_global.h>
#include <DObject>

#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickRectanglePrivate;
class Q_DECL_EXPORT DQuickRectangle : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(Corners corners READ corners WRITE setCorners NOTIFY cornersChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(RoundRectangle)
#endif

public:
    enum Corner {
        NoneCorner = 0x0,
        TopLeftCorner = 0x1,
        TopRightCorner = 0x2,
        BottomLeftCorner = 0x4,
        BottomRightCorner = 0x8,
        TopCorner = TopLeftCorner | TopRightCorner,
        BottomCorner = BottomLeftCorner| BottomRightCorner,
        LeftCorner = TopLeftCorner| BottomLeftCorner,
        RightCorner = TopRightCorner| BottomRightCorner,
        AllCorner = TopCorner | BottomCorner
    };
    Q_DECLARE_FLAGS(Corners, Corner)
    Q_FLAG(Corners)

    explicit DQuickRectangle(QQuickItem *parent = nullptr);
    ~DQuickRectangle();

    QColor color() const;
    void setColor(const QColor &color);

    qreal radius() const;
    void setRadius(qreal radius);

    DQuickRectangle::Corners corners() const;
    void setCorners(Corners corners);

private Q_SLOTS:
    void invalidateSceneGraph();

Q_SIGNALS:
    void colorChanged();
    void radiusChanged();
    void cornersChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

protected:
    DQuickRectangle(DQuickRectanglePrivate &dd, QQuickItem *parent = nullptr);

private:
    Q_DISABLE_COPY(DQuickRectangle)
    Q_DECLARE_PRIVATE(DQuickRectangle)

    void releaseResources() override;
};

DQUICK_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DTK_QUICK_NAMESPACE::DQuickRectangle::Corners)
Q_DECLARE_METATYPE(DTK_QUICK_NAMESPACE::DQuickRectangle::Corner)

#endif // DQUICKRECTANGLE_P_H
