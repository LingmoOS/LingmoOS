#include "mattingimageprovider.h"


MattingImageProvider::MattingImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage MattingImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    if (m_image.isNull()) {
        m_image.load("qrc:/res/res/damaged_img.png");
    }

    return m_image;
}

void MattingImageProvider::setImage(QImage &img)
{
    m_image = img;
}

ShowMattingImage::ShowMattingImage(QObject *parent)
    : QObject(parent)
{
    m_pImgProvider = new MattingImageProvider();
}

void ShowMattingImage::setImage(QImage &img)
{
    getImgProvider()->setImage(img);
    Q_EMIT callQmlRefeshImg();
}

MattingImageProvider* ShowMattingImage::getImgProvider()
{
    return m_pImgProvider;
}
