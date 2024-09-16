// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#define protected public
#define private public
#include <rhi/qrhi.h>
#include <private/qsgrenderer_p.h>
#undef protected
#undef private

#include "dbackdropnode_p.h"
#include "dqmlglobalobject_p.h"

#include <QQuickItem>
#include <QRunnable>
#include <QSGImageNode>
#include <private/qquickitem_p.h>
#include <private/qsgplaintexture_p.h>
#include <private/qrhi_p.h>
#include <private/qrhivulkan_p.h>
#include <private/qsgrenderer_p.h>
#include <private/qsgdefaultrendercontext_p.h>
#include <private/qsgrhisupport_p.h>
#include <private/qquickrendercontrol_p.h>

#ifndef QT_NO_OPENGL
#include <qopengl.h>
#include <QOpenGLExtraFunctions>
#include <private/qrhigles2_p.h>
#include <private/qopenglcontext_p.h>
#endif

#include <algorithm>

DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN DataManagerBase : public QObject
{
public:
    mutable QAtomicInt ref;

    explicit DataManagerBase(QQuickWindow *owner)
        : QObject() {
        moveToThread(owner->thread());
        setParent(owner);
    }
};

template <class T>
class Q_DECL_HIDDEN DataManagerPointer
{
    static_assert(std::is_base_of<DataManagerBase, T>::value);
public:
    DataManagerPointer() noexcept = default;
    constexpr DataManagerPointer(std::nullptr_t) noexcept : DataManagerPointer{} {}
    inline DataManagerPointer(T *p) : pointer(p) {
        if (pointer)
            pointer->ref.ref();
    }

    DataManagerPointer(DataManagerPointer<T> &&other) noexcept
        : pointer(std::exchange(other.pointer, nullptr)) {}

    DataManagerPointer(const DataManagerPointer<T> &other) noexcept
        : pointer(other.pointer) {
        ref();
    }

    DataManagerPointer &operator=(const DataManagerPointer<T> &other) noexcept
    {
        deref();
        pointer = other.pointer;
        ref();
        return *this;
    }

    DataManagerPointer &operator=(DataManagerPointer<T> &&other) noexcept
    {
        pointer = std::exchange(other.pointer, nullptr);
        return *this;
    }

    ~DataManagerPointer() {
        deref();
    }

    inline DataManagerPointer<T> &operator=(T* p) {
        deref();
        pointer = p;
        ref();
        return *this;
    }

    T* data() const noexcept
    { return pointer; }
    T* get() const noexcept
    { return data(); }
    T* operator->() const noexcept
    { return data(); }
    T& operator*() const noexcept
    { return *data(); }
    operator T*() const noexcept
    { return data(); }

    bool isNull() const noexcept
    { return pointer.isNull(); }

    bool operator==(T *other) const noexcept
    { return pointer == other; }
    bool operator!=(T *other) const noexcept
    { return pointer != other; }

private:
    void deref() {
        if (!pointer)
            return;
        pointer->ref.deref();
        if (pointer->ref == 0) {
            pointer->DataManagerBase::deleteLater();
            pointer.clear();
        }
    }

    void ref() {
        if (pointer)
            pointer->ref.ref();
    }

    QPointer<T> pointer;
};

template <class Derive, class DataType, typename... DataKeys>
class Q_DECL_HIDDEN DataManager : public DataManagerBase
{
public:
    struct Data {
        int released = 0;
        DataType *data = nullptr;
    };

    static DataManagerPointer<Derive> get(QQuickWindow *owner) {
        return owner->findChild<Derive*>({}, Qt::FindDirectChildrenOnly);
    }

    static DataManagerPointer<Derive> resolve(const DataManagerPointer<Derive> &other, QQuickWindow *owner) {
        static_assert(&Derive::metaObject);
        Q_ASSERT(owner);
        if (other && other->owner() == owner)
            return other;
        {
            Derive *other = get(owner);
            if (!other)
                other = new Derive(owner);
            return other;
        }
    }

    inline QQuickWindow *owner() const {
        return static_cast<QQuickWindow*>(parent());
    }

    Data *resolve(Data *data, DataKeys&&... keys) {
        if (data && get()->check(data->data, std::forward<DataKeys>(keys)...)
            && dataList.contains(data)) {
            return data;
        }
        if (data)
            release(data);

        for (auto data : dataList) {
            if (get()->check(data->data, std::forward<DataKeys>(keys)...)) {
                data->released = 0;
                return data;
            }
        }

        data = new Data();
        if ((data->data = get()->create(std::forward<DataKeys>(keys)...))) {
            dataList.append(data);
            return data;
        }

        delete data;
        return nullptr;
    }

    inline void release(Data *data) {
        data->released++;
        ensureCleanJob();
    }

protected:
    struct CleanJob : public QRunnable {
        CleanJob(DataManager *manager)
            : manager(manager) {}

