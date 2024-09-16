// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "slideshowscreensaver.h"
#include "slideshowconfig.h"

#include <DApplication>

#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QWindow>
#include <QScreen>
#include <QPainter>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QImageReader>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

#define IMAGE_MAX_SIZE 30 * 1024 * 1024   // Only display image smaller than 30M

DWIDGET_USE_NAMESPACE

SlideshowScreenSaver::SlideshowScreenSaver(bool subWindow, QWidget *parent)
    : QWidget(parent), m_subWindow(subWindow)
{
    if (m_subWindow)
        setWindowFlag(Qt::WindowTransparentForInput, true);

    m_shuffle = SlideshowConfig::instance()->isShuffle();
    m_intervalTime = SlideshowConfig::instance()->intervalTime();

    m_timer.reset(new QTimer);
    connect(m_timer.get(), &QTimer::timeout, this, &SlideshowScreenSaver::onUpdateImage);
    m_timer->setInterval(m_intervalTime * 1000);
}

SlideshowScreenSaver::~SlideshowScreenSaver()
{
}

bool SlideshowScreenSaver::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
        int type = (event->response_type & ~0x80);
        if (XCB_CONFIGURE_NOTIFY == type) {
            xcb_configure_notify_event_t *ce = reinterpret_cast<xcb_configure_notify_event_t *>(event);
            qInfo() << "parent window size changed" << ce->width << ce->height;
            QSize widSize = mapFromHandle(QSize(ce->width, ce->height));
            if (widSize != size()) {
                qInfo() << "update window size:" << widSize;
                resize(widSize);
            }
        } else if (XCB_DESTROY_NOTIFY == type) {
            xcb_destroy_notify_event_t *ce = reinterpret_cast<xcb_destroy_notify_event_t *>(event);
            if (ce->window == Window(this->windowHandle()->winId())) {
                qInfo() << "parent window closed";
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            }
        }
    }
    return false;
}

void SlideshowScreenSaver::init()
{
    loadSlideshowImages();

    if (!m_playOrder.isEmpty())
        m_timer->start();
}

void SlideshowScreenSaver::onUpdateImage()
{
    if (!m_playOrder.isEmpty()) {
        if (m_currentIndex == m_playOrder.count()) {
            if (m_shuffle) {
                randomImageIndex();
            } else {
                m_currentIndex = 1;
            }
            filterInvalidFile(m_playOrder.value(m_currentIndex));
        } else {
            nextValidPath();
        }
    } else {
        loadSlideshowImages();
    }

    m_pixmap.reset(new QPixmap(m_playOrder.value(m_currentIndex)));
    scaledPixmap();
    update();
    return;
}

void SlideshowScreenSaver::paintEvent(QPaintEvent *event)
{
    if (m_pixmap && !m_pixmap->isNull()) {
        QPainter pa(this);
        pa.drawPixmap(event->rect().topLeft(), *m_pixmap.data(), QRectF(QPointF(event->rect().topLeft()), QSizeF(event->rect().size())));

    } else {
        showDefaultBlack(event);
    }

    return QWidget::paintEvent(event);
}

void SlideshowScreenSaver::closeEvent(QCloseEvent *event)
{
    for (auto wid : findChildren<QWidget *>())
        wid->close();

    QWidget::closeEvent(event);
}

QSize SlideshowScreenSaver::mapFromHandle(const QSize &handleSize)
{
    qreal ratio = devicePixelRatioF();
    qDebug() << "parent window handle size" << handleSize << "devicePixelRatio" << ratio;

    if (ratio > 0 && ratio != 1.0)
        return handleSize / ratio;
    else
        return handleSize;
}

