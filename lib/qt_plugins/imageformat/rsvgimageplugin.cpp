#include "rsvgimageplugin.h"

#include "rsvgimageiohandler.h"

#include <QIODevice>

QImageIOPlugin::Capabilities RsvgImagePlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    const bool svgFormat = format.compare("svg", Qt::CaseInsensitive) == 0;
    if (!device) {
        return svgFormat ? Capabilities(CanRead) : Capabilities();
    }

    if (!device->isReadable()) {
        return {};
    }

    if (!format.isEmpty()) {
        return svgFormat ? Capabilities(CanRead) : Capabilities();
    }

    return RsvgImageIOHandler::hasSvgContent(device) ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *RsvgImagePlugin::create(QIODevice *device, const QByteArray &format) const
{
    auto *handler = new RsvgImageIOHandler;
    handler->setDevice(device);
    handler->setFormat(format.isEmpty() ? QByteArrayLiteral("svg") : format);
    return handler;
}
