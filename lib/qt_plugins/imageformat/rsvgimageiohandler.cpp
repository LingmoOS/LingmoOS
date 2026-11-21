#include "rsvgimageiohandler.h"

#include <QImage>
#include <QIODevice>
#include <QVariant>

#include <librsvg/rsvg.h>

#include <cmath>
#include <limits>
#include <memory>

namespace {

struct CairoContextDeleter
{
    void operator()(cairo_t *context) const noexcept
    {
        if (context) {
            cairo_destroy(context);
        }
    }
};

struct CairoSurfaceDeleter
{
    void operator()(cairo_surface_t *surface) const noexcept
    {
        if (surface) {
            cairo_surface_destroy(surface);
        }
    }
};

struct GErrorDeleter
{
    void operator()(GError *error) const noexcept
    {
        if (error) {
            g_error_free(error);
        }
    }
};

struct RsvgHandleDeleter
{
    void operator()(RsvgHandle *handle) const noexcept
    {
        if (handle) {
            g_object_unref(handle);
        }
    }
};

using CairoContextPtr = std::unique_ptr<cairo_t, CairoContextDeleter>;
using CairoSurfacePtr = std::unique_ptr<cairo_surface_t, CairoSurfaceDeleter>;
using GErrorPtr = std::unique_ptr<GError, GErrorDeleter>;
using RsvgHandlePtr = std::unique_ptr<RsvgHandle, RsvgHandleDeleter>;

} // namespace

static RsvgHandlePtr createHandle(const QByteArray &data)
{
    GError *rawError = nullptr;
    RsvgHandlePtr handle(rsvg_handle_new_from_data(
        reinterpret_cast<const guint8 *>(data.constData()),
        static_cast<gsize>(data.size()),
        &rawError));
    GErrorPtr error(rawError);
    return handle;
}

static QByteArray readDocument(QIODevice *device)
{
    const qint64 position = device->pos();
    QByteArray data = device->readAll();
    if (!device->isSequential() && position >= 0) {
        device->seek(position);
    }

    return data;
}

static int toPixelDimension(gdouble dimension)
{
    if (!std::isfinite(dimension) || dimension <= 0.0
        || dimension > static_cast<gdouble>(std::numeric_limits<int>::max())) {
        return 0;
    }

    return static_cast<int>(std::ceil(dimension));
}

RsvgImageIOHandler::~RsvgImageIOHandler()
{
    if (m_handle) {
        g_object_unref(m_handle);
    }
}

bool RsvgImageIOHandler::hasSvgContent(QIODevice *device)
{
    return device->peek(64 * 1024).toLower().contains("<svg");
}

bool RsvgImageIOHandler::canRead() const
{
    if (!device() || !device()->isReadable()) {
        return false;
    }

    return format().compare("svg", Qt::CaseInsensitive) == 0
        || hasSvgContent(device());
}

bool RsvgImageIOHandler::read(QImage *image)
{
    if (!image || !loadIntrinsicSize()) {
        return false;
    }

    const QSize targetSize = m_scaledSize.isValid() ? m_scaledSize : m_intrinsicSize;
    return render(targetSize, image);
}

QVariant RsvgImageIOHandler::option(ImageOption option) const
{
    switch (option) {
    case Size:
        return loadIntrinsicSize() ? QVariant::fromValue(m_intrinsicSize) : QVariant();
    case ScaledSize:
        return m_scaledSize;
    default:
        return QImageIOHandler::option(option);
    }
}

void RsvgImageIOHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == ScaledSize) {
        m_scaledSize = value.toSize();
        return;
    }

    QImageIOHandler::setOption(option, value);
}

bool RsvgImageIOHandler::supportsOption(ImageOption option) const
{
    return option == Size || option == ScaledSize || QImageIOHandler::supportsOption(option);
}

bool RsvgImageIOHandler::loadIntrinsicSize() const
{
    if (m_metadataLoaded) {
        return m_handle != nullptr;
    }

    m_metadataLoaded = true;
    if (!device()) {
        return false;
    }

    const QByteArray data = readDocument(device());
    if (data.isEmpty()) {
        return false;
    }

    RsvgHandlePtr handle = createHandle(data);
    if (!handle) {
        return false;
    }

    gdouble width = 0.0;
    gdouble height = 0.0;
    if (!rsvg_handle_get_intrinsic_size_in_pixels(handle.get(), &width, &height)) {
        gboolean hasViewbox = false;
        RsvgRectangle viewbox {};
        rsvg_handle_get_intrinsic_dimensions(handle.get(),
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             &hasViewbox,
                                             &viewbox);
        if (!hasViewbox || viewbox.width <= 0.0 || viewbox.height <= 0.0) {
            return false;
        }
        width = viewbox.width;
        height = viewbox.height;
    }

    const int pixelWidth = toPixelDimension(width);
    const int pixelHeight = toPixelDimension(height);
    if (pixelWidth <= 0 || pixelHeight <= 0) {
        return false;
    }

    m_intrinsicSize = QSize(pixelWidth, pixelHeight);
    m_handle = handle.release();
    return true;
}

bool RsvgImageIOHandler::render(const QSize &size, QImage *image) const
{
    QImage target(size, QImage::Format_ARGB32_Premultiplied);
    if (target.isNull()) {
        return false;
    }
    target.fill(Qt::transparent);

    const int cairoStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, size.width());
    if (cairoStride != target.bytesPerLine()) {
        return false;
    }

    CairoSurfacePtr surface(cairo_image_surface_create_for_data(
        target.bits(),
        CAIRO_FORMAT_ARGB32,
        size.width(),
        size.height(),
        target.bytesPerLine()));
    if (!surface || cairo_surface_status(surface.get()) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    CairoContextPtr context(cairo_create(surface.get()));
    if (!context || cairo_status(context.get()) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    const RsvgRectangle viewport {
        0.0,
        0.0,
        static_cast<gdouble>(size.width()),
        static_cast<gdouble>(size.height())
    };

    GError *rawError = nullptr;
    const gboolean rendered = rsvg_handle_render_document(m_handle, context.get(), &viewport, &rawError);
    GErrorPtr error(rawError);
    if (!rendered || cairo_status(context.get()) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    context.reset();
    cairo_surface_flush(surface.get());
    *image = target;
    return true;
}
