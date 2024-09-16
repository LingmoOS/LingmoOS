// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickitemviewport.h"
#include "private/dquickitemviewport_p.h"
#include "private/dmaskeffectnode_p.h"
#include "private/dsoftwareroundedimagenode_p.h"

#include <QOpenGLFunctions>
#include <QQuickImageProvider>
#include <QtMath>

DQUICK_BEGIN_NAMESPACE

template<typename T>
inline static T *load(const QAtomicPointer<T> &atomicValue) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return atomicValue.load();
#else
    return atomicValue.loadRelaxed();
#endif
}

class Q_DECL_HIDDEN PreprocessNode : public QSGTransformNode {
public:
    PreprocessNode(DQuickItemViewportPrivate *item)
        : item(item)
    {
        setFlag(IsVisitableNode, false);
        item->setPreprocessNode(this);
    }
    ~PreprocessNode() {
        if (auto i = load(item))
            i->clearPreprocessNode(this);
    }

    inline void setImageNode(QSGImageNode *node) {
        Q_ASSERT(!softwareNode);
        Q_ASSERT(!imageNode);
        maskNode = dynamic_cast<MaskEffectNode*>(node);
        imageNode = node;
        appendChildNode(imageNode);
    }

    inline void setImageNode(DSoftwareRoundedImageNode *node) {
        Q_ASSERT(!softwareNode);
        Q_ASSERT(!imageNode);
        softwareNode = node;
        appendChildNode(softwareNode);
    }

    inline void clearImageNode() {
        delete imageNode;
        imageNode = nullptr;
        maskNode = nullptr;
        delete softwareNode;
        softwareNode = nullptr;
    }

    void preprocess() override {
        auto i = load(item);
        if (Q_UNLIKELY(!i))
            return;

        const QPointF newPos = i->q_func()->mapToItem(i->sourceItem.data(), QPointF(0, 0));
        if (!i->updateOffset(newPos))
            return;

        if (imageNode) {
            i->updateSourceRect(imageNode);
            if (maskNode) {
                maskNode->setMaskOffset(i->getMaskOffset());
            }
        } else if (softwareNode) {
            i->updateSourceRect(softwareNode);
        }
    }

    QAtomicPointer<DQuickItemViewportPrivate> item;
    MaskEffectNode *maskNode = nullptr;
    QSGImageNode *imageNode = nullptr;
    DSoftwareRoundedImageNode *softwareNode = nullptr;
};

DQuickItemViewportPrivate::~DQuickItemViewportPrivate()
{
    if (auto node = load(preprocessNode))
        node->item = nullptr;
    // 清理对sourceItem的操作
    initSourceItem(sourceItem, nullptr);
}

void DQuickItemViewportPrivate::initSourceItem(QQuickItem *old, QQuickItem *item)
{
    if (old) {
        QQuickItemPrivate *sd = QQuickItemPrivate::get(old);
        sd->removeItemChangeListener(this, QQuickItemPrivate::Geometry);
        sd->derefFromEffectItem(hideSource);
    }

    // 监听材质变化的信号
    if (textureChangedConnection)
        QObject::disconnect(textureChangedConnection);

    if (item) {
        QQuickItemPrivate *sd = QQuickItemPrivate::get(item);
        sd->addItemChangeListener(this, QQuickItemPrivate::Geometry);
        sd->refFromEffectItem(hideSource);
    }
}

void DQuickItemViewportPrivate::itemGeometryChanged(QQuickItem *, QQuickGeometryChange data, const QRectF &)
{
    D_Q(DQuickItemViewport);
    // 当DQuickItemViewport或其sourceItem的大小改变时应当更新sourceSizeRatio和maskOffset
    if (data.sizeChange()) {
        markDirtys(DirtySourceSizeRatio | DirtyMaskOffset);
        q->update();
    }
}

void DQuickItemViewportPrivate::setPreprocessNode(PreprocessNode *newNode)
{
    Q_ASSERT(load(preprocessNode) == nullptr);
    newNode->item = this;
    preprocessNode = newNode;
    updateUsePreprocess();
}

void DQuickItemViewportPrivate::clearPreprocessNode(PreprocessNode *oldNode)
{
    Q_ASSERT(load(preprocessNode) == oldNode);
    preprocessNode = nullptr;
}

