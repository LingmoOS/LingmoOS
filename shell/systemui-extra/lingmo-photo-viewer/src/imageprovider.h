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
#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>
#include <QPixmapCache>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDebug>
#include <QIcon>
#include <QDate>
#include "./view/showimagewidget.h"
#include "./view/sizedate.h"
#include "./global/variable.h"
class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    //这个是自动调用的。路径"image://screen"，如需要使用id等参数，如"image://screen/id"等
    {
        //如果图片已经缓存过，则直接从map里取
        if (Variable::imageHasLoaded(id) /*&& Variable::g_currentImageType != "multiTiff"*/) {
            m_image = Variable::getLoadedImage(id);
            return m_image;
        }
        if (Variable::g_tempImage.isNull()) {
            m_image.load("qrc:/res/res/damaged_img.png");
        } else {
            Variable::setLoadedImageMap(id, Variable::g_tempImage);
            m_image = Variable::g_tempImage;
        }
        return m_image;
    }
    QImage m_image;
};

#endif // PQASYNCIMAGEPROVIDERTHUMB_H
