/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_KYWLCOM_WINDOW_THUMBNAIL_ITEM_H
#define LINGMO_QUICK_KYWLCOM_WINDOW_THUMBNAIL_ITEM_H

#include <QQuickItem>
#include <libkywc.h>
#include <QSocketNotifier>
#include <QImage>
#include <QOpenGLTexture>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/mman.h>

#include "kywlcom-thumbnail.h"
#include "kywlcom-contex.h"


typedef struct _kywc_thumbnail kywc_thumbnail;

class KywlcomWindowThumbnailItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    Q_PROPERTY(bool fixHeight READ fixHeight WRITE setFixHeight NOTIFY fixHeightChanged)
    Q_PROPERTY(bool fixWidth READ fixWidth WRITE setFixWidth NOTIFY fixWidthChanged)
public:
    explicit KywlcomWindowThumbnailItem(QQuickItem *parent = nullptr);
    ~KywlcomWindowThumbnailItem();

    QString uuid();
    void setUuid(QString &uuid);

    qreal paintedWidth() const;
    qreal paintedHeight() const;

    bool fixHeight() const;
    void setFixHeight(bool fixHeight);

    bool fixWidth() const;
    void setFixWidth(bool fixWidth);
    void componentComplete() override;
    void releaseResources() override;

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

Q_SIGNALS:
    void uuidChanged();
    void paintedSizeChanged();
    void fixHeightChanged();
    void fixWidthChanged();

private Q_SLOTS:
    void BufferImportDmabuf();
    void thumbnailIsDeleted();

private:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void createEglImage(Thumbnail *thumbnail);
    void imageFromMemfd(Thumbnail *thumbnail);
    void active(bool active);

    QString m_uuid;
    QString m_currentUuid;
    Context *m_context = nullptr;
    Thumbnail *m_thumbnail = nullptr;
    wl_display *m_display = nullptr;
    EGLImage m_image = EGL_NO_IMAGE_KHR;
    QImage m_qImage;
    uint32_t m_format;
    Thumbnail::BufferFlags m_thumFlags = Thumbnail::BufferFlag::Dmabuf;

    QOpenGLTexture *m_texture = nullptr;
    bool m_needsRecreateTexture = false;
    QSizeF m_paintedSize;
    bool m_fixHeight = false;
    bool m_fixWidth = false;
    bool m_active = false;
    bool m_thumbnailIsDeleted = false;
    uint8_t *map = nullptr;
    size_t dataSize = 0;
};


#endif //LINGMO_QUICK_KYWLCOM_WINDOW_THUMBNAIL_ITEM_H
