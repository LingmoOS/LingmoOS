// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickbackdropblitter_p.h"
#include "dbackdropnode_p.h"

#include <DObjectPrivate>
#include <dqmlglobalobject_p.h>

#include <QSGImageNode>
#include <QSGTextureProvider>
#include <private/qsgplaintexture_p.h>
#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN BlitTextureProvider : public QSGTextureProvider {
public:
    BlitTextureProvider()
        : QSGTextureProvider()
    {

    }

    inline QSGTexture *texture() const override {
        return m_texture;
    }
    inline void setTexture(QSGTexture *tex) {
        m_texture = tex;
    }

private:
    QSGTexture *m_texture = nullptr;
};

class Content;
class Q_DECL_HIDDEN DQuickBackdropBlitterPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    DQuickBackdropBlitterPrivate(DQuickBackdropBlitter *qq)
        : DObjectPrivate(qq)
    {

    }

    ~DQuickBackdropBlitterPrivate() {
        cleanTextureProvider();
    }

    static inline DQuickBackdropBlitterPrivate *get(DQuickBackdropBlitter *qq) {
        return qq->d_func();
    }

    void init();
    void onBlitterEnabledChanged();

    inline QQmlListProperty<QObject> data() {
        if (!container)
            return QQuickItemPrivate::get(q_func())->data();

        return QQuickItemPrivate::get(container)->data();
    }

    BlitTextureProvider *ensureTextureProvider() const;
    void cleanTextureProvider();

    D_DECLARE_PUBLIC(DQuickBackdropBlitter)
    Content *content;
    QQuickItem *container = nullptr;
    mutable BlitTextureProvider *tp = nullptr;
};

class Q_DECL_HIDDEN Content : public QQuickItem
{
public:
    explicit Content(DQuickBackdropBlitter *parent)
        : QQuickItem(parent) {

    }

    inline DQuickBackdropBlitterPrivate *d() const {
        auto p = qobject_cast<DQuickBackdropBlitter*>(parent());
        Q_ASSERT(p);
        return DQuickBackdropBlitterPrivate::get(p);
    }

    bool isTextureProvider() const override {
        return true;
    }

    QSGTextureProvider *textureProvider() const override {
        if (QQuickItem::isTextureProvider())
            return QQuickItem::textureProvider();

        return d()->ensureTextureProvider();
    }

    inline bool offscreen() const {
        return !flags().testFlag(ItemHasContents);
    }

    inline bool setOffscreen(bool newOffscreen) {
        if (offscreen() == newOffscreen)
            return false;

        if (d()->tp) {
            if (newOffscreen)
                disconnect(d()->tp, &BlitTextureProvider::textureChanged, this, &Content::update);
            else
                connect(d()->tp, &BlitTextureProvider::textureChanged, this, &Content::update);
        }

        setFlag(ItemHasContents, !newOffscreen);
        return true;
    }

private:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override {
        const auto tp = d()->ensureTextureProvider();
        if (Q_LIKELY(!tp->texture())) {
            delete old;
            return nullptr;
        }

        auto node = static_cast<QSGImageNode*>(old);
        if (Q_UNLIKELY(!node)) {
            node = window()->createImageNode();
            node->setOwnsTexture(false);
        }

        auto texture = tp->texture();
        node->setTexture(texture);

        const QRectF sourceRect(QPointF(0, 0), texture->textureSize());

        node->setSourceRect(sourceRect);
        node->setRect(QRectF(QPointF(0, 0), size()));
        node->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
        node->setAnisotropyLevel(antialiasing() ? QSGTexture::Anisotropy4x : QSGTexture::AnisotropyNone);

        return node;
    }
};

void DQuickBackdropBlitterPrivate::init()
{
    D_Q(DQuickBackdropBlitter);
    content = new Content(q);

    if (q->window()->graphicsApi() != QSGRendererInterface::Software) {
        container = new QQuickItem(q);
        onBlitterEnabledChanged();
    }
}