        void run() override {
            if (!manager)
                return;

            manager->cleanJob = nullptr;

            QList<Data*> tmp;
            std::swap(manager->dataList, tmp);
            manager->dataList.reserve(tmp.size());

            for (Data *data : tmp) {
                if (data->released > 2) {
                    manager->get()->destroy(data->data);
                    delete data;
                } else {
                    manager->dataList << data;

                    if (data->released > 0) {
                        data->released++;
                        manager->ensureCleanJob();
                    }
                }
            }
        }

        QPointer<DataManager> manager;
    };

    inline void ensureCleanJob() {
        if (!cleanJob) {
            cleanJob = new CleanJob(this);
            owner()->scheduleRenderJob(cleanJob, QQuickWindow::AfterRenderingStage);
        }
    }

    inline const Derive *get() const {
        return static_cast<const Derive*>(this);
    }

    inline Derive *get() {
        return static_cast<Derive*>(this);
    }

    DataManager(QQuickWindow *owner)
        : DataManagerBase(owner) {
        Q_ASSERT(owner->findChildren<Derive*>(Qt::FindDirectChildrenOnly).size() == 0);
    }

    using QObject::deleteLater;
    ~DataManager() {
        for (auto data : dataList) {
            Derive::destroy(data->data);
            delete data;
        }
    }

    QList<Data*> dataList;
    QRunnable *cleanJob = nullptr;
};

class Q_DECL_HIDDEN RhiTextureManager : public DataManager<RhiTextureManager, QRhiTexture, QRhiTexture::Format, const QSize&>
{
    Q_OBJECT

    friend class DataManager;

    RhiTextureManager(QQuickWindow *owner)
        : DataManager<RhiTextureManager, QRhiTexture, QRhiTexture::Format, const QSize&>(owner) {
        Q_ASSERT(owner->findChildren<RhiTextureManager*>(Qt::FindDirectChildrenOnly).size() == 1);
    }

    static bool check(QRhiTexture *texture, QRhiTexture::Format format, const QSize &size) {
        return texture->format() == format && texture->pixelSize() == size;
    }

    QRhiTexture *create(QRhiTexture::Format format, const QSize &size) {
        auto texture = owner()->rhi()->newTexture(format, size, 1, QRhiTexture::RenderTarget);
        if  (!texture->create()) {
            delete texture;
            return nullptr;
        }

        return texture;
    }

    static void destroy(QRhiTexture *texture) {
        texture->deleteLater();
    }
};

class Q_DECL_HIDDEN RhiManager : public DataManager<RhiManager, void>
{
    Q_OBJECT
public:
    QRhi *rhi() const {
        return m_rhi->rhi;
    }

    QQuickGraphicsConfiguration graphicsConfiguration() const {
        return m_rhi->gc;
    }

    void sync(const QSize &pixelSize, QSGRootNode *rootNode,
              const QMatrix4x4 &matrix = {}, QSGRenderer *base = nullptr,
              const QVector2D &dpr = {}, bool flipY = false) {
        Q_ASSERT(!renderer->rootNode());

        if (base) {
            renderer->setDevicePixelRatio(base->devicePixelRatio());
            renderer->setDeviceRect(base->deviceRect());
            renderer->setViewportRect(base->viewportRect());
            renderer->setProjectionMatrix(base->projectionMatrix());
            renderer->setProjectionMatrixWithNativeNDC(base->projectionMatrixWithNativeNDC());
        } else {
            renderer->setDevicePixelRatio(1.0);
            renderer->setDeviceRect(QRect(QPoint(0, 0), pixelSize));
            renderer->setViewportRect(pixelSize);

            Q_ASSERT(dpr.x() > 0 && dpr.y() > 0);
            QRectF rect(QPointF(0, 0), QSizeF(pixelSize.width() / dpr.x(),
                                              pixelSize.height() / dpr.y()));
            renderer->setProjectionMatrixToRect(rect, rhi()->isYUpInNDC() && !flipY
                                                          ? QSGRenderer::MatrixTransformFlipY
                                                          : QSGRenderer::MatrixTransformFlag {}, false);
        }

        if (Q_UNLIKELY(!matrix.isIdentity())) {
            renderer->setProjectionMatrix(renderer->projectionMatrix() * matrix);
            renderer->setProjectionMatrixWithNativeNDC(renderer->projectionMatrixWithNativeNDC() * matrix);
        }

        renderer->setRootNode(rootNode);
    }

