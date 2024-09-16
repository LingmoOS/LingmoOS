// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "commondefine.h"

#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

using namespace dfm_wallpapersetting;

PixmapProducer::PixmapProducer(QObject *parent) : QThread(parent)
{
    Q_ASSERT(parent);
}

void PixmapProducer::append(const PixmapProducer::PixmapInfo &info)
{
    condMtx.lock();
    infos.append(info);
    condMtx.unlock();

    cond.wakeAll();
}

void PixmapProducer::stop()
{
    condMtx.lock();
    running = false;
    condMtx.unlock();

    cond.wakeAll();
}

void PixmapProducer::launch()
{
    running = true;
    start();
}

void PixmapProducer::run()
{
    qDebug() << "preivew work thread run.";

    while(true) {
        condMtx.lock();
        if (!running) {
            condMtx.unlock();
            qInfo() << "preivew work thread exit";
            return;
        }

        if (infos.isEmpty())
            cond.wait(&condMtx);

        if (!running || infos.isEmpty()) {
            condMtx.unlock();
            continue;
        }

        auto info = infos.last();
        infos.clear();
        condMtx.unlock();

        QPixmap pixmap(info.path);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(info.size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmap.setDevicePixelRatio(info.ratio);
        }

        QMetaObject::invokeMethod(parent(), "setPixmap", Qt::QueuedConnection, Q_ARG(QPixmap, pixmap));
    }
}


PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , worker(new PixmapProducer(this))
{
    bkgColor = QColor("#868686");
    worker->launch();
}

PreviewWidget::~PreviewWidget()
{
    qDebug() << "wait wallaper thread finish..";
    worker->stop();
    worker->wait(1000);
    qInfo() << "wallaper thread is exited, release preview widget";
}

void PreviewWidget::updateImage()
{
    curSize = imageSize();
    worker->append(PixmapProducer::PixmapInfo{imgPath, curSize, devicePixelRatioF()});
}

void PreviewWidget::setImage(const QString &img)
{
    imgPath = img;
    updateImage();
}

void PreviewWidget::setBackground(const QColor &color)
{
    bkgColor = color;
}

void PreviewWidget::setBoder(const QColor &color)
{
    bdColor = color;
}

QSize PreviewWidget::imageSize() const
{
    const int border = PREVIEW_ICON_MARGIN * 2;
    QSize orgSize = (size() - QSize(border, border)) * devicePixelRatioF();
    return orgSize;
}

void PreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (curSize != imageSize())
        updateImage();
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    const int margin = PREVIEW_ICON_MARGIN;
    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);

    // draw border
    QRect bkgRect(QPoint(0, 0), size());
    {
        pa.setBrush(bdColor);
        pa.setPen(Qt::NoPen);
        pa.drawRoundedRect(bkgRect, PREVIEW_BORDER_ROUND, PREVIEW_BORDER_ROUND);
    }

    bkgRect = bkgRect.marginsRemoved(QMargins(margin, margin, margin, margin));
    if (pixmap.isNull()) {
        // draw background
        pa.setBrush(bkgColor);
        pa.drawRoundedRect(bkgRect, PREVIEW_ICON_ROUND, PREVIEW_ICON_ROUND);
    } else {
        QPainterPath roundedPath;
        roundedPath.addRoundedRect(bkgRect, PREVIEW_ICON_ROUND, PREVIEW_ICON_ROUND);
        pa.setClipPath(roundedPath);
        pa.drawPixmap(bkgRect, pixmap);
    }
}

void PreviewWidget::setPixmap(QPixmap pix)
{
    pixmap = pix;
    update();
}
