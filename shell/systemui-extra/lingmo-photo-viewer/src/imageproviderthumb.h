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
#ifndef PQASYNCIMAGEPROVIDERTHUMB_H
#define PQASYNCIMAGEPROVIDERTHUMB_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>
#include <QMutex>
#include <QPixmapCache>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDebug>
#include <QIcon>
#include <QDate>
#include "./view/showimagewidget.h"
#include "./view/sizedate.h"
#include "global/variable.h"
#include "model/processing/processing.h"
#include "model/file/file.h"
class PQAsyncImageProviderThumb : public QQuickAsyncImageProvider
{

public:
    PQAsyncImageProviderThumb(QMutex *mutex);
    ~PQAsyncImageProviderThumb();
    QQuickImageResponse *requestImageResponse(const QString &url, const QSize &requestedSize) override;

private:
    QMutex *m_mutex;
};

class ScreenImageProvider : public QQuickImageResponse, public QRunnable
{
public:
    ScreenImageProvider(const QString &url, const QSize &requestedSize, QMutex *mutex);
    ~ScreenImageProvider();

    QQuickTextureFactory *textureFactory() const override;

    void run() override;

    QString m_url;
    QSize m_requestedSize;
    QImage m_image;
    QMutex *m_mutex;
    bool isRunning = false;
};

#endif // PQASYNCIMAGEPROVIDERTHUMB_H