    bool preprocess(QRhiRenderTarget *rt, qreal &oldDPR, QRhiCommandBuffer* &oldCB, QSurface *forceSurface = nullptr) {
        QRhiCommandBuffer *cb = nullptr;

#ifndef QT_NO_OPENGL
        QSurface *fallbackSurface = nullptr;
        QRhiGles2 *gles2Rhi = nullptr;
        if (forceSurface) {
            Q_ASSERT(rhi()->backend() == QRhi::OpenGLES2);
            gles2Rhi = static_cast<QRhiGles2*>(rhi()->d);
            fallbackSurface = gles2Rhi->fallbackSurface;
            gles2Rhi->fallbackSurface = forceSurface;
        }
#endif

        if (rhi()->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
            return false;
        Q_ASSERT(cb);

#ifndef QT_NO_OPENGL
        if (forceSurface) {
            Q_ASSERT(gles2Rhi);
            gles2Rhi->fallbackSurface = fallbackSurface;
        }
#endif

        renderer->setRenderTarget({ rt, rt->renderPassDescriptor(), cb });
        auto dc = static_cast<QSGDefaultRenderContext*>(context);
        oldDPR = dc->currentDevicePixelRatio();
        oldCB = dc->currentFrameCommandBuffer();
        context->prepareSync(renderer->devicePixelRatio(), cb, graphicsConfiguration());

        renderer->m_is_rendering = true;
        renderer->preprocess();

        return true;
    }

    bool render(qreal oldDPR, QRhiCommandBuffer* &oldCB) {
        Q_ASSERT(renderer->m_is_rendering);
        renderer->render();
        renderer->m_is_rendering = false;
        renderer->m_changed_emitted = false;

        context->prepareSync(oldDPR, oldCB, graphicsConfiguration());

        bool ok = rhi()->endOffscreenFrame() == QRhi::FrameOpSuccess;
        renderer->setRootNode(nullptr);

        return ok;
    }

    inline bool render(QRhiRenderTarget *rt, QSurface *forceSurface = nullptr) {
        qreal oldDPR;
        QRhiCommandBuffer *oldCB;

        if (!preprocess(rt, oldDPR, oldCB, forceSurface))
            return false;

        return render(oldDPR, oldCB);
    }

private:
    friend class DataManager;

    RhiManager(QQuickWindow *owner)
        : DataManager<RhiManager, void>(owner) {
        Q_ASSERT(owner->findChildren<RhiManager*>(Qt::FindDirectChildrenOnly).size() == 1);
        auto oldGraphicsDevice = owner->graphicsDevice();
        if (owner->rhi()->backend() == QRhi::OpenGLES2) {
            auto gln = static_cast<const QRhiGles2NativeHandles*>(owner->rhi()->nativeHandles());
            owner->setGraphicsDevice(QQuickGraphicsDevice::fromOpenGLContext(gln->context));
        }
        auto rhi = QSGRhiSupport::instance()->createRhi(owner, owner);
        owner->setGraphicsDevice(oldGraphicsDevice);
        if (!rhi.rhi)
            return;
        Q_ASSERT(rhi.rhi->backend() == owner->rhi()->backend());
        m_rhi.reset(new Rhi());
        m_rhi->rhi = rhi.rhi;
        m_rhi->own = rhi.own;
        m_rhi->gc = owner->graphicsConfiguration();

        context = QQuickWindowPrivate::get(owner)->context;
        renderer = context->createRenderer(QSGRendererInterface::RenderMode2DNoDepthBuffer);
    }

    ~RhiManager() {
        delete renderer;
    }

    static bool check() {
        Q_UNREACHABLE();
        return true;
    }

    static void *create() {
        Q_UNREACHABLE();
        return nullptr;
    }

    static void destroy(void*) {
        Q_UNREACHABLE();
    }

    struct Rhi {
        QRhi *rhi;
        bool own;
        QQuickGraphicsConfiguration gc;

        static inline void cleanup(Rhi *pointer) {
            if (!pointer || !pointer->own)
                return;

            auto rhiSupport = QSGRhiSupport::instance();
            if (rhiSupport)
                rhiSupport->destroyRhi(pointer->rhi, pointer->gc);
            else
                delete pointer->rhi;
        }
    };

    QSGRenderContext *context;
    QSGRenderer *renderer;

    QScopedPointer<Rhi> m_rhi;
};

#ifndef QT_NO_OPENGL
class Q_DECL_HIDDEN GLFramebufferManager : public DataManager<GLFramebufferManager, void>
{
    Q_OBJECT

public:
    QRhiTexture::Format format() const {
        // TODO: get format from window
        return QRhiTexture::RGBA8;
    }

    static void blit(const QSize sourcePixelSize, QRhiTexture *dst, const QRect &sr, const QRect &dr) {
        auto f = QOpenGLContext::currentContext()->extraFunctions();
        Q_ASSERT(f);

        GLuint oldFBO;
        f->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldFBO);

        GLuint tmpFBO;
        f->glGenFramebuffers(1, &tmpFBO);
        f->glBindFramebuffer(GL_FRAMEBUFFER, tmpFBO);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, dst->nativeTexture().object, 0);

