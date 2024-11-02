#include "albumthumbnail.h"

AlbumThumbnail::AlbumThumbnail(const QString &path, const QString &realFormat)
{
    m_path = path;
    m_realFormat = realFormat;
}

void AlbumThumbnail::run()
{
    ImageAndInfo package;
    MatAndFileinfo maf = File::loadImage(m_path, m_realFormat, IMREAD_REDUCED_COLOR_8);
    if (maf.mat.data) {
        QPixmap pix = Processing::converFormat(maf.mat);
        pix = pix.scaled(Variable::ALBUM_IMAGE_SIZE, Qt::KeepAspectRatio); // 快速缩放
        package.image = changImageSize(pix);
    }
    package.info = maf.info;
    QVariant var;
    var.setValue<ImageAndInfo>(package);
    Q_EMIT albumFinish(var);
}

QPixmap AlbumThumbnail::changImageSize(const QPixmap &pix)
{
    if (pix.size() == QSize(Variable::ALBUM_IMAGE_SIZE.width(), Variable::ALBUM_IMAGE_SIZE.height() - 2)) {
        return pix;
    }

    int w = 0;
    int h = 0;

    w = Variable::ALBUM_IMAGE_SIZE.width() - pix.width();
    h = Variable::ALBUM_IMAGE_SIZE.height() - pix.height();

    QPixmap newPix(QSize(Variable::ALBUM_IMAGE_SIZE.width(), Variable::ALBUM_IMAGE_SIZE.height() - 2));
    newPix.fill(Qt::transparent);
    QPainter painter(&newPix);
    painter.drawPixmap(w / 2, h / 2, pix);
    return newPix;
}

AlbumRealImg::AlbumRealImg(const QString &path, const QString &realFormat)
{
    m_path = path;
    m_realFormat = realFormat;
}

void AlbumRealImg::run()
{
    MatAndFileinfo maf = File::loadImage(m_path, m_realFormat);

    QVariant var;
    QPair<MatAndFileinfo, QPixmap> matPix;
    matPix.first = maf;
    if (maf.mat.data) {
        QPixmap pix = Processing::converFormat(maf.mat);
        matPix.second = pix;
    }
    var.setValue<QPair<MatAndFileinfo, QPixmap>>(matPix);
    Q_EMIT albumRealFinish(var);
}
