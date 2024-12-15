// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dshadownode_p.h"

#include <QSGMaterialShader>

DQUICK_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
class ShadowMaterialShader : public QSGMaterialShader
{
    const char *vertexShader() const override;
    const char *fragmentShader() const override;
    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *) override;
    char const *const *attributeNames() const override;

private:
    void initialize() override;
#if QT_CONFIG(opengl)
    int m_matrix_id;
    int m_opacity_id;
#endif
};

const char *ShadowMaterialShader::vertexShader() const
{
    return "uniform highp mat4 qt_Matrix;                      \n"
           "attribute highp vec4 qt_VertexPosition;            \n"
           "attribute highp vec2 qt_VertexTexCoord;            \n"
           "varying highp vec2 qt_TexCoord;                    \n"
           "void main() {                                      \n"
           "    qt_TexCoord = qt_VertexTexCoord;               \n"
           "    gl_Position = qt_Matrix * qt_VertexPosition;   \n"
           "}";
}

const char *ShadowMaterialShader::fragmentShader() const
{
    return "uniform highp float qt_Opacity;                                              \n"
           "uniform mediump float relativeSizeX;                                         \n"
           "uniform mediump float relativeSizeY;                                         \n"
           "uniform mediump float spread;                                                \n"
           "uniform lowp vec4 color;                                                     \n"
           "varying highp vec2 qt_TexCoord;                                              \n"
           "highp float linearstep(highp float e0, highp float e1, highp float x) {      \n"
           "    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);                            \n"
           "}                                                                            \n"
           "void main() {                                                                \n"
           "    lowp float alpha =                                                       \n"
           "        smoothstep(0.0, relativeSizeX, 0.5 - abs(0.5 - qt_TexCoord.x)) *     \n"
           "        smoothstep(0.0, relativeSizeY, 0.5 - abs(0.5 - qt_TexCoord.y));      \n"
           "highp float spreadMultiplier = linearstep(spread, 1.0 - spread, alpha);      \n"
           "    gl_FragColor = color * qt_Opacity * spreadMultiplier * spreadMultiplier; \n"
           "}";
}

void ShadowMaterialShader::updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *)
{
    ShadowMaterial *newSurface = static_cast<ShadowMaterial *>(newEffect);
#if QT_CONFIG(opengl)
    program()->setUniformValue("color", newSurface->color());
    program()->setUniformValue("relativeSizeX", (GLfloat)newSurface->relativeSizeX());
    program()->setUniformValue("relativeSizeY", (GLfloat)newSurface->relativeSizeY());
    program()->setUniformValue("spread", (GLfloat)newSurface->spread());
    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacity_id, state.opacity());

    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrix_id, state.combinedMatrix());
#else
    Q_UNUSED(state)
    Q_UNUSET(newSuface)
#endif
}

const char * const *ShadowMaterialShader::attributeNames() const
{
    static const char *attributes[] = {
        "qt_VertexPosition",
        "qt_VertexTexCoord",
        nullptr
    };
    return attributes;
}

void ShadowMaterialShader::initialize()
{
#if QT_CONFIG(opengl)
    m_matrix_id = program()->uniformLocation("qt_Matrix");
    m_opacity_id = program()->uniformLocation("qt_Opacity");
#endif
}
#else

// Mapping shader's ubuf of shadowmaterial
// NOTE: The byte alignment in`glsl` and `cpp` are different.
enum Ubuf {
    QtMatrixSize = 64,
    QtOpacitySize = 4,
    ColorSize = 16,
    RelativeSizeXSize = 4,
    RelativeSizeYSize = 4,
    SpreadSize = 4,

    QtMatrixOffset = 0,
    QtOpacityOffset = QtMatrixOffset + QtMatrixSize,
    RelativeSizeXOffset = QtOpacityOffset + QtOpacitySize,
    RelativeSizeYOffset = RelativeSizeXOffset + RelativeSizeXSize,
    SpreadOffset = RelativeSizeYOffset + RelativeSizeYSize,
    ColorOffset = SpreadOffset + SpreadSize,
    TotalSize = ColorOffset + ColorSize
};