void DQuickBackdropBlitterPrivate::onBlitterEnabledChanged()
{
    D_Q(DQuickBackdropBlitter);

    if (!container)
        return;

    auto d = QQuickItemPrivate::get(container);

    if (q->blitterEnabled()) {
        d->refFromEffectItem(true);
    } else {
        d->derefFromEffectItem(true);
    }
}

BlitTextureProvider *DQuickBackdropBlitterPrivate::ensureTextureProvider() const
{
    if (Q_LIKELY(tp))
        return tp;

    tp = new BlitTextureProvider();

    if (!content->offscreen())
        tp->connect(tp, &BlitTextureProvider::textureChanged, content, &Content::update);

    return tp;
}

void DQuickBackdropBlitterPrivate::cleanTextureProvider()
{
    if (tp) {
        QQuickWindowQObjectCleanupJob::schedule(q_func()->window(), tp);
        tp = nullptr;
    }
}

DQuickBackdropBlitter::DQuickBackdropBlitter(QQuickItem *parent)
    : QQuickItem(parent)
    , DObject(*new DQuickBackdropBlitterPrivate(this))
{
    setFlag(ItemHasContents);
    D_D(DQuickBackdropBlitter);
    d->init();
}

DQuickBackdropBlitter::~DQuickBackdropBlitter()
{
    DQuickBackdropBlitter::releaseResources();
}

QQuickItem *DQuickBackdropBlitter::content() const
{
    D_DC(DQuickBackdropBlitter);
    return d->content;
}

bool DQuickBackdropBlitter::offscreen() const
{
    D_DC(DQuickBackdropBlitter);
    return d->content->offscreen();
}

void DQuickBackdropBlitter::setOffscreen(bool newOffscreen)
{
    D_D(DQuickBackdropBlitter);
    if (d->content->setOffscreen(newOffscreen))
        Q_EMIT offscreenChanged();
}

bool DQuickBackdropBlitter::blitterEnabled() const
{
    return flags().testFlag(ItemHasContents);
}

void DQuickBackdropBlitter::setBlitterEnabled(bool newBlitterEnabled)
{
    if (blitterEnabled() == newBlitterEnabled)
        return;
    setFlag(ItemHasContents, newBlitterEnabled);

    D_D(DQuickBackdropBlitter);
    d->onBlitterEnabledChanged();

    Q_EMIT blitterEnabledChanged();
}

void DQuickBackdropBlitter::invalidateSceneGraph()
{
    D_D(DQuickBackdropBlitter);
    delete d->tp;
    d->tp = nullptr;
}

static void onTextureChanged(DBackdropNode *node, void *data) {
    auto *d = reinterpret_cast<DQuickBackdropBlitterPrivate*>(data);
    if (!d->tp)
        return;

    d->tp->setTexture(node->texture());
    Q_EMIT d->tp->textureChanged();
}

QSGNode *DQuickBackdropBlitter::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *oldData)
{
    Q_UNUSED(oldData)

    auto node = static_cast<DBackdropNode*>(oldNode);
    if (Q_LIKELY(node)) {
        node->resize(size());
        return node;
    }

    D_D(DQuickBackdropBlitter);
    if (DQMLGlobalObject::isSoftwareRender()) {
        node = DBackdropNode::createSoftwareNode(this);
    } else {
        node = DBackdropNode::createRhiNode(this);
    }

    node->setContentItem(d->container);
    node->setTextureChangedCallback(onTextureChanged, d);
    node->resize(size());
    onTextureChanged(node, d);

    return node;
}

void DQuickBackdropBlitter::itemChange(ItemChange type, const ItemChangeData &data)
{
    if (type == ItemDevicePixelRatioHasChanged) {
        update();
    }

    QQuickItem::itemChange(type, data);
}

void DQuickBackdropBlitter::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    D_D(DQuickBackdropBlitter);
    d->content->setSize(newGeometry.size());

    if (d->container)
        d->container->setSize(newGeometry.size());
}

void DQuickBackdropBlitter::releaseResources()
{
    D_D(DQuickBackdropBlitter);
    d->cleanTextureProvider();
}

DQUICK_END_NAMESPACE

#include "moc_dquickbackdropblitter_p.cpp"
