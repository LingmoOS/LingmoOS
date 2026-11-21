#ifndef RSVGIMAGEPLUGIN_H
#define RSVGIMAGEPLUGIN_H

#include <QImageIOPlugin>

class RsvgImagePlugin final : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "rsvgimage.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // RSVGIMAGEPLUGIN_H