        f->glBindFramebuffer(GL_READ_FRAMEBUFFER, oldFBO);

        f->glBlitFramebuffer(sr.x(), sourcePixelSize.height() - sr.y(), sr.right() + 1,
                             sourcePixelSize.height() - sr.bottom() - 1,
                             dr.x(), dr.y(), dr.right() + 1, dr.bottom() + 1,
                             GL_COLOR_BUFFER_BIT,
                             GL_NEAREST);

        f->glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
        f->glDeleteFramebuffers(1, &tmpFBO);
    }

    void updateTexture(const QRect &region) {
        Q_ASSERT(m_texture);
        auto f = QOpenGLContext::currentContext()->extraFunctions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, QOpenGLContext::currentContext()->defaultFramebufferObject());
        blit(m_texture->pixelSize(), m_texture, region, region);
    }

    QRhiTexture *ensureTexture(const QSize &size) {
        if (m_texture)
            m_texture->deleteLater();

        m_texture = owner()->rhi()->newTexture(format(), size, 1, QRhiTexture::RenderTarget);
        if (!m_texture->create()) {
            delete m_texture;
            m_texture = nullptr;
        }

        return m_texture;
    }

    QGles2TextureRenderTarget *ensureProxy(QRhi *rhi, const QSize &pixelSize) {
        const GLuint fbo = QOpenGLContext::currentContext()->defaultFramebufferObject();

        if (!m_rtProxy) {
            QRhiTextureRenderTargetDescription rtDesc;
            const auto flags = QRhiTextureRenderTarget::PreserveColorContents;
            m_rtProxy = static_cast<QGles2TextureRenderTarget*>(rhi->newTextureRenderTarget(rtDesc, flags));
            m_rtProxy->d.pixelSize = pixelSize;
            m_rtProxy->framebuffer = fbo;
        }

        Q_ASSERT(m_rtProxy->framebuffer == fbo);
        return m_rtProxy;
    }

    void destroy() {
        if (m_rtProxy) {
            m_rtProxy->deleteLater();
            m_rtProxy = nullptr;
        }
        if (m_texture) {
            m_texture->deleteLater();
            m_texture = nullptr;
        }
    }

private:
    friend class DataManager;

    GLFramebufferManager(QQuickWindow *owner)
        : DataManager<GLFramebufferManager, void>(owner) {
        Q_ASSERT(owner->findChildren<GLFramebufferManager*>(Qt::FindDirectChildrenOnly).size() == 1);

    }

    ~GLFramebufferManager() {
        Q_ASSERT(!m_texture);
        Q_ASSERT(!m_rtProxy);
    }

    static bool check() {
        Q_UNREACHABLE();
        return true;
    }

    static void *create() {
        Q_UNREACHABLE();
        return nullptr;
    }

    static void destroy(void*) {
        Q_UNREACHABLE();
    }

    QRhiTexture *m_texture = nullptr;
    QGles2TextureRenderTarget *m_rtProxy = nullptr;
};
#endif

static QSizeF mapSize(const QRectF &source, const QMatrix4x4 &matrix)
{
    auto topLeft = matrix.map(source.topLeft());
    auto bottomLeft = matrix.map(source.bottomLeft());
    auto topRight = matrix.map(source.topRight());
    auto bottomRight = matrix.map(source.bottomRight());

    const qreal width1 = std::sqrt(std::pow(topRight.x() - topLeft.x(), 2)
                                   + std::pow(topRight.y() - topLeft.y(), 2));
    const qreal width2 = std::sqrt(std::pow(bottomRight.x() - bottomLeft.x(), 2)
                                   + std::pow(bottomRight.y() - bottomLeft.y(), 2));
    const qreal height1 = std::sqrt(std::pow(bottomLeft.x() - topLeft.x(), 2)
                                    + std::pow(bottomLeft.y() - topLeft.y(), 2));
    const qreal height2 = std::sqrt(std::pow(bottomRight.x() - topRight.x(), 2)
                                    + std::pow(bottomRight.y() - topRight.y(), 2));

    QSizeF size;
    size.setWidth(std::max(width1, width2));
    size.setHeight(std::max(height1, height2));

    return size;
}

inline static void restoreChildNodesTo(QSGNode *node, QSGNode *realParent) {
    Q_ASSERT(realParent->firstChild() == node->firstChild());
    Q_ASSERT(realParent->lastChild() == node->lastChild());

    DQMLGlobalObject::QSGNode_subtreeRenderableCount(node) = 0;
    DQMLGlobalObject::QSGNode_firstChild(node) = nullptr;
    DQMLGlobalObject::QSGNode_lastChild(node) = nullptr;

    node = realParent->firstChild();
    do {
        DQMLGlobalObject::QSGNode_parent(node) = realParent;
        node = node->nextSibling();
    } while (node);
}