static inline QColor qsg_premultiply_color(const QColor &c)
{
    return QColor::fromRgbF(c.redF() * c.alphaF(), c.greenF() * c.alphaF(), c.blueF() * c.alphaF(), c.alphaF());
}
class ShadowMaterialShader : public QSGMaterialShader
{
public:
    ShadowMaterialShader() {
        setShaderFileName(QSGMaterialShader::VertexStage, QStringLiteral(":/dtk/declarative/shaders_ng/shadowmaterial.vert.qsb"));
        setShaderFileName(QSGMaterialShader::FragmentStage, QStringLiteral(":/dtk/declarative/shaders_ng/shadowmaterial.frag.qsb"));
    }
public:
    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
    {
        bool changed = false;
        QByteArray *buf = state.uniformData();
        Q_ASSERT(buf->size() >= TotalSize);

        if (state.isMatrixDirty()) {
            const QMatrix4x4 m = state.combinedMatrix();
            memcpy(buf->data() + QtMatrixOffset, m.constData(), QtMatrixSize);
            changed = true;
        }

        if (state.isOpacityDirty()) {
            const float opacity = state.opacity();
            memcpy(buf->data() + QtOpacityOffset, &opacity, QtOpacitySize);
            changed = true;
        }

        ShadowMaterial *customMaterial = static_cast<ShadowMaterial *>(newMaterial);
        if (oldMaterial != newMaterial) {
            const QColor color = qsg_premultiply_color(customMaterial->color());
            QVector4D c(color.redF(),
                        color.greenF(),
                        color.blueF(),
                        color.alphaF());
            const float relativeSizeX = customMaterial->relativeSizeX();
            const float relativeSizeY = customMaterial->relativeSizeY();
            const float spread = customMaterial->spread();
            memcpy(buf->data() + ColorOffset, &c, ColorSize);
            memcpy(buf->data() + RelativeSizeXOffset, &relativeSizeX, RelativeSizeXSize);
            memcpy(buf->data() + RelativeSizeYOffset, &relativeSizeY, RelativeSizeYSize);
            memcpy(buf->data() + SpreadOffset, &spread, SpreadSize);
            changed = true;
        }

        return changed;
    }
};
#endif

ShadowMaterial::ShadowMaterial()
{
    setFlag(Blending, true);
}

QSGMaterialType *ShadowMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

QSGMaterialShader *ShadowMaterial::createShader() const
{
    return new ShadowMaterialShader;
}
#else
QSGMaterialShader *ShadowMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    Q_UNUSED(renderMode)
    return new ShadowMaterialShader;
}
#endif

int ShadowMaterial::compare(const QSGMaterial *) const
{
    return 0;
}

ShadowNode::ShadowNode()
{
    setMaterial(&m_material);
    setGeometry(&m_geometry);

    m_geometry.setDrawingMode(QSGGeometry::DrawTriangles);
}

void ShadowNode::setRect(const QRectF &r)
{
    if (r == m_rect)
        return;

    m_geometryChanged = true;
    m_rect = r;
}

void ShadowNode::setColor(const QColor &color)
{
    if (color == m_material.color())
        return;

    m_material.setColor(color);
    markDirty(DirtyMaterial);
}

void ShadowNode::setGlowRadius(qreal radius)
{
    if (qFuzzyCompare(m_glowRadius, radius))
        return;

    m_geometryChanged = true;
    m_glowRadius = radius;
}

void ShadowNode::setRelativeSizeX(qreal x)
{
    if (qFuzzyCompare(x, m_material.relativeSizeX()))
        return;

    m_material.setRelativeSizeX(x);
    markDirty(DirtyMaterial);
}

void ShadowNode::setRelativeSizeY(qreal y)
{
    if (qFuzzyCompare(y, m_material.relativeSizeY()))
        return;

    m_material.setRelativeSizeY(y);
    markDirty(DirtyMaterial);
}

