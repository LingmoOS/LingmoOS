// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKGLOW_P_H
#define DQUICKGLOW_P_H

#include <dtkdeclarative_global.h>
#include <DObject>

#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickGlowPrivate;
class Q_DECL_EXPORT DQuickGlow : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal glowRadius READ glowRadius WRITE setGlowRadius NOTIFY glowRadiusChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal spread READ spread WRITE setSpread NOTIFY spreadChanged)
    Q_PROPERTY(qreal relativeSizeX READ relativeSizeX WRITE setRelativeSizeX NOTIFY relativeSizeXChanged)
    Q_PROPERTY(qreal relativeSizeY READ relativeSizeY WRITE setRelativeSizeY NOTIFY relativeSizeYChangd)
    Q_PROPERTY(bool fill READ fill WRITE setFill NOTIFY fillChanged)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(GlowEffect)
#endif
public:
    explicit DQuickGlow(QQuickItem *parent = nullptr);

    qreal glowRadius() const;
    void setGlowRadius(qreal radius);

    QColor color() const;
    void setColor(const QColor &);

    qreal spread() const;
    void setSpread(qreal radius);

    qreal relativeSizeX() const;
    void setRelativeSizeX(qreal x);

    qreal relativeSizeY() const;
    void setRelativeSizeY(qreal y);

    bool fill();
    void setFill(bool);
Q_SIGNALS:
    void glowRadiusChanged();
    void colorChanged();
    void spreadChanged();
    void relativeSizeXChanged();
    void relativeSizeYChangd();
    void fillChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

protected:
    DQuickGlow(DQuickGlowPrivate &dd, QQuickItem *parent = nullptr);

private:
    Q_DISABLE_COPY(DQuickGlow)
    Q_DECLARE_PRIVATE(DQuickGlow)
};

DQUICK_END_NAMESPACE

#endif // DQUICKGLOW_P_H