inline static void overrideChildNodesTo(QSGNode *node, QSGNode *newParent) {
    Q_ASSERT(newParent->childCount() == 0);
    Q_ASSERT(node->firstChild());
    DQMLGlobalObject::QSGNode_subtreeRenderableCount(newParent) = 0;
    DQMLGlobalObject::QSGNode_firstChild(newParent) = node->firstChild();
    DQMLGlobalObject::QSGNode_lastChild(newParent) = node->lastChild();

    node = newParent->firstChild();
    do {
        DQMLGlobalObject::QSGNode_parent(node) = newParent;
        node->markDirty(QSGNode::DirtyNodeAdded);
        node = node->nextSibling();
    } while (node);
}

class Q_DECL_HIDDEN RhiNode : public DBackdropNode {
public:
    RhiNode(QQuickItem *item)
        : DBackdropNode(item, new Texture)
    {

    }

    ~RhiNode() {
        destroy();
    }

    StateFlags changedStates() const override {
        if (Q_UNLIKELY(renderData && !contentNode))
            return (RenderTargetState | ViewportState);

        return {0};
    }

    RenderingFlags flags() const override {
        if (Q_UNLIKELY(!contentNode))
            return BoundedRectRendering | DepthAwareRendering;

        return DepthAwareRendering;
    }

    void releaseResources() override {
        destroy();
    }

