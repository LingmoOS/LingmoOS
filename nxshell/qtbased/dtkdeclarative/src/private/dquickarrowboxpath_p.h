// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKARROWBOXPATH_H
#define DQUICKARROWBOXPATH_H

#include <dtkdeclarative_global.h>
#include <private/dqmlglobalobject_p.h>

#include <private/qquickpath_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickArrowBoxPath : public QQuickCurve
{
    Q_OBJECT
    Q_PROPERTY(Direction arrowDirection READ arrowDirection WRITE setArrowDirection NOTIFY arrowDirectionChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(qreal arrowX READ arrowX WRITE setArrowX NOTIFY arrowXChanged)
    Q_PROPERTY(qreal arrowY READ arrowY WRITE setArrowY NOTIFY arrowYChanged)
    Q_PROPERTY(qreal arrowWidth READ arrowWidth WRITE setArrowWidth NOTIFY arrowWidthChanged)
    Q_PROPERTY(qreal arrowHeight READ arrowHeight WRITE setArrowHeight NOTIFY arrowHeightChanged)
    Q_PROPERTY(qreal roundedRadius READ roundedRadius WRITE setRoundedRadius NOTIFY roundedRadiusChanged)
    Q_PROPERTY(qreal spread READ spread WRITE setSpread NOTIFY spreadChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(ArrowBoxPath)
#endif

public:
    enum Direction {
        Up,
        Down,
        Left,
        Right
    };
    Q_ENUM(Direction)

    explicit DQuickArrowBoxPath(QObject *parent = nullptr);

    qreal width() const;
    void setWidth(qreal newWidth);

    qreal height() const;
    void setHeight(qreal newHeight);

    const Direction &arrowDirection() const;
    void setArrowDirection(const Direction &newArrowDirection);

    qreal arrowX() const;
    void setArrowX(qreal newArrowX);

    qreal arrowY() const;
    void setArrowY(qreal newArrowY);

    qreal arrowWidth() const;
    void setArrowWidth(qreal newArrowWidth);

    qreal arrowHeight() const;
    void setArrowHeight(qreal newArrowHeight);

    qreal roundedRadius() const;
    void setRoundedRadius(qreal newRoundedRadius);

    qreal spread() const;
    void setSpread(qreal newSpread);

Q_SIGNALS:
    void widthChanged();
    void heightChanged();
    void arrowDirectionChanged();
    void arrowXChanged();
    void arrowYChanged();
    void arrowWidthChanged();
    void arrowHeightChanged();
    void roundedRadiusChanged();

    void spreadChanged();

private:
    void addToPath(QPainterPath &path, const QQuickPathData &) override;

    Direction m_arrowDirection;
    qreal m_width;
    qreal m_height;
    qreal m_arrowX;
    qreal m_arrowY;
    qreal m_arrowWidth;
    qreal m_arrowHeight;
    qreal m_roundedRadius;
    qreal m_spread;
};

DQUICK_END_NAMESPACE

#endif // DQUICKARROWBOXPATH_H
