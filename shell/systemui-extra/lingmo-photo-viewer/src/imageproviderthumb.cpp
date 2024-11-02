/**************************************************************************
 **                                                                      **
 ** Copyright (C) 2011-2021 Lukas Spies                                  **
 ** Contact: http://photoqt.org                                          **
 **                                                                      **
 ** This file is part of PhotoQt.                                        **
 **                                                                      **
 ** PhotoQt is free software: you can redistribute it and/or modify      **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation, either version 2 of the License, or    **
 ** (at your option) any later version.                                  **
 **                                                                      **
 ** PhotoQt is distributed in the hope that it will be useful,           **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with PhotoQt. If not, see <http://www.gnu.org/licenses/>.      **
 **                                                                      **
 **************************************************************************/

#include "imageproviderthumb.h"
#include <QDebug>
//继承自QQuickImageProvider,与QQuickImageResponse结合起来使用，实现异步加载图片到qml
QQuickImageResponse *PQAsyncImageProviderThumb::requestImageResponse(const QString &url, const QSize &requestedSize)
{
    ScreenImageProvider *response = new ScreenImageProvider(url, requestedSize, m_mutex);
    //开线程池
    QThreadPool::globalInstance()->start(response);

    return response;
}

PQAsyncImageProviderThumb::PQAsyncImageProviderThumb(QMutex *mutex)
    : m_mutex(mutex)
{

}
PQAsyncImageProviderThumb::~PQAsyncImageProviderThumb()
{
    // 可选：在析构时等待线程池中的任务完成
    QThreadPool::globalInstance()->waitForDone();
}


ScreenImageProvider::ScreenImageProvider(const QString &url, const QSize &requestedSize, QMutex *mutex)
    : m_url(url), m_requestedSize(requestedSize), m_mutex(mutex)
{
    setAutoDelete(false);
}

ScreenImageProvider::~ScreenImageProvider() {
    if (isRunning) {
        isRunning = false;
        cancel();
    }

}
QQuickTextureFactory *ScreenImageProvider::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

void ScreenImageProvider::run()
{
    isRunning = true;
    if (THEMEDARK == Variable::g_themeStyle || THEMEBLACK == Variable::g_themeStyle) {
        m_image.load(Variable::DAMAGED_IMAGE_BLACK);
    } else {
        m_image.load(Variable::DAMAGED_IMAGE_WHITE);
    }
    if (m_url.isEmpty()) {
        Q_EMIT finished();
        return;
    }
    if (Variable::g_isClosing == true) {
        Q_EMIT finished();
        return;
    }
    // 如果图片已经缓存过，则直接从map里取
    if (Variable::imageHasLoaded(m_url)) {
        m_image = Variable::getLoadedImage(m_url);
        Q_EMIT finished();
        return;
    }

    m_mutex->lock();
    QString realFormat = File::realFormat(m_url);
    m_mutex->unlock();

    //如果时多页tiff
    if (realFormat == "tiff" || realFormat == "tif") {
        if (Variable::imageHasLoaded(m_url + "0")) {
            m_image = Variable::getLoadedImage(m_url);
            Q_EMIT finished();
            return;
        }
    }
    m_mutex->lock();
    MatAndFileinfo maf = File::loadImage(m_url, realFormat);
    m_mutex->unlock();

    bool openSuccess = maf.openSuccess; //判断是否打开失败-损坏
    if (!openSuccess) {
        Q_EMIT finished();
        return;
    }
    m_mutex->lock();
    QPixmap nowImage = Processing::converFormat(maf.mat);    
    m_mutex->unlock();

    if (nowImage.isNull()) {
        Q_EMIT finished();
        return;
    }
    if (!(realFormat == "tiff" || realFormat == "tif")) {
        Variable::setLoadedImageMap(m_url, nowImage.toImage());
    }
    m_image = nowImage.toImage();
    Q_EMIT finished();
}