    void render(const RenderState *state) override {
        Q_UNUSED(state)

        // begin pre init for render
        contentNode = nullptr;

        if (Q_UNLIKELY(!m_item || !m_item->window())) {
            reset();
            return;
        }

        auto window = renderWindow();
        // TODO: Apple viewport to matrix, needs get QSGRenderer
        const auto renderMatrix = *this->matrix();
        const bool hasRotation = renderMatrix.flags().testAnyFlags(QMatrix4x4::Rotation2D | QMatrix4x4::Rotation);

        QRhiTexture *ct = nullptr;
        auto rt = renderTarget();
        const bool isTextureRenderTarget = rt->resourceType() == QRhiResource::TextureRenderTarget;

        if (isTextureRenderTarget) {
            auto rhiRT = static_cast<QRhiTextureRenderTarget*>(rt);
            auto rtDesc = rhiRT->description();
            if (rtDesc.colorAttachmentCount() >= 1)
                ct = rtDesc.colorAttachmentAt(0)->texture();

            if (!ct) {
                reset();
                return;
            }
            Q_ASSERT(ct->rhi() == window->rhi());
        }
#ifndef QT_NO_OPENGL
        else if (rt->resourceType() == QRhiResource::SwapChainRenderTarget
                 && rt->rhi()->backend() == QRhi::OpenGLES2) {
            glfbManager = glfbManager->resolve(glfbManager, window);
            if (hasRotation)
                ct = glfbManager->ensureTexture(rt->pixelSize());
        } else {
            reset();
            return;
        }
#endif
        Q_ASSERT(ct || glfbManager);

        const float devicePixelRatio = effectiveDevicePixelRatio();
        const auto oldManager = manager;
        manager = RhiTextureManager::resolve(manager, window);

        if (oldManager != manager) {
            sgTexture()->setTexture(nullptr);
            if (oldManager && texture)
                oldManager->release(texture);
        }

        rhi = rhi->resolve(rhi, window);
        Q_ASSERT(rhi);

        QSize itemPixelSize;

        if (hasRotation) {
            const QSizeF size = mapSize(m_rect, renderMatrix) * devicePixelRatio;
            if (size.isEmpty()) {
                reset();
                return;
            }

            itemPixelSize = size.toSize();

            if (!renderData) {
                renderData.reset(new RenderData);

                renderData->imageNode = window->createImageNode();
                renderData->imageNode->setFlag(QSGNode::OwnedByParent);
                renderData->imageNode->setOwnsTexture(false);
                renderData->texture.setOwnsTexture(false);
                renderData->imageNode->setTexture(&renderData->texture);
                renderData->rootNode.appendChildNode(renderData->imageNode);
            }
        } else {
            renderData.reset();

            QSizeF size = renderMatrix.mapRect(m_rect).size() * devicePixelRatio;
            if (!size.isValid()) {
                reset();
                return;
            }

            itemPixelSize = size.toSize();
        }

        texture = manager->resolve(texture, ct ? ct->format() : glfbManager->format(), itemPixelSize);
        if (Q_UNLIKELY(!texture)) {
            reset();
            return;
        }
        Q_ASSERT(texture->data);

        if (renderData) {
            if (!renderData->rt || sgTexture()->rhiTexture() != texture->data) {
                QRhiTextureRenderTargetDescription rtDesc(texture->data);
                const auto flags = QRhiTextureRenderTarget::PreserveColorContents;
                auto newRT = rhi->rhi()->newTextureRenderTarget(rtDesc, flags);
                newRT->setRenderPassDescriptor(newRT->newCompatibleRenderPassDescriptor());
                if (!newRT->create()) {
                    delete newRT;
                    return;
                }

                renderData->rt.reset(newRT);
            }
        }

        if (m_content) {
            auto rootNode = DQMLGlobalObject::getRootNode(m_content);
            if (rootNode && rootNode->firstChild()) {
                contentNode = rootNode;
            }
        }
        // end pre init for render

        if (Q_UNLIKELY(!texture))
            return;

        if (renderData) {
#ifndef QT_NO_OPENGL
            if (glfbManager) {
                QRectF region = renderMatrix.mapRect(m_rect);
                region.moveTopLeft(region.topLeft() * devicePixelRatio);
                region.setSize(region.size() * devicePixelRatio);
                glfbManager->updateTexture(region.toRect());
            }
#endif

            renderData->texture.setTexture(ct);
            renderData->texture.setTextureSize(ct->pixelSize());

            const QPointF sourcePos = renderMatrix.map(m_rect.topLeft());
            renderData->imageNode->setRect(QRectF(-(devicePixelRatio - 1) * sourcePos, ct->pixelSize()));

            rhi->sync(texture->data->pixelSize(), &renderData->rootNode, renderMatrix.inverted(), nullptr,
                      {texture->data->pixelSize().width() / float(m_rect.width() * devicePixelRatio),
                       texture->data->pixelSize().height() / float(m_rect.height() * devicePixelRatio)});
            rhi->render(renderData->rt.get());
        } else {
            QPointF sourcePos = renderMatrix.map(m_rect.topLeft()) * devicePixelRatio;

            if (ct) {
                auto rhi = this->rhi->rhi();
                auto rub = rhi->nextResourceUpdateBatch();
                QRhiTextureCopyDescription desc;
                desc.setPixelSize(texture->data->pixelSize());
                desc.setSourceTopLeft(sourcePos.toPoint());
                rub->copyTexture(texture->data, ct, desc);

                QRhiCommandBuffer *cb = nullptr;
                if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
                    return;
                Q_ASSERT(cb);
                // TODO: needs vkCmdPipelineBarrier?
                cb->resourceUpdate(rub);
                rhi->endOffscreenFrame();
            } else {
#ifndef QT_NO_OPENGL
                GLFramebufferManager::blit(renderTarget()->pixelSize(), texture->data,
                                           QRect(sourcePos.toPoint(), texture->data->pixelSize()),
                                           QRect(QPoint(0, 0), texture->data->pixelSize()));
#else
                Q_UNREACHABLE();
#endif
            }
        }

        if (sgTexture()->rhiTexture() != texture->data)
            sgTexture()->setTexture(texture->data);
        doNotifyTextureChanged();

        if (contentNode) {
            QRhiTextureRenderTarget *textureRT = nullptr;
            if (isTextureRenderTarget) {
                textureRT = static_cast<QRhiTextureRenderTarget*>(renderTarget());
            }
#ifndef QT_NO_OPENGL
            else if (renderTarget()->rhi()->backend() == QRhi::OpenGLES2) {
                auto rt = glfbManager->ensureProxy(renderTarget()->rhi(),
                                                   renderTarget()->pixelSize());
                textureRT = rt;
                Q_ASSERT(textureRT);
                textureRT->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
            }
#endif
            else {
                Q_UNREACHABLE();
            }

            auto saveFlags = textureRT->flags();
            textureRT->setFlags(QRhiTextureRenderTarget::PreserveColorContents);

            if (!node)
                node.reset(new Node);

            node->opacityNode.setOpacity(inheritedOpacity());
            node->transformNode.setMatrix(*this->matrix());

            QSGNode *childContainer = &node->opacityNode;
            if (clipList()) {
                if (!node->clipNode) {
                    node->clipNode = new QQuickDefaultClipNode(QRectF(0, 0, 65535, 65535));
                    node->clipNode->setFlag(QSGNode::OwnedByParent, false);
                    node->clipNode->setClipRect(node->clipNode->rect());
                    node->clipNode->update();

                    node->rootNode.reparentChildNodesTo(node->clipNode);
                    node->rootNode.appendChildNode(node->clipNode);
                }
            } else {
                if (node->clipNode)
                    node->clipNode->reparentChildNodesTo(&node->rootNode);

                delete node->clipNode;
                node->clipNode = nullptr;
            }

            overrideChildNodesTo(contentNode, childContainer);

            rhi->sync(textureRT->pixelSize(), &node->rootNode, {}, nullptr,
                      {devicePixelRatio, devicePixelRatio}, !isTextureRenderTarget);
            qreal oldDPR;
            QRhiCommandBuffer *oldCB;
            if (rhi->preprocess(textureRT, oldDPR, oldCB, renderWindow())) {
                if (node->clipNode) {
                    if (!node->clipNode->clipList()) {
                        node->clipNode->setRendererClipList(clipList());
                    } else {
                        auto lastClipNode = node->clipNode->clipList();
                        while (auto cliplist = lastClipNode->clipList())
                            lastClipNode = cliplist;
                        Q_ASSERT(lastClipNode->clipList());
                        const_cast<QSGClipNode*>(lastClipNode)->setRendererClipList(clipList());
                    }
                }

                rhi->render(oldDPR, oldCB);
            }

            restoreChildNodesTo(childContainer, contentNode);

            textureRT->setFlags(saveFlags);
        }
    }

private:
    void reset(bool notifyTexture = true) {
        if (renderData)
            renderData->rt.reset();

        if (!sgTexture()->rhiTexture() && notifyTexture)
            doNotifyTextureChanged();
        sgTexture()->setTexture(nullptr);
        if (texture && manager)
            manager->release(texture);
        texture = nullptr;

#ifndef QT_NO_OPENGL
        if (glfbManager)
            glfbManager->destroy();
#endif
    }