void ShadowNode::setSpread(qreal spread)
{
    if (qFuzzyCompare(spread, m_material.spread()))
        return;

    m_material.setSpread(spread);
    markDirty(DirtyMaterial);
}

void ShadowNode::setFill(bool isFill)
{
    if (m_fill == isFill)
        return;

    m_geometryChanged |= true;
    m_fill = isFill;
}

void ShadowNode::update()
{
    if (m_geometryChanged) {
        updateGeometry();
        markDirty(QSGNode::DirtyGeometry);
        m_geometryChanged = false;
    }
}

void ShadowNode::updateGeometry()
{
    qreal radius = std::min({ m_rect.width() / 2, m_rect.height() / 2, m_glowRadius });

    const float innerL = m_rect.left();
    const float innerR = m_rect.right();
    const float innerT = m_rect.top();
    const float innerB = m_rect.bottom();

    const float xScale = radius / m_rect.width();
    const float yScale = radius / m_rect.height();

    m_geometry.setDrawingMode(m_fill ? QSGGeometry::DrawTriangleStrip : QSGGeometry::DrawTriangles);
    m_geometry.allocate(m_fill ? 4 : 24);
    ShadowVertex *vertices = static_cast<ShadowVertex *>(m_geometry.vertexData());

    if (m_fill) {
        vertices[0].set(innerL, innerT, 0, 0);
        vertices[1].set(innerL, innerB, 0, 1);
        vertices[2].set(innerR, innerT, 1, 0);
        vertices[3].set(innerR, innerB, 1, 1);
        return;
    }

    // top
    //      (0)************************(2)(4)
    //         *                      *
    //         *                      *
    //         *                      *
    //   (1)(5)************************(3)
    vertices[0].set(innerL, innerT, 0, 0);
    vertices[1].set(innerL , innerT + radius, 0, yScale);
    vertices[2].set(innerR, innerT, 1, 0);
    vertices[3].set(innerR, innerT + radius, 1, yScale);
    vertices[4].set(innerR, innerT, 1, 0);
    vertices[5].set(innerL , innerT + radius, 0, yScale);

    // right
    //      (6)* * * * *(8)(10)
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //  (7)(11)* * * * *(9)
    vertices[6].set(innerR - radius, innerT + radius, 1 - xScale, yScale);
    vertices[7].set(innerR - radius, innerB - radius, 1 - xScale, 1 - yScale);
    vertices[8].set(innerR, innerT + radius, 1, yScale);
    vertices[9].set(innerR, innerB - radius, 1, 1 - yScale);
    vertices[10].set(innerR, innerT + radius, 1, yScale);
    vertices[11].set(innerR - radius, innerB - radius, 1 - xScale, 1 - yScale);

    // bottom
    //     (12)************************(14)(16)
    //         *                      *
    //         *                      *
    //         *                      *
    //  (13)(17)************************(15)
    vertices[12].set(innerL, innerB - radius, 0, 1 - yScale);
    vertices[13].set(innerL, innerB, 0, 1);
    vertices[14].set(innerR, innerB - radius, 1, 1 - yScale);
    vertices[15].set(innerR, innerB, 1, 1);
    vertices[16].set(innerR, innerB - radius, 1, 1 - yScale);
    vertices[17].set(innerL, innerB, 0, 1);

    // left
    //      (18)* * * * *(20)(22)
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //         *       *
    //  (19)(23)* * * * *(21)
    vertices[18].set(innerL, innerT + radius, 0, yScale);
    vertices[19].set(innerL, innerB - radius, 0, 1 - yScale);
    vertices[20].set(innerL + radius, innerT + radius, xScale, yScale);
    vertices[21].set(innerL + radius, innerB - radius, xScale, 1 - yScale);
    vertices[22].set(innerL + radius, innerT + radius, xScale, yScale);
    vertices[23].set(innerL, innerB - radius, 0, 1 - yScale);
}

DQUICK_END_NAMESPACE
