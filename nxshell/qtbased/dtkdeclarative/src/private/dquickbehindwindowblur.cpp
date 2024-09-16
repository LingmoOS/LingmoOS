// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickbehindwindowblur_p.h"
#include "dquickwindow.h"

#include "private/dquickbehindwindowblur_p_p.h"
#include "private/dquickwindow_p.h"

#include <DWindowManagerHelper>
#include <QSGRenderNode>
#include <QOpenGLFunctions>
#include <QQuickWindow>

DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN DSGBlendNode : public QSGRenderNode
{
public:
    explicit DSGBlendNode(bool restore = false);

    void render(const RenderState *state) override;
    StateFlags changedStates() const override;
    RenderingFlags flags() const override;
    QRectF rect() const override;

    void sync(DQuickBehindWindowBlur *blur);
protected:
    inline QQuickWindow *window() const {
        return Q_LIKELY(m_item) ? m_item->window() : nullptr;
    }

    bool m_isRestore = false;
    DQuickBehindWindowBlur *m_item = nullptr;
    QMatrix4x4 m_lastMatrix;
    QRegion m_lastClip;
    qreal m_lastRadius = -1;
    QSizeF m_lastSize;
};

DSGBlendNode::DSGBlendNode(bool restore)
    : m_isRestore(restore)
{

}

void DSGBlendNode::render(const QSGRenderNode::RenderState *state)
{
    Q_ASSERT(m_item);

    if (m_isRestore)
        return;

    if (QSGRendererInterface::Software == window()->rendererInterface()->graphicsApi()) {
        QSGRendererInterface *rif = window()->rendererInterface();
        QPainter *p = static_cast<QPainter *>(rif->getResource(window(), QSGRendererInterface::PainterResource));
        Q_ASSERT(p);

        if (p->isActive())
            p->setCompositionMode(QPainter::CompositionMode_Source);
    } else {
#ifndef QT_NO_OPENGL
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_ONE, GL_ZERO);
#endif
    }

    const auto blurData = m_item->d_func();
    if (!blurData->effectiveBlurEnabled)
        return;

    const bool dirtyMatrix = m_lastMatrix != *matrix();
    const bool dirtyClip = state->clipRegion() ? m_lastClip != *state->clipRegion() : !m_lastClip.isEmpty();
    const bool dirtyRadius = !qFuzzyCompare(m_lastRadius, blurData->radius);
    // QMatrix4x4 represents position and rotation only
    const bool dirtySzie = m_lastSize != m_item->size();

    if (Q_LIKELY(!dirtyMatrix && !dirtyClip && !dirtyRadius && !dirtySzie)) {
        return;
    }
    m_lastMatrix = *matrix();
    m_lastClip = state->clipRegion() ? *state->clipRegion() : QRegion();
    m_lastRadius = blurData->radius;
    m_lastSize = m_item->size();

    const  auto transform = matrix()->toTransform();
    const QRectF itemRect(0, 0, m_item->width(), m_item->height());
    const bool windowColorHasAlpha = window()->color().alpha() < 255;

    if (windowColorHasAlpha && transform.isRotating()) {
        QPainterPath mapOnScene;
        if (blurData->radius > 0) {
            mapOnScene.addRoundedRect(itemRect, blurData->radius, blurData->radius);
        } else {
            mapOnScene.addRect(itemRect);
        }

        mapOnScene = transform.map(mapOnScene);
        const QRegion *clipRegion = state->clipRegion();
        if (clipRegion && !clipRegion->isEmpty()) {
            QPainterPath clip;
            clip.addRegion(*clipRegion);
            mapOnScene &= clip;
        }

        blurData->blurPath = mapOnScene;
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        blurData->blurPath.clear();
#else
        blurData->blurPath = QPainterPath();
#endif

        const QRect blurArea = matrix()->mapRect(itemRect).toRect();
        blurData->blurArea.x = blurArea.x();
        blurData->blurArea.y = blurArea.y();
        blurData->blurArea.width = blurArea.width();
        blurData->blurArea.height = blurArea.height();
        blurData->blurArea.xRadius = windowColorHasAlpha ? blurData->radius : 0;
        blurData->blurArea.yRaduis = windowColorHasAlpha ? blurData->radius : 0;
    }

    // call in main thread
    m_item->metaObject()->invokeMethod(m_item, "_q_updateBlurArea");
}

QSGRenderNode::StateFlags DSGBlendNode::changedStates() const
{
    return m_isRestore ? BlendState : StateFlag();
}

QSGRenderNode::RenderingFlags DSGBlendNode::flags() const
{
    return BoundedRectRendering | DepthAwareRendering;
}

QRectF DSGBlendNode::rect() const
{
    if (Q_UNLIKELY(!m_item))
        return QRectF();
    return QRect(0, 0, m_item->width(), m_item->height());
}

void DSGBlendNode::sync(DQuickBehindWindowBlur *blur)
{
    m_item = blur;
    if (!m_isRestore)
        markDirty(QSGNode::DirtyMaterial);
}

DQuickBehindWindowBlurPrivate::~DQuickBehindWindowBlurPrivate()
{
    if (windowAttach)
        windowAttach->d_func()->removeBlur(q_func());
}

void DQuickBehindWindowBlurPrivate::_q_onHasBlurWindowChanged()
{
    if (blurEnabled)
        Q_EMIT q_func()->validChanged();

    if (!updateBlurEnable())
        _q_updateBlurArea();
}

