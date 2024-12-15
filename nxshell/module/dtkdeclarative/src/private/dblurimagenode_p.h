// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSOFTWAREBLURIMAGENODE_H
#define DSOFTWAREBLURIMAGENODE_H

#include <dtkdeclarative_global.h>
#include <QSGRenderNode>
#include <QImage>
#include <QPointer>
#ifndef QT_NO_OPENGL
#include <qopengl.h>
#endif

QT_BEGIN_NAMESPACE
class QQuickItem;
class QSGTexture;
class QSGPlainTexture;
class QQuickWindow;
#ifndef QT_NO_OPENGL
class QOpenGLShaderProgram;
class QOpenGLFramebufferObject;
class QOpenGLBuffer;
#endif
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DSGBlurNode : public QSGRenderNode
{
public:
    DSGBlurNode(QQuickItem *owner);

    typedef void(*RenderCallback)(DSGBlurNode *node, void *data);
    void setRenderCallback(RenderCallback callback, void *data);
    inline void doRenderCallback() {
        if (!m_renderCallback)
            return;
        m_renderCallback(this, m_callbackData);
    }

    inline void setOffscreen(bool offscreen) {
        if (m_offscreen == offscreen)
            return;
        m_offscreen = offscreen;
        markDirty(DirtyMaterial);
    }

    virtual void setTexture(QSGTexture *texture);
    inline QSGTexture *texture() const
    { return m_texture;}
    virtual bool writeToTexture(QSGPlainTexture *targetTexture) const;
    virtual void setRadius(qreal radius);
    void setSourceRect(const QRectF &source);
    void setRect(const QRectF &target);
    void setDisabledOpaqueRendering(bool disabled);
    void setBlendColor(const QColor &color);
    void setFollowMatrixForSource(bool on);

    RenderingFlags flags() const override;
    QRectF rect() const override;
    void setWindow(QQuickWindow *window);

protected:
    RenderCallback m_renderCallback = nullptr;
    void *m_callbackData = nullptr;
    bool m_offscreen = false;

    QPointer<QQuickItem> m_item;
    QSGTexture *m_texture = nullptr;
    qreal m_radius = 0;
    QRectF m_sourceRect;
    QRectF m_targetRect;
    QColor m_blendColor = Qt::transparent;
    bool m_disabledOpaqueRendering = false;
    bool m_followMatrixForSource = false;
    QPointer<QQuickWindow> m_window;
};

class DSoftwareBlurImageNode : public DSGBlurNode
{
public:
    DSoftwareBlurImageNode(QQuickItem *owner);

private:
    void render(const RenderState *state) override;
    void releaseResources() override;
    void updateCachedImage();
    bool writeToTexture(QSGPlainTexture *targetTexture) const override;

    QImage cachedSource;
};

#ifndef QT_NO_OPENGL
class DOpenGLBlurEffectNode : public DSGBlurNode
{
public:
    DOpenGLBlurEffectNode(QQuickItem *owner);
    ~DOpenGLBlurEffectNode() override;

    void setTexture(QSGTexture *texture) override;
    void setRadius(qreal radius) override;

    void render(const RenderState *state) override;
    StateFlags changedStates() const override;

private:
    bool writeToTexture(QSGPlainTexture *targetTexture) const override;

    void initialize();
    void initBlurSahder();
    void applyDaulBlur(QOpenGLFramebufferObject* targetFBO, GLuint sourceTexture, QOpenGLShaderProgram *shader
                     , const QSGRenderNode::RenderState *state, int matrixUniform, int scale);
    void applyNoise(GLuint sourceTexture, const QSGRenderNode::RenderState *state);
    void renderToScreen(GLuint sourceTexture, const QSGRenderNode::RenderState *state);
    void initFBOTextures();
    void initDispalyShader();
    void initNoiseShader();

private:
    bool m_needUpdateFBO = false;

    QOpenGLShaderProgram *m_programKawaseUp = nullptr;
    QOpenGLShaderProgram *m_programKawaseDown = nullptr;
    QVector<QOpenGLFramebufferObject*> m_fboVector;
    int m_matrixKawaseUpUniform;
    int m_matrixKawaseDownUniform;
    QOpenGLBuffer *m_sampleVbo = nullptr;

    QOpenGLShaderProgram *m_program = nullptr;
    int m_matrixUniform;
    int m_opacityUniform;
    QOpenGLBuffer *m_vbo = nullptr;

    QOpenGLShaderProgram *m_programNoise = nullptr;
    QOpenGLBuffer *m_noiseVbo = nullptr;
};
#endif

DQUICK_END_NAMESPACE

#endif // DSOFTWAREBLURIMAGENODE_H
