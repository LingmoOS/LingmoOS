// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickinwindowblur_p.h"
#include "private/dblitframebuffernode_p.h"
#include "private/dblurimagenode_p.h"

#include <private/qquickitem_p.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <private/qsgplaintexture_p.h>
#else
#include <private/qsgtexture_p.h>
#endif

#include <QQuickWindow>

#include <DGuiApplicationHelper>
DGUI_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN InWindowBlurTextureProvider : public QSGTextureProvider {
public:
    InWindowBlurTextureProvider()
        : QSGTextureProvider()
        , m_texture(new QSGPlainTexture())
    {
        m_texture->setOwnsTexture(false);
    }

    inline QSGPlainTexture *plainTexture() const {
        return m_texture.data();
    }
    inline QSGTexture *texture() const override {
        return plainTexture();
    }

private:
    QScopedPointer<QSGPlainTexture> m_texture;
};

DQuickInWindowBlur::DQuickInWindowBlur(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    // TODO, `update` should be called when QSGRenderNode changed.
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this](){
        update();
    });
}

DQuickInWindowBlur::~DQuickInWindowBlur()
{
    DQuickInWindowBlur::releaseResources();
}

qreal DQuickInWindowBlur::radius() const
{
    return m_radius;
}

void DQuickInWindowBlur::setRadius(qreal newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;

    m_radius = newRadius;
    update();
}

bool DQuickInWindowBlur::offscreen() const
{
    return m_offscreen;
}

void DQuickInWindowBlur::setOffscreen(bool newOffscreen)
{
    if (m_offscreen == newOffscreen)
        return;
    m_offscreen = newOffscreen;
    Q_EMIT offscreenChanged();

    update();
}

QSGTextureProvider *DQuickInWindowBlur::textureProvider() const
{
    const QQuickItemPrivate *d = QQuickItemPrivate::get(this);
    if (!d->window || !d->sceneGraphRenderContext() || QThread::currentThread() != d->sceneGraphRenderContext()->thread()) {
        qWarning("DQuickInWindowBlendBlur::textureProvider: can only be queried on the rendering thread of an exposed window");
        return nullptr;
    }

    if (!m_tp) {
        m_tp = new InWindowBlurTextureProvider();
    }
    return m_tp;
}

static void updateBlurNodeTexture(DBlitFramebufferNode *node, void *blurNode) {
    auto bn = reinterpret_cast<DSGBlurNode*>(blurNode);
    bn->setTexture(node->texture());
}

void onRender(DSGBlurNode *node, void *data) {
    DQuickInWindowBlur *that = reinterpret_cast<DQuickInWindowBlur*>(data);
    if (!that || !that->m_tp)
        return;
    node->writeToTexture(that->m_tp->plainTexture());
    // Don't direct emit the signal, must ensure the signal emit on current render loop after.
    that->m_tp->metaObject()->invokeMethod(that->m_tp, &InWindowBlurTextureProvider::textureChanged, Qt::QueuedConnection);
}

QSGNode *DQuickInWindowBlur::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data)

    auto node = static_cast<DBlitFramebufferNode*>(oldNode);
    const QSGRendererInterface::GraphicsApi ga = window()->rendererInterface()->graphicsApi();
    if (Q_UNLIKELY(!node)) {
        if (ga == QSGRendererInterface::Software) {
            node = DBlitFramebufferNode::createSoftwareNode(this, true, true);
            auto blurNode = new DSoftwareBlurImageNode(this);
            blurNode->setDisabledOpaqueRendering(true);
            blurNode->setRenderCallback(onRender, this);
            node->appendChildNode(blurNode);
            node->setRenderCallback(updateBlurNodeTexture, blurNode);
        }
#ifndef QT_NO_OPENGL
        else if (ga == QSGRendererInterface::OpenGL
         #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                 || ga == QSGRendererInterface::OpenGLRhi
         #endif
                 ) {
            node = DBlitFramebufferNode::createOpenGLNode(this, true, true);
            auto blurNode = new DOpenGLBlurEffectNode(this);
            blurNode->setDisabledOpaqueRendering(true);
            blurNode->setRenderCallback(onRender, this);
            node->appendChildNode(blurNode);
            node->setRenderCallback(updateBlurNodeTexture, blurNode);
        }
#endif
        else {
            qWarning() << "Not supported graphics API:" << ga;
            return nullptr;
        }
    }

    if (!m_tp) {
        m_tp = new InWindowBlurTextureProvider();
    }

    node->resize(size());
    DSGBlurNode *blurNode = static_cast<DSGBlurNode*>(node->firstChild());
    Q_ASSERT(blurNode);
    blurNode->setRadius(m_radius);
    blurNode->setWindow(window());
    const QRectF rect(0, 0, width(), height());
    blurNode->setSourceRect(rect);
    blurNode->setRect(rect);
    blurNode->setTexture(node->texture());
    blurNode->setFollowMatrixForSource(true);
    blurNode->setOffscreen(m_offscreen);

    return node;
}

void DQuickInWindowBlur::itemChange(ItemChange type, const ItemChangeData &data)
{
    if (type == ItemDevicePixelRatioHasChanged) {
        update();
    }

    QQuickItem::itemChange(type, data);
}

void DQuickInWindowBlur::releaseResources()
{
    if (m_tp) {
        QQuickWindowQObjectCleanupJob::schedule(window(), m_tp);
        m_tp = nullptr;
    }
}

void DQuickInWindowBlur::invalidateSceneGraph()
{
    delete m_tp;
    m_tp = nullptr;
}

DQUICK_END_NAMESPACE
