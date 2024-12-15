// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dopenglpaintdevice.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <QOpenGLTextureBlitter>
#include <QMatrix4x4>
#include <QColor>
#include <private/qopenglextensions_p.h>
#include <private/qopenglcontext_p.h>
#include <private/qopenglpaintdevice_p.h>

QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha);
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

// GLES2 builds won't have these constants with the suffixless names
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

class DOpenGLPaintDevicePrivate : public QOpenGLPaintDevicePrivate
{
    Q_DECLARE_PUBLIC(DOpenGLPaintDevice)
public:
    DOpenGLPaintDevicePrivate(DOpenGLPaintDevice *qq, QSurface *surface, QOpenGLContext *shareContext, DOpenGLPaintDevice::UpdateBehavior updateBehavior)
        : QOpenGLPaintDevicePrivate(QSize())
        , q_ptr(qq)
        , updateBehavior(updateBehavior)
        , hasFboBlit(false)
        , shareContext(shareContext)
        , targetSurface(surface)
    {
        if (!shareContext)
            this->shareContext = qt_gl_global_share_context();
    }

    ~DOpenGLPaintDevicePrivate();

    static DOpenGLPaintDevicePrivate *get(DOpenGLPaintDevice *w) { return w->d_func(); }

    void bindFBO();
    void initialize();

    void beginPaint() override;
    void endPaint() override;

    DOpenGLPaintDevice *q_ptr;

    DOpenGLPaintDevice::UpdateBehavior updateBehavior;
    bool hasFboBlit;
    QScopedPointer<QOpenGLContext> context;
    QOpenGLContext *shareContext;
    QScopedPointer<QOpenGLFramebufferObject> fbo;
    QOpenGLTextureBlitter blitter;
    QColor backgroundColor;
    QSurface *targetSurface;
    bool builtinSurface;
};

DOpenGLPaintDevicePrivate::~DOpenGLPaintDevicePrivate()
{
    Q_Q(DOpenGLPaintDevice);
    if (q->isValid()) {
        q->makeCurrent(); // this works even when the platformwindow is destroyed
        fbo.reset(nullptr);
        blitter.destroy();
        q->doneCurrent();
    }

    if (builtinSurface && targetSurface) {
        delete targetSurface;
    }
}

void DOpenGLPaintDevicePrivate::initialize()
{
    if (context)
        return;

    if (builtinSurface) {
        static_cast<QOffscreenSurface*>(targetSurface)->create();
    }

    if (!targetSurface->surfaceHandle())
        qWarning("Attempted to initialize DOpenGLPaintDevice without a platform surface");

    context.reset(new QOpenGLContext);
    context->setShareContext(shareContext);
    context->setFormat(targetSurface->format());
    if (!context->create())
        qWarning("DOpenGLPaintDevice::beginPaint: Failed to create context");
    if (!context->makeCurrent(targetSurface))
        qWarning("DOpenGLPaintDevice::beginPaint: Failed to make context current");

    if (updateBehavior == DOpenGLPaintDevice::PartialUpdateBlit)
        hasFboBlit = QOpenGLFramebufferObject::hasOpenGLFramebufferBlit();

    ctx = context.data();
}

static int defaultSamples(int fallback)
{
    bool envIsIntValue = false;
    int global_samples = qEnvironmentVariableIntValue("D_GL_PAINT_SAMPLES", &envIsIntValue);

    return envIsIntValue ? global_samples : fallback;
}

void DOpenGLPaintDevicePrivate::beginPaint()
{
    Q_Q(DOpenGLPaintDevice);

    initialize();
    context->makeCurrent(targetSurface);

    const int deviceWidth = q->width() * q->devicePixelRatio();
    const int deviceHeight = q->height() * q->devicePixelRatio();
    const QSize deviceSize(deviceWidth, deviceHeight);
    if (updateBehavior > DOpenGLPaintDevice::NoPartialUpdate) {
        if (!fbo || fbo->size() != deviceSize) {
            QOpenGLFramebufferObjectFormat fboFormat;
            fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            int samples = targetSurface->format().samples();

            // set the default samples
            if (samples < 0) {
                static int global_samples = defaultSamples(4);
                samples = global_samples;
            }

            if (updateBehavior != DOpenGLPaintDevice::PartialUpdateBlend)
                fboFormat.setSamples(samples);
            else
                qWarning("DOpenGLPaintDevice: PartialUpdateBlend does not support multisampling");
            fbo.reset(new QOpenGLFramebufferObject(deviceSize, fboFormat));
        }
    }

    context->functions()->glViewport(0, 0, deviceWidth, deviceHeight);
    context->functions()->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());

    if (updateBehavior > DOpenGLPaintDevice::NoPartialUpdate)
        fbo->bind();
}