void DQuickBehindWindowBlurPrivate::_q_updateBlurArea()
{
    if (!effectiveBlurEnabled)
        return;

    Q_Q(DQuickBehindWindowBlur);
    windowAttach->d_func()->updateBlurAreaFor(q);
}

bool DQuickBehindWindowBlurPrivate::updateBlurEnable()
{
    Q_Q(DQuickBehindWindowBlur);
    const bool enabled = q->isVisible() && q->valid() && windowAttach;
    if (effectiveBlurEnabled == enabled)
        return false;
    effectiveBlurEnabled = enabled;
    // Needs DSGBlendNode::render to map blur area
    q->update();

    if (enabled) {
        Q_ASSERT(windowAttach);
        windowAttach->d_func()->addBlur(q);
    } else if (windowAttach) {
        windowAttach->d_func()->removeBlur(q);
    }

    return true;
}

DQuickBehindWindowBlur::DQuickBehindWindowBlur(QQuickItem *parent)
    : QQuickItem(*(new DQuickBehindWindowBlurPrivate), parent)
{
    setFlag(ItemHasContents);

    connect(DWindowManagerHelper::instance(), SIGNAL(hasBlurWindowChanged()),
            this, SLOT(_q_onHasBlurWindowChanged()));
}

DQuickBehindWindowBlur::~DQuickBehindWindowBlur()
{
}

qreal DQuickBehindWindowBlur::cornerRadius() const
{
    Q_D(const DQuickBehindWindowBlur);

    return d->radius;
}

void DQuickBehindWindowBlur::setCornerRadius(qreal newRadius)
{
    Q_D(DQuickBehindWindowBlur);

    if (qFuzzyCompare(d->radius, newRadius))
        return;

    d->radius = newRadius;
    Q_EMIT cornerRadiusChanged();
    update();
}

const QColor &DQuickBehindWindowBlur::blendColor() const
{
    Q_D(const DQuickBehindWindowBlur);

    return d->blendColor;
}

void DQuickBehindWindowBlur::setBlendColor(const QColor &newBlendColor)
{
    Q_D(DQuickBehindWindowBlur);

    if (newBlendColor == d->blendColor)
        return;

    d->blendColor = newBlendColor;
    Q_EMIT blendColorChanged();
    update();
}

bool DQuickBehindWindowBlur::valid() const
{
    Q_D(const DQuickBehindWindowBlur);
    return d->blurEnabled && DWindowManagerHelper::instance()->hasBlurWindow();
}

bool DQuickBehindWindowBlur::blurEnabled() const
{
    Q_D(const DQuickBehindWindowBlur);
    return d->blurEnabled;
}

void DQuickBehindWindowBlur::setBlurEnabled(bool newBlurEnabled)
{
    Q_D(DQuickBehindWindowBlur);
    if (d->blurEnabled == newBlurEnabled)
        return;
    bool oldValid = valid();
    d->blurEnabled = newBlurEnabled;
    d->updateBlurEnable();
    Q_EMIT blurEnabledChanged();

    if (oldValid != valid())
        Q_EMIT validChanged();
}

QSGNode *DQuickBehindWindowBlur::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    Q_D(DQuickBehindWindowBlur);

    if (width() <= 0 || height() <= 0 || !window()) {
        delete oldNode;
        d->makeToInvalidBlur();
        // call in main thread
        metaObject()->invokeMethod(this, "_q_updateBlurArea");
        return nullptr;
    }

    auto blendNode = static_cast<DSGBlendNode *>(oldNode);
    if (!blendNode) {
        blendNode = new DSGBlendNode;
        auto rectangle = d->sceneGraphContext()->createInternalRectangleNode();
        blendNode->appendChildNode(rectangle);
        auto restoreNode = new DSGBlendNode(true);
        blendNode->appendChildNode(restoreNode);
    }

    blendNode->sync(this);
    static_cast<DSGBlendNode*>(blendNode->lastChild())->sync(this);
    auto rectangle = static_cast<QSGInternalRectangleNode*>(blendNode->firstChild());
    rectangle->setRect(QRectF(0, 0, width(), height()));
    rectangle->setColor(d->blendColor);
    rectangle->setPenWidth(0);
    rectangle->setRadius(d->radius);
    rectangle->setAntialiasing(antialiasing());
    rectangle->update();

    return blendNode;
}

inline DQuickWindowAttached *windowAttached(QQuickWindow *window) {
    if (!window)
        return nullptr;
    return qobject_cast<DQuickWindowAttached*>(qmlAttachedPropertiesObject<DQuickWindow>(window));
}

void DQuickBehindWindowBlur::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemSceneChange) {
        setWindowAttached(windowAttached(value.window));
    } else if (change == ItemVisibleHasChanged) {
        Q_D(DQuickBehindWindowBlur);
        d->updateBlurEnable();
    }

    QQuickItem::itemChange(change, value);
}

void DQuickBehindWindowBlur::setWindowAttached(DQuickWindowAttached *wa)
{
    Q_D(DQuickBehindWindowBlur);
    if (wa == d->windowAttach)
        return;
    if (d->windowAttach) {
        d->windowAttach->d_func()->removeBlur(this);
    }

    d->windowAttach = wa;
    d->updateBlurEnable();
}

void DQuickBehindWindowBlur::componentComplete()
{
    setWindowAttached(windowAttached(window()));
    QQuickItem::componentComplete();
}

DQUICK_END_NAMESPACE

#include "moc_dquickbehindwindowblur_p.cpp"
