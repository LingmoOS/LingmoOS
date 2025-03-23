// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSHADOWNODE_P_H
#define DSHADOWNODE_P_H

#include <dtkdeclarative_global.h>

#include <private/qsgadaptationlayer_p.h>

#include <QSGVertexColorMaterial>
#include <QSGGeometry>

DQUICK_BEGIN_NAMESPACE

class ShadowMaterial : public QSGVertexColorMaterial
{
public:
    ShadowMaterial();

    QColor color() const { return  m_color; }
    void setColor(const QColor &color) { m_color = color; }

    qreal relativeSizeX() const { return m_relativeSizeX; }
    void setRelativeSizeX(qreal x) { m_relativeSizeX = x; }

    qreal relativeSizeY() const { return m_relativeSizeY; }
    void setRelativeSizeY(qreal y) { m_relativeSizeY = y; }

    qreal spread() const { return m_spread; }
    void setSpread(qreal spread) { m_spread = spread; }

    QSGMaterialType *type() const override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader *createShader() const override;
#else
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;
#endif

    int compare(const QSGMaterial *) const override;

private:
    QColor m_color;
    qreal  m_relativeSizeX;
    qreal  m_relativeSizeY;
    qreal  m_spread;
};

struct ShadowVertex
{
    float m_x;
    float m_y;
    float m_tx;
    float m_ty;

    void set(float x, float y, float tx, float ty)
    {
        m_x = x; m_y = y; m_tx = tx; m_ty = ty;
    }
};

class ShadowNode : public QSGVisitableNode
{
public:
    ShadowNode();

    void setRect(const QRectF &);
    void setColor(const QColor &color);
    void setGlowRadius(qreal radius);
    void setRelativeSizeX(qreal x);
    void setRelativeSizeY(qreal y);
    void setSpread(qreal spread);
    void setFill(bool);
    void update();
    void updateGeometry();
    bool geometryIsDirty() const { return m_geometryChanged; }
    void accept(QSGNodeVisitorEx *visitor) override { if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this); }
private:
    static const QSGGeometry::AttributeSet &shadowAttributes()
    {
        static QSGGeometry::Attribute data[] = {
            QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
            QSGGeometry::Attribute::create(1, 2, GL_FLOAT)
        };
        static QSGGeometry::AttributeSet attributes = { 2, sizeof(ShadowVertex), data };
        return attributes;
    }

private:
    ShadowMaterial m_material;
    QSGGeometry m_geometry { shadowAttributes(), 0 };

    QRectF m_rect;
    bool m_geometryChanged = true;
    qreal m_glowRadius;
    bool m_fill;
};

DQUICK_END_NAMESPACE

#endif // DSHADOWNODE_P_H