    void destroy() {
        reset(false);
        renderData.reset();
        node.reset();
        manager = nullptr;
        texture = nullptr;
#ifndef QT_NO_OPENGL
        glfbManager = nullptr;
#endif
    }

    DataManagerPointer<RhiTextureManager> manager;
    RhiTextureManager::Data *texture = nullptr;
    DataManagerPointer<RhiManager> rhi;

#ifndef QT_NO_OPENGL
    DataManagerPointer<GLFramebufferManager> glfbManager;
#endif

    struct Node {
        Node() {
            transformNode.setFlag(QSGNode::OwnedByParent, false);
            opacityNode.setFlag(QSGNode::OwnedByParent, false);
            rootNode.setFlag(QSGNode::OwnedByParent, false);
            transformNode.appendChildNode(&opacityNode);
            rootNode.appendChildNode(&transformNode);
        }

        QSGRootNode rootNode;
        QSGTransformNode transformNode;
        QSGOpacityNode opacityNode;
        QQuickDefaultClipNode *clipNode = nullptr;
    };

    std::unique_ptr<Node> node;
    QSGRootNode *contentNode = nullptr;

    struct RenderData {
        struct QRhiTextureRenderTargetDeleter {
            inline void operator()(QRhiTextureRenderTarget *pointer) const {
                if (pointer) {
                    delete pointer->renderPassDescriptor();
                    pointer->setRenderPassDescriptor(nullptr);
                    pointer->deleteLater();
                }
            }
        };

        std::unique_ptr<QRhiTextureRenderTarget, QRhiTextureRenderTargetDeleter> rt;
        QSGRootNode rootNode;
        QSGImageNode *imageNode;
        QSGPlainTexture texture;
    };

    std::unique_ptr<RenderData> renderData;

    struct Texture : public QSGDynamicTexture {
        void setTexture(QRhiTexture *texture) {
            if (texture)
                m_textureSize = texture->pixelSize();
            m_texture = texture;
        }

        bool updateTexture() override {
            return true;
        }

        qint64 comparisonKey() const override {
            if (m_texture)
                return qint64(m_texture);

            return qint64(this);
        }

        QRhiTexture *rhiTexture() const override {
            return m_texture;
        }

        QSize textureSize() const override {
            return m_textureSize;
        }

        bool hasAlphaChannel() const override {
            return true;
        }

        bool hasMipmaps() const override {
            return mipmapFiltering() != QSGTexture::None;
        }

        QRhiTexture *m_texture = nullptr;
        QSize m_textureSize;
    };

    inline Texture *sgTexture() const {
        return static_cast<Texture*>(m_texture.get());
    }
};

QSGTexture *DBackdropNode::texture() const
{
    return m_texture.data();
}

DBackdropNode *DBackdropNode::createRhiNode(QQuickItem *item)
{
    auto node = new RhiNode(item);
    return node;
}

class Q_DECL_HIDDEN QImageManager : public DataManager<QImageManager, QImage, QImage::Format, const QSize&>
{
    Q_OBJECT

    friend class DataManager;

    QImageManager(QQuickWindow *owner)
        : DataManager<QImageManager, QImage, QImage::Format, const QSize&>(owner) {
        Q_ASSERT(owner->findChildren<QImageManager*>(Qt::FindDirectChildrenOnly).size() == 1);
    }

    static bool check(QImage *image, QImage::Format format, const QSize &size) {
        return image->format() == format && image->size() == size;
    }

    QImage *create(QImage::Format format, const QSize &size) {
        return new QImage(size, format);
    }

    static void destroy(QImage *image) {
        delete image;
    }
};

class Q_DECL_HIDDEN SoftwareNode : public DBackdropNode {
public:
    SoftwareNode(QQuickItem *item)
        : DBackdropNode(item, new QSGPlainTexture)
    {
        texture()->setOwnsTexture(false);
        // Ensuse always render on software renderer
        texture()->setHasAlphaChannel(true);
    }

