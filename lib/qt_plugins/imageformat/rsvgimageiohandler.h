#ifndef RSVGIMAGEIOHANDLER_H
#define RSVGIMAGEIOHANDLER_H

#include <QImageIOHandler>
#include <QSize>

typedef struct _RsvgHandle RsvgHandle;

class RsvgImageIOHandler final : public QImageIOHandler
{
public:
    ~RsvgImageIOHandler() override;

    static bool hasSvgContent(QIODevice *device);

    bool canRead() const override;
    bool read(QImage *image) override;

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

private:
    bool loadIntrinsicSize() const;
    bool render(const QSize &size, QImage *image) const;

    // Parsed by loadIntrinsicSize() and kept for render()
    mutable RsvgHandle *m_handle = nullptr;
    mutable QSize m_intrinsicSize;
    mutable bool m_metadataLoaded = false;
    QSize m_scaledSize;
};

#endif // RSVGIMAGEIOHANDLER_H
