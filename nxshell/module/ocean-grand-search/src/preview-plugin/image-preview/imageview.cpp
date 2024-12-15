// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreview_global.h"
#include "imageview.h"
#include "global/commontools.h"

#include <DLabel>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QDebug>
#include <QtConcurrent>
#include <QPainter>
#include <QUrl>
#include <QHBoxLayout>
#include <QImageReader>
#include <QBitmap>
#include <QPainterPath>
#include <QFontMetrics>

#define IMAGEWIDTH      310
#define IMAGEHEIGHT     110
#define ROUNDRADIUS     8

DWIDGET_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::image_preview;

ImageView::ImageView(QWidget *parent)
    :DWidget (parent)
{
    initUI();
    initConnect();
}

ImageView::~ImageView()
{
    if (m_movie)
        m_movie->stop();
}

QSize ImageView::sourceSize()
{
    return m_sourceSize;
}

bool ImageView::stopPreview()
{
    if (m_movie)
        m_movie->stop();

    return true;
}

void ImageView::loadImage(const QString &file, const QString &type)
{
    m_imageFile = file;
    m_formats = type.toLocal8Bit();

    // 名称超长时省略中间部分,并增加提示
    QFileInfo fileInfo(m_imageFile);
    QString name = fileInfo.fileName();
    m_titleLabel->setElideMode(Qt::ElideMiddle);
    m_titleLabel->setText(name);

    QFontMetrics fontMetrics(m_titleLabel->font());
    int textWidth = fontMetrics.size(Qt::TextSingleLine, name).width();
    if (textWidth >= m_titleLabel->width())
        m_titleLabel->setToolTip(name);

    if (!fileInfo.isReadable() || !canPreview()) {
        showErrorPage();
        return;
    }

    if (m_formats == QByteArrayLiteral("gif")) {
        m_isMovie = true;
        m_movie = new QMovie(this);
        m_movie->setFileName(m_imageFile);
        m_movie->start();    // 启动后才能获取到正确的frameRect

        connect(m_movie, &QMovie::error, this, &ImageView::showErrorPage);

        m_sourceSize = m_movie->frameRect().size();
        m_movie->stop();
    } else {
        m_isMovie = false;
        bool success = m_image.load(m_imageFile, m_formats);
        if (!success) {
            showErrorPage();
            return;
        }
        m_sourceSize = m_image.size();
    }

    // 宽高等比缩放，满足宽和高哪个定格，就以哪个为准调整另一个做等比
    QSize showSize = m_sourceSize;
    qreal wZoom = qreal(IMAGEWIDTH) / showSize.width();
    qreal hZoom = qreal(IMAGEHEIGHT) / showSize.height();
    qreal zoom = wZoom > hZoom ? hZoom : wZoom;
    // 原始尺寸小于缩略图区域尺寸，不做拉伸
    if (zoom > 1)
        zoom = 1;
    int w = int(showSize.width() * zoom);
    int h = int(showSize.height() * zoom);
    m_imageLabel->setFixedSize(w, h);

    if (m_isMovie) {
        m_movie->setScaledSize(QSize(w, h));
        connect(m_movie, &QMovie::frameChanged, this, &ImageView::onMovieFrameChanged);
        m_movie->start();
    } else {
        QPixmap roundPixmap = getRoundPixmap(QPixmap::fromImage(m_image).scaled(w, h));
        m_imageLabel->setPixmap(roundPixmap);
    }

}

void ImageView::initUI()
{
    setFixedHeight(150);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(IMAGEWIDTH, IMAGEHEIGHT);
    m_imageLabel->setAlignment(Qt::AlignCenter);

    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedWidth(IMAGEWIDTH);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    QFont titleFont = m_titleLabel->font();
    titleFont.setWeight(QFont::Medium);
    titleFont = DFontSizeManager::instance()->get(DFontSizeManager::T5, titleFont);
    m_titleLabel->setFont(titleFont);

    QColor textColor(0, 0, 0, int(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.9));
    QPalette pa = m_titleLabel->palette();
    pa.setColor(QPalette::WindowText, textColor);
    m_titleLabel->setPalette(pa);

    QHBoxLayout *imageLayout = new QHBoxLayout;
    imageLayout->addWidget(m_imageLabel);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->addWidget(m_titleLabel);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(35, 12, 35, 0);
    m_mainLayout->setSpacing(0);

    m_mainLayout->addStretch();
    m_mainLayout->addLayout(imageLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(titleLayout);

    this->setLayout(m_mainLayout);
}

void ImageView::initConnect()
{

}

bool ImageView::canPreview()
{
    QByteArray f = QImageReader::imageFormat(m_imageFile);

    if (f.isEmpty()) {
        QMimeDatabase mimeDatabase;
        const QMimeType &mt = mimeDatabase.mimeTypeForFile(m_imageFile, QMimeDatabase::MatchContent);

        f = mt.preferredSuffix().toLatin1();

        if (f.isEmpty()) {
            m_formats.clear();
            return false;
        }
    }

    m_formats = f;

    return QImageReader::supportedImageFormats().contains(f);
}

QPixmap ImageView::getRoundPixmap(const QPixmap &pixmap)
{
    int w = m_imageLabel->width();
    int h = m_imageLabel->height();

    QPixmap roundPixmap(w, h);
    roundPixmap.fill(Qt::transparent);
    QPainter painter(&roundPixmap);
    painter.setRenderHints(QPainter::Antialiasing, true);           // 抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform, true);  // 平滑

    QPainterPath path;
    QRect rect(0, 0, w, h);
    path.addRoundedRect(rect, ROUNDRADIUS, ROUNDRADIUS);            // 圆角
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, w, h, pixmap);

    return roundPixmap;
}

void ImageView::onMovieFrameChanged(int frameNumber)
{
    Q_UNUSED(frameNumber)

    Q_ASSERT(m_movie);

    QPixmap roundPixmap = getRoundPixmap(m_movie->currentPixmap());

    m_imageLabel->setPixmap(roundPixmap);
}

void ImageView::showErrorPage()
{
    m_imageLabel->setFixedSize(192, 108);
    QImage errorImg(":/icons/image_damaged.svg");
    errorImg = errorImg.scaled(46, 46);
    errorImg = CommonTools::creatErrorImage({192, 108}, errorImg);

    auto errorPixmap = getRoundPixmap(QPixmap::fromImage(errorImg));
    m_imageLabel->setPixmap(errorPixmap);
}