void DQuickItemViewportPrivate::updateUsePreprocess() const
{
    if (auto pn = load(preprocessNode)) {
        pn->setFlag(QSGNode::UsePreprocess, !fixed && (pn->imageNode || pn->softwareNode));
    }
}

void DQuickItemViewportPrivate::ensureTexture()
{
    D_Q(DQuickItemViewport);
    if (texture)
        return;

    Q_ASSERT_X(QQuickItemPrivate::get(q)->window
               && QQuickItemPrivate::get(q)->sceneGraphRenderContext()
               && QThread::currentThread() == QQuickItemPrivate::get(q)->sceneGraphRenderContext()->thread(),
               "DQuickItemViewport",
               "Cannot be used outside the rendering thread");

    QSGRenderContext *rc = QQuickItemPrivate::get(q)->sceneGraphRenderContext();
    texture = rc->sceneGraphContext()->createLayer(rc);
    QObject::connect(QQuickItemPrivate::get(q)->window, SIGNAL(sceneGraphInvalidated()), texture, SLOT(invalidated()), Qt::DirectConnection);
    QObject::connect(texture, SIGNAL(updateRequested()), q, SLOT(update()));
}

/*!
 * \~chinese \class DQuickItemViewport
 * \~chinese \brief DQuickItemViewport 类是根据 \a sourceItem 属性设定的 \a QQuickItem
 * \~chinese 作为绘制时的材质来源，这个行为依赖于 \a QQuickItem::textureProvider 提供组件的
 * \~chinese 材质。故可将qml的 Image 和 ShaderEffectSource 作为 sourceItem 使用。
 * \~chinese
 * \~chinese 源材质的区域受 \a sourceRect 属性控制。此属性的坐标相对于 \a sourceItem，当属性未
 * \~chinese 设置时，坐标默认为 (0, 0)，大小则跟随 DQuickItemViewport。此外，当 \a fixed 的值
 * \~chinese 为 false 时，源材质的位置还会受到 DQuickItemViewport 相对于 \a sourceItem 的位置
 * \~chinese 的影响，即当 \a fixed 为 false 且 \a sourceRect 的坐标为 (0, 0) 时，源材质的位置
 * \~chinese 是 DQuickItemViewport 在窗口坐标系中对 \a sourceItem 的投影，且绘制时也会实时跟随。
 * \~chinese 当 \a radius 属性为 0 时，将直接使用 \a QSGImageNode 作为渲染节点，当其值大于 0
 * \~chinese 时，将使用 QPainter 渲染按 radius 的值绘制一张圆角 QImage 资源，并将其作为 mask
 * \~chinese 材质跟 \a sourceItem 的材质一起运算，并且会使用自定义的片段着色器为材质实现圆角效果。
 * \~chinese \sa QQuickItem::textureProvider
 * \~chinese \sa QQuickItem::isTextureProvider
 */
DQuickItemViewport::DQuickItemViewport(QQuickItem *parent)
    : QQuickItem (parent)
    , DCORE_NAMESPACE::DObject(*new DQuickItemViewportPrivate(this))
{
    setFlag(ItemHasContents);
}

DQuickItemViewport::~DQuickItemViewport()
{
    if (window()) {
        DQuickItemViewport::releaseResources();
    }
}

QQuickItem *DQuickItemViewport::sourceItem() const
{
    D_DC(DQuickItemViewport);
    return d->sourceItem;
}

QRectF DQuickItemViewport::sourceRect() const
{
    D_DC(DQuickItemViewport);
    return d->sourceRect;
}

float DQuickItemViewport::radius() const
{
    D_DC(DQuickItemViewport);
    return d->radius;
}

bool DQuickItemViewport::fixed() const
{
    D_DC(DQuickItemViewport);
    return d->fixed;
}

void DQuickItemViewport::setFixed(bool newFixed)
{
    D_D(DQuickItemViewport);
    if (d->fixed == newFixed)
        return;
    d->fixed = newFixed;
    Q_EMIT fixedChanged();
    update();
}

bool DQuickItemViewport::hideSource() const
{
    D_DC(DQuickItemViewport);
    return d->hideSource;
}