void SlideshowScreenSaver::showDefaultBlack(QPaintEvent *event)
{
    qreal scale = devicePixelRatioF();
    QPixmap pip(this->geometry().size());
    pip.fill(Qt::black);

    QPainter pa(&pip);
    pa.setPen(Qt::white);
    pa.drawText(pip.rect(), Qt::AlignCenter,
                tr("Please select a valid image path in the Custom Screensaver \"Screensaver Setting\"."));

    const auto &pix = pip.scaled(mapFromHandle(this->geometry().size()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPainter p(this);
    p.drawPixmap(event->rect().topLeft(), pix, QRectF(QPointF(event->rect().topLeft()) * scale, QSizeF(event->rect().size()) * scale));
}

void SlideshowScreenSaver::randomImageIndex()
{
    // When "sise < 2", it's like play in sequence
    if (m_playOrder.size() > 2) {
        auto vct = m_playOrder.keys().toVector();
        std::mt19937 rg(std::random_device {}());
        std::shuffle(vct.begin(), vct.end(), rg);
        QMap<int, QString> temp;
        for (int i = 0; i < vct.size(); ++i)
            temp.insert(i + 1, m_playOrder.value(vct[i]));
        m_playOrder.clear();
        m_playOrder = std::move(temp);

        // make sure the display is different twice in a row
        m_currentIndex = (m_lastImage == m_playOrder.first()) ? 2 : 1;
        m_lastImage = m_playOrder.last();
    } else {
        m_currentIndex = 1;
    }
}

void SlideshowScreenSaver::loadSlideshowImages()
{
    m_playOrder.clear();
    QString path = SlideshowConfig::instance()->slideshowPath();

    QFileInfo fileInfo(path);
    if (fileInfo.exists() && fileInfo.isDir()) {
        QDir dir(path);
        QDir::Filters filters = QDir::Files | QDir::NoDotAndDotDot | QDir::Readable;
        QFileInfoList infoList = dir.entryInfoList(filters, QDir::Name);
        if (infoList.isEmpty()) {
            qWarning() << "Warning:no image file in " << path;
            m_timer->stop();
            return;
        }

        static const QStringList validSuffix { QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("bmp"), QStringLiteral("png") };
        int idx = 1;
        for (auto info : infoList) {
            if (info.size() < IMAGE_MAX_SIZE && validSuffix.contains(info.suffix(), Qt::CaseInsensitive)) {
                QScopedPointer<QPixmap> pixmapPointer(new QPixmap(info.absoluteFilePath()));
                if (pixmapPointer && !pixmapPointer->isNull()){
                    m_imagefiles.append(info.absoluteFilePath());   // 记录图片列表
                    m_playOrder.insert(idx, info.absoluteFilePath());
                }
                idx++;
            }
        }
    }

    initPixMap();
}

void SlideshowScreenSaver::initPixMap()
{
    if (!m_playOrder.isEmpty()) {
        if (m_shuffle)
            randomImageIndex();
        QPixmap pix(m_playOrder.first());
        if (pix.isNull()) {
            QImageReader reader(m_playOrder.first());
            reader.setDecideFormatFromContent(true);
            pix = QPixmap::fromImage(reader.read());
        }
        m_pixmap.reset(new QPixmap(pix));
        scaledPixmap();
        m_currentIndex = 1;
        m_lastImage = m_playOrder.last();
    } else {
        m_pixmap.reset(nullptr);
        m_timer->stop();
    }
}

void SlideshowScreenSaver::nextValidPath()
{
    const int num = m_playOrder.count();

    // It almost never happens
    if (++m_currentIndex > num)
        m_currentIndex = 1;

    filterInvalidFile(m_playOrder.value(m_currentIndex));
}

void SlideshowScreenSaver::filterInvalidFile(const QString &path)
{
    QFileInfo f(path);
    if (!f.exists())
        loadSlideshowImages();
}

void SlideshowScreenSaver::scaledPixmap()
{
    if (m_pixmap && !m_pixmap->isNull()) {
        auto pix = m_pixmap->scaled(mapFromHandle(this->geometry().size()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_pixmap->swap(pix);
    } else if (m_pixmap && !m_pixmap->isNull()) {
        loadSlideshowImages();
    }
}