void DOpenGLPaintDevicePrivate::endPaint()
{
    Q_Q(DOpenGLPaintDevice);

    if (updateBehavior > DOpenGLPaintDevice::NoPartialUpdate)
        fbo->release();

    context->functions()->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());

    if (updateBehavior == DOpenGLPaintDevice::PartialUpdateBlit && hasFboBlit) {
        const int deviceWidth = q->width() * q->devicePixelRatio();
        const int deviceHeight = q->height() * q->devicePixelRatio();
        QOpenGLExtensions extensions(context.data());
        extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle());
        extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->defaultFramebufferObject());
        extensions.glBlitFramebuffer(0, 0, deviceWidth, deviceHeight,
                                     0, 0, deviceWidth, deviceHeight,
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
    } else if (updateBehavior > DOpenGLPaintDevice::NoPartialUpdate) {
        if (updateBehavior == DOpenGLPaintDevice::PartialUpdateBlend) {
            context->functions()->glEnable(GL_BLEND);
            context->functions()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (!blitter.isCreated())
            blitter.create();

        QRect windowRect(QPoint(0, 0), fbo->size());
        QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(windowRect, windowRect);
        blitter.bind();
        blitter.blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
        blitter.release();

        if (updateBehavior == DOpenGLPaintDevice::PartialUpdateBlend)
            context->functions()->glDisable(GL_BLEND);
    }
}

void DOpenGLPaintDevicePrivate::bindFBO()
{
    if (updateBehavior > DOpenGLPaintDevice::NoPartialUpdate)
        fbo->bind();
    else
        QOpenGLFramebufferObject::bindDefault();
}

/*!
  Constructs a new DOpenGLPaintDevice with the given \a parent and \a updateBehavior.

  \sa DOpenGLPaintDevice::UpdateBehavior
 */
DOpenGLPaintDevice::DOpenGLPaintDevice(QSurface *surface, DOpenGLPaintDevice::UpdateBehavior updateBehavior)
    : QOpenGLPaintDevice(*new DOpenGLPaintDevicePrivate(this, surface, nullptr, updateBehavior))
{
    setSize(surface->size());
    d_func()->builtinSurface = false;
}

DOpenGLPaintDevice::DOpenGLPaintDevice(const QSize &size, DOpenGLPaintDevice::UpdateBehavior updateBehavior)
    : QOpenGLPaintDevice(*new DOpenGLPaintDevicePrivate(this, new QOffscreenSurface(), nullptr, updateBehavior))
{
    setSize(size);
    d_func()->builtinSurface = true;
}

/*!
  Constructs a new DOpenGLPaintDevice with the given \a parent and \a updateBehavior. The DOpenGLPaintDevice's context will share with \a shareContext.

  \sa DOpenGLPaintDevice::UpdateBehavior shareContext
*/
DOpenGLPaintDevice::DOpenGLPaintDevice(QOpenGLContext *shareContext, QSurface *surface, UpdateBehavior updateBehavior)
    : QOpenGLPaintDevice(*new DOpenGLPaintDevicePrivate(this, surface, shareContext, updateBehavior))
{
    setSize(surface->size());
    d_func()->builtinSurface = false;
}

DOpenGLPaintDevice::DOpenGLPaintDevice(QOpenGLContext *shareContext, const QSize &size, DOpenGLPaintDevice::UpdateBehavior updateBehavior)
    : QOpenGLPaintDevice(*new DOpenGLPaintDevicePrivate(this, new QOffscreenSurface(), shareContext, updateBehavior))
{
    setSize(size);
    d_func()->builtinSurface = true;
}

/*!
  Destroys the DOpenGLPaintDevice instance, freeing its resources.

  The OpenGLWindow's context is made current in the destructor, allowing for
  safe destruction of any child object that may need to release OpenGL
  resources belonging to the context provided by this window.

  \warning if you have objects wrapping OpenGL resources (such as
  QOpenGLBuffer, QOpenGLShaderProgram, etc.) as members of a DOpenGLPaintDevice
  subclass, you may need to add a call to makeCurrent() in that subclass'
  destructor as well. Due to the rules of C++ object destruction, those objects
  will be destroyed \e{before} calling this function (but after that the
  destructor of the subclass has run), therefore making the OpenGL context
  current in this function happens too late for their safe disposal.

  \sa makeCurrent

  \since 5.5
*/
DOpenGLPaintDevice::~DOpenGLPaintDevice()
{
    makeCurrent();
}

void DOpenGLPaintDevice::resize(const QSize &size)
{
    Q_ASSERT(!paintingActive());
    setSize(size);

    Q_D(DOpenGLPaintDevice);

    if (d->fbo)
        d->fbo.reset(nullptr);
}