void DQuickItemViewport::setHideSource(bool newHideSource)
{
    D_D(DQuickItemViewport);
    if (d->hideSource == newHideSource)
        return;

    if (d->sourceItem) {
        QQuickItemPrivate::get(d->sourceItem)->refFromEffectItem(newHideSource);
        QQuickItemPrivate::get(d->sourceItem)->derefFromEffectItem(d->hideSource);
    }
    d->hideSource = newHideSource;
    Q_EMIT hideSourceChanged();
}

QSGTextureProvider *DQuickItemViewport::textureProvider() const
{
    D_DC(DQuickItemViewport);
    if (!d->sourceItem)
        return nullptr;

    auto provider = d->sourceItem->textureProvider();
    if (provider)
        return provider;

    if (!d->provider) {
        const_cast<DQuickItemViewport *>(this)->d_func()->provider = new DQuickViewportTextureProvider();
        const_cast<DQuickItemViewport *>(this)->d_func()->ensureTexture();
        connect(d->texture, SIGNAL(updateRequested()), d->provider, SIGNAL(textureChanged()));
        d->provider->sourceTexture = d->texture;
    }

    return d->provider;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QPainter::CompositionMode DQuickItemViewport::compositionMode() const
{
    D_DC(DQuickItemViewport);
    return d->compositionMode;
}

void DQuickItemViewport::setCompositionMode(QPainter::CompositionMode newCompositionMode)
{
    D_D(DQuickItemViewport);
    if (d->compositionMode == newCompositionMode)
        return;

    if (d->compositionMode == DQuickItemViewportPrivate::DefaultCompositionMode
        || newCompositionMode == DQuickItemViewportPrivate::DefaultCompositionMode) {
        d->markDirty(DQuickItemViewportPrivate::DirtyContentNode);
    }

    d->compositionMode = newCompositionMode;
    Q_EMIT compositionModeChanged();
}

void DQuickItemViewport::resetCompositionMode()
{
    setCompositionMode(DQuickItemViewportPrivate::DefaultCompositionMode);
}
#endif

void DQuickItemViewport::invalidateSceneGraph()
{
    D_D(DQuickItemViewport);
    if (d->texture)
        delete d->texture;
    if (d->provider)
        delete d->provider;
    d->texture = nullptr;
    d->maskTexture.reset();
    d->provider = nullptr;
}

void DQuickItemViewport::setSourceItem(QQuickItem *sourceItem)
{
    D_D(DQuickItemViewport);

    if (d->sourceItem == sourceItem)
        return;

    if (isComponentComplete()) {
        d->initSourceItem(d->sourceItem, sourceItem);
    }

    d->sourceItem = sourceItem;
    d->markDirtys(DQuickItemViewportPrivate::DirtySourceSizeRatio
                  | DQuickItemViewportPrivate::DirtyMaskOffset);
    Q_EMIT sourceItemChanged();
    update();
}

void DQuickItemViewport::setSourceRect(const QRectF &sourceRect)
{
    D_D(DQuickItemViewport);
    if (d->sourceRect == sourceRect)
        return;

    if (d->sourceRect.topLeft() != sourceRect.topLeft())
        d->markDirty(DQuickItemViewportPrivate::DirtyMaskOffset);
    if (d->sourceRect.size() != sourceRect.size())
        d->markDirtys(DQuickItemViewportPrivate::DirtySourceSizeRatio
                      | DQuickItemViewportPrivate::DirtyMaskSizeRatio);
    d->sourceRect = sourceRect;
    Q_EMIT sourceRectChanged();
    update();
}

void DQuickItemViewport::setRadius(float radius)
{
    // TODO：目前深浅色融合在着色器中完成，融合不支持圆角为0
    D_D(DQuickItemViewport);
    if (qFuzzyCompare(d->radius, radius))
        return;

    if (qIsNull(d->radius) || qIsNull(radius)) {
        d->markDirty(DQuickItemViewportPrivate::DirtyContentNode);
    }
    d->markDirtys(DQuickItemViewportPrivate::DirtyMaskSizeRatio
                  | DQuickItemViewportPrivate::DirtyMaskTexture);

    d->radius = radius;
    Q_EMIT radiusChanged();
    update();
}

void DQuickItemViewport::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    D_D(DQuickItemViewport);

    switch (static_cast<int>(change)) {
    case ItemAntialiasingHasChanged:
        d->markDirty(DQuickItemViewportPrivate::DirtyMaskTexture);
        update();
        break;
    case ItemDevicePixelRatioHasChanged:
        d->markDirtys(DQuickItemViewportPrivate::DirtyMaskTexture
                      | DQuickItemViewportPrivate::DirtyMaskSizeRatio);
        update();
        break;
    }

    QQuickItem::itemChange(change, data);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void DQuickItemViewport::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    D_D(DQuickItemViewport);
    if (!d->sourceRect.isValid() && newGeometry.size() != oldGeometry.size()) {
        d->markDirty(DQuickItemViewportPrivate::DirtyMaskSizeRatio);
        if (!d->sourceRect.isValid())
            d->markDirty(DQuickItemViewportPrivate::DirtySourceSizeRatio);
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}
#else
void DQuickItemViewport::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    D_D(DQuickItemViewport);
    if (!d->sourceRect.isValid() && newGeometry.size() != oldGeometry.size()) {
        d->markDirty(DQuickItemViewportPrivate::DirtyMaskSizeRatio);
        if (!d->sourceRect.isValid())
            d->markDirty(DQuickItemViewportPrivate::DirtySourceSizeRatio);
    }
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}
#endif