    ~SoftwareNode() {
        destroy();
    }

    void releaseResources() override {
        destroy();
    }

    QImage toImage() const override
    {
        return image ? *image->data : QImage();
    }

    void render(const RenderState *state) override {
        Q_UNUSED(state)
        auto window = renderWindow();
        if (!window)
            return;
        QSGRendererInterface *rif = window->rendererInterface();
        QPainter *p = static_cast<QPainter *>(rif->getResource(window,
                                                               QSGRendererInterface::PainterResource));
        Q_ASSERT(p);

        const auto matrix = *this->matrix();
        const auto oldManager = manager;
        manager = QImageManager::resolve(manager, window);

        if (oldManager != manager) {
            texture()->setTexture(nullptr);
            if (oldManager && image)
                oldManager->release(image);
        }

        const bool hasRotation = matrix.flags().testAnyFlags(QMatrix4x4::Rotation2D | QMatrix4x4::Rotation);
        QSizeF size;

        if (hasRotation) {
            size = mapSize(m_rect, matrix);
        } else {
            size = matrix.mapRect(m_rect).size();
        }

        if (size.isEmpty()) {
            reset();
            return;
        }

        const qreal dpr = effectiveDevicePixelRatio();
        size *= dpr;
        const QSize pixelSize = size.toSize();
        const auto device = p->device();

        QImage sourceImage;
        QPixmap sourcePixmap;

        if (device->devType() == QInternal::Image) {
            sourceImage = *static_cast<QImage*>(device);
        } else if (device->devType() == QInternal::Pixmap) {
            sourcePixmap = *static_cast<QPixmap*>(device);
        } else {
            return;
        }

        if (Q_UNLIKELY(sourceImage.isNull())) {
            image = manager->resolve(image, QImage::Format_RGB30, pixelSize);
        } else {
            image = manager->resolve(image, sourceImage.format(), pixelSize);
        }

        painter.begin(image->data);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        auto transform = matrix.toTransform().inverted();
        QTransform resetPos;
        resetPos.translate((dpr - 1) * transform.dx(),
                           (dpr - 1) * transform.dy());
        painter.setTransform(transform * resetPos);

        // TODO: copy damage area from the previous frame
        if (Q_UNLIKELY(sourceImage.isNull())) {
            painter.drawPixmap(sourcePixmap.rect(), sourcePixmap, sourcePixmap.rect());
        } else {
            painter.drawImage(sourceImage.rect(), sourceImage, sourceImage.rect());
        }

        painter.end();

        texture()->setImage(*image->data);
        // Ensuse always render on software renderer
        texture()->setHasAlphaChannel(true);
        doNotifyTextureChanged();
    }

private:
    inline QSGPlainTexture *texture() const {
        return static_cast<QSGPlainTexture*>(m_texture.get());
    }

    void reset(bool notifyTexture = true) {
        if (!texture()->image().isNull() && notifyTexture)
            doNotifyTextureChanged();
        texture()->setTexture(nullptr);
        if (image && manager)
            manager->release(image);
        image = nullptr;
    }

    void destroy() {
        reset(false);
        manager = nullptr;
        image = nullptr;
    }

    friend class DBackdropNode;
    DataManagerPointer<QImageManager> manager;
    QImageManager::Data *image = nullptr;
    QPainter painter;
};

DBackdropNode *DBackdropNode::createSoftwareNode(QQuickItem *item)
{
    auto node = new SoftwareNode(item);
    return node;
}

QRectF DBackdropNode::rect() const
{
    return QRectF(0, 0, m_item->width(), m_item->height());
}

DBackdropNode::RenderingFlags DBackdropNode::flags() const
{
    return BoundedRectRendering;
}

void DBackdropNode::resize(const QSizeF &size)
{
    if (m_size == size)
        return;
    m_size = size;
    m_rect = QRectF(QPointF(0, 0), m_size);
}

void DBackdropNode::setContentItem(QQuickItem *item)
{
    if (m_content == item)
        return;
    m_content = item;
    markDirty(DirtyMaterial);
}

void DBackdropNode::setTextureChangedCallback(TextureChangedNotifer callback, void *data)
{
    m_renderCallback = callback;
    m_callbackData = data;
}

QQuickWindow *DBackdropNode::renderWindow() const
{
    return m_item ? m_item->window() : nullptr;
}

qreal DBackdropNode::effectiveDevicePixelRatio() const
{
    auto window = renderWindow();
    return window ? window->effectiveDevicePixelRatio() : 1.0;
}

DBackdropNode::DBackdropNode(QQuickItem *item, QSGTexture *texture)
    : m_item(item)
    , m_texture(texture)
{

}

DQUICK_END_NAMESPACE

#include "dbackdropnode.moc"