/*!
  \return the update behavior for this DOpenGLPaintDevice.
*/
DOpenGLPaintDevice::UpdateBehavior DOpenGLPaintDevice::updateBehavior() const
{
    Q_D(const DOpenGLPaintDevice);
    return d->updateBehavior;
}

/*!
  \return \c true if the window's OpenGL resources, like the context, have
  been successfully initialized. Note that the return value is always \c false
  until the window becomes exposed (shown).
*/
bool DOpenGLPaintDevice::isValid() const
{
    Q_D(const DOpenGLPaintDevice);
    return d->context && d->context->isValid();
}

/*!
  Prepares for rendering OpenGL content for this window by making the
  corresponding context current and binding the framebuffer object, if there is
  one, in that context context.

  It is not necessary to call this function in most cases, because it is called
  automatically before invoking paintGL(). It is provided nonetheless to support
  advanced, multi-threaded scenarios where a thread different than the GUI or main
  thread may want to update the surface or framebuffer contents. See QOpenGLContext
  for more information on threading related issues.

  This function is suitable for calling also when the underlying platform window
  is already destroyed. This means that it is safe to call this function from
  a DOpenGLPaintDevice subclass' destructor. If there is no native window anymore,
  an offscreen surface is used instead. This ensures that OpenGL resource
  cleanup operations in the destructor will always work, as long as
  this function is called first.

  \sa QOpenGLContext, context(), paintGL(), doneCurrent()
 */
void DOpenGLPaintDevice::makeCurrent()
{
    Q_D(DOpenGLPaintDevice);

    if (!isValid())
        return;

    // The platform window may be destroyed at this stage and therefore
    // makeCurrent() may not safely be called with 'this'.
    d->context->makeCurrent(d->targetSurface);
    d->bindFBO();
}

/*!
  Releases the context.

  It is not necessary to call this function in most cases, since the widget
  will make sure the context is bound and released properly when invoking
  paintGL().

  \sa makeCurrent()
 */
void DOpenGLPaintDevice::doneCurrent()
{
    Q_D(DOpenGLPaintDevice);

    if (!isValid())
        return;

    d->context->doneCurrent();
}

void DOpenGLPaintDevice::flush()
{
    Q_D(DOpenGLPaintDevice);
    d->context->makeCurrent(d->targetSurface);
    d->context->swapBuffers(d->targetSurface);
}

/*!
  \return The QOpenGLContext used by this window or \c 0 if not yet initialized.
 */
QOpenGLContext *DOpenGLPaintDevice::context() const
{
    Q_D(const DOpenGLPaintDevice);
    return d->context.data();
}

/*!
  \return The QOpenGLContext requested to be shared with this window's QOpenGLContext.
*/
QOpenGLContext *DOpenGLPaintDevice::shareContext() const
{
    Q_D(const DOpenGLPaintDevice);
    return d->shareContext;
}

/*!
  The framebuffer object handle used by this window.

  When the update behavior is set to \c NoPartialUpdate, there is no separate
  framebuffer object. In this case the returned value is the ID of the
  default framebuffer.

  Otherwise the value of the ID of the framebuffer object or \c 0 if not
  yet initialized.
 */
GLuint DOpenGLPaintDevice::defaultFramebufferObject() const
{
    Q_D(const DOpenGLPaintDevice);
    if (d->updateBehavior > NoPartialUpdate && d->fbo)
        return d->fbo->handle();
    else if (QOpenGLContext *ctx = QOpenGLContext::currentContext())
        return ctx->defaultFramebufferObject();
    else
        return 0;
}

/*!
  Returns a copy of the framebuffer.

  \note This is a potentially expensive operation because it relies on
  glReadPixels() to read back the pixels. This may be slow and can stall the
  GPU pipeline.

  \note When used together with update behavior \c NoPartialUpdate, the returned
  image may not contain the desired content when called after the front and back
  buffers have been swapped (unless preserved swap is enabled in the underlying
  windowing system interface). In this mode the function reads from the back
  buffer and the contents of that may not match the content on the screen (the
  front buffer). In this case the only place where this function can safely be
  used is paintGL() or paintOverGL().
 */
QImage DOpenGLPaintDevice::grabFramebuffer()
{
    if (!isValid())
        return QImage();

    makeCurrent();

    const bool hasAlpha = context()->format().hasAlpha();
    QImage img = qt_gl_read_framebuffer(QSize(width(), height()) * devicePixelRatio(), hasAlpha, hasAlpha);
    img.setDevicePixelRatio(devicePixelRatio());
    return img;
}

void DOpenGLPaintDevice::ensureActiveTarget()
{
    Q_D(DOpenGLPaintDevice);

    d->initialize();
    d->context->makeCurrent(d->targetSurface);
}

DPP_END_NAMESPACE