static inline void safeSetMaterialBlending(QSGMaterial *m, bool b) {
    if (m && m != reinterpret_cast<QSGMaterial*>(1)) {
        m->setFlag(QSGMaterial::Blending, b);
    }
}

QSGNode *DQuickItemViewport::updatePaintNode(QSGNode *old, QQuickItem::UpdatePaintNodeData *)
{
    D_D(DQuickItemViewport);

    if (Q_UNLIKELY(!d->sourceItem || d->sourceItem->width() <=0 || d->sourceItem->height() <= 0)) {
        delete old;
        return nullptr;
    }

    const auto tp = this->textureProvider();
    if (Q_LIKELY(!tp)) {
        delete old;
        return nullptr;
    }

    if (auto texture = qobject_cast<QSGLayer *>(tp->texture())) {
        texture->setItem(QQuickItemPrivate::get(d->sourceItem)->itemNode());
        QRectF sourceRect = d->sourceRect.width() == 0 || d->sourceRect.height() == 0
                          ? QRectF(0, 0, d->sourceItem->width(), d->sourceItem->height())
                          : d->sourceRect;
        texture->setRect(sourceRect);
        QSize textureSize = QSize(qCeil(qAbs(sourceRect.width())), qCeil(qAbs(sourceRect.height())));
        QQuickItemPrivate *item_d = static_cast<QQuickItemPrivate *>(QObjectPrivate::get(this));

        if (d->sourceItem)
            textureSize *= item_d->window->effectiveDevicePixelRatio();

        const QSize minTextureSize = item_d->sceneGraphContext()->minimumFBOSize();

        while (textureSize.width() < minTextureSize.width())
            textureSize.rwidth() *= 2;
        while (textureSize.height() < minTextureSize.height())
            textureSize.rheight() *= 2;
        texture->setDevicePixelRatio(item_d->window->effectiveDevicePixelRatio());
        texture->setSize(textureSize);
        texture->updateTexture();
    }

    PreprocessNode *preNode = static_cast<PreprocessNode*>(old);
    if (Q_UNLIKELY(!preNode)) {
        preNode = new PreprocessNode(d);
        if (!d->textureChangedConnection) {
            // 注意不要将此代码移动到别处，有些对象不允许在非渲染线程中获取 textureProvider
            auto onTextureChanged = [this, d, tp] {
                auto preNode = load(d->preprocessNode);
                Q_ASSERT(preNode);
                auto texture = tp->texture();
                if (Q_LIKELY(texture)) {
                    if (Q_LIKELY(preNode->imageNode)) {
                        if (preNode->imageNode->texture() != texture) {
                            preNode->imageNode->setTexture(texture);
                        }
                        // Maybe the texture size is changed
                        d->updateSourceRect(preNode->imageNode);
                    } else if (Q_LIKELY(preNode->softwareNode)) {
                        if (preNode->softwareNode->texture() != texture) {
                            preNode->softwareNode->setTexture(texture);
                        }
                        // Maybe the texture size is changed
                        d->updateSourceRect(preNode->softwareNode);
                    } else {
                        // to create image node
                        update();
                    }
                } else {
                    preNode->clearImageNode();
                    d->updateUsePreprocess();
                }
            };

            if (tp)
                d->textureChangedConnection = QObject::connect(tp,
                                                               &QSGTextureProvider::textureChanged,
                                                               this, onTextureChanged, Qt::DirectConnection);
        }
    }

    if (Q_UNLIKELY(d->dirtyState.testFlag(DQuickItemViewportPrivate::DirtyContentNode))) {
        d->markDirty(DQuickItemViewportPrivate::DirtyContentNode, false);
        preNode->clearImageNode();
    }

    QSGImageNode *imageNode = preNode->imageNode;
    DSoftwareRoundedImageNode *softwareNode = preNode->softwareNode;
    const bool useMaskNode = d->needMaskNode();
    const bool usingMaskNode = preNode->maskNode || preNode->softwareNode;

    if (useMaskNode != usingMaskNode) {
        Q_ASSERT(!preNode->maskNode);
        Q_ASSERT(!preNode->softwareNode);
    }

    if (Q_UNLIKELY(!imageNode && !softwareNode) && Q_LIKELY(tp->texture())) {
        if (useMaskNode) {
            // 创建image node
            QSGRendererInterface *ri = window()->rendererInterface();
            Q_ASSERT(ri);
            if (Q_UNLIKELY(ri->graphicsApi() == QSGRendererInterface::Software)) {
                softwareNode = new DSoftwareRoundedImageNode(this);
            } else {
                imageNode = new MaskEffectNode;
            }
        } else {
            imageNode = window()->createImageNode();
        }

        if (imageNode) {
            safeSetMaterialBlending(imageNode->material(), useMaskNode);
            safeSetMaterialBlending(imageNode->opaqueMaterial(), useMaskNode);
            imageNode->setOwnsTexture(false);
            imageNode->setTexture(tp->texture());
            preNode->setImageNode(imageNode);
        } else {
            Q_ASSERT(softwareNode);
            softwareNode->setTexture(tp->texture());
            preNode->setImageNode(softwareNode);
        }
    }

    d->updateUsePreprocess();
    if (Q_LIKELY(imageNode)) {
        imageNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
        // 目标绘制区域
        imageNode->setRect(QRectF(QPointF(0, 0), size()));
        d->updateSourceRect(imageNode);

        if (useMaskNode) {
            auto maskNode = static_cast<MaskEffectNode*>(imageNode);
            maskNode->setMaskOffset(d->getMaskOffset());
            maskNode->setSourceScale(d->getSoureSizeRatio());
            maskNode->setMaskTexture(d->textureForRadiusMask());
            maskNode->setMaskScale(d->getMaskSizeRatio());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            maskNode->setCompositionMode(d->compositionMode);
#endif
        }
    } else if (softwareNode) {
        softwareNode->setSmooth(smooth());
        softwareNode->setRect(QRectF(QPointF(0, 0), size()));
        softwareNode->setRadius(d->radius);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        softwareNode->setCompositionMode(d->compositionMode);
#endif
        d->updateSourceRect(softwareNode);
    }

    return preNode;
}

void DQuickItemViewport::componentComplete()
{
    D_D(DQuickItemViewport);

    if (d->sourceItem)
        d->initSourceItem(nullptr, d->sourceItem);

    return QQuickItem::componentComplete();
}

void DQuickItemViewport::releaseResources()
{
    D_D(DQuickItemViewport);
    if (d->texture || d->provider || d->maskTexture) {
        window()->scheduleRenderJob(new DQuickViewportCleanup(d->texture, d->maskTexture, d->provider),
                                    QQuickWindow::AfterSynchronizingStage);
        d->texture = nullptr;
        d->provider = nullptr;
        d->maskTexture.reset();
    }
}

DQUICK_END_NAMESPACE
