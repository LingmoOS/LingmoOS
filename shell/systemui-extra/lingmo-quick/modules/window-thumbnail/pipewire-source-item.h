/*
    Render a PipeWire stream into a QtQuick scene as a standard Item
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileContributor: iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PIPEWIRESOURCEITEM_H
#define PIPEWIRESOURCEITEM_H

#include <QQuickItem>
#include <functional>

#include <pipewire/pipewire.h>
#include <spa/param/format-utils.h>
#include <spa/param/props.h>
#include <spa/param/video/format-utils.h>

struct DmaBufPlane;
class PipeWireSourceStream;
class QSGTexture;
class QOpenGLTexture;
typedef void *EGLImage;

class PipeWireSourceItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(uint nodeId READ nodeId WRITE setNodeId NOTIFY nodeIdChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    Q_PROPERTY(bool fixHeight READ fixHeight WRITE setFixHeight NOTIFY fixHeightChanged)
    Q_PROPERTY(bool fixWidth READ fixWidth WRITE setFixWidth NOTIFY fixWidthChanged)

public:
    PipeWireSourceItem(QQuickItem *parent = nullptr);
    ~PipeWireSourceItem() override;

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    Q_SCRIPTABLE QString error() const;

    void setNodeId(uint nodeId);
    uint nodeId() const
    {
        return m_nodeId;
    }

    void componentComplete() override;
    void releaseResources() override;
    bool fixHeight() const;
    void setFixHeight(bool fixHeight);

    bool fixWidth() const;
    void setFixWidth(bool fixWidth);

    qreal paintedWidth() const;
    qreal paintedHeight() const;

Q_SIGNALS:
    void nodeIdChanged(uint nodeId);
    void fixHeightChanged();
    void fixWidthChanged();
    void paintedSizeChanged();

private:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void updateTextureDmaBuf(const QVector<DmaBufPlane> &plane, uint32_t format);
    void updateTextureImage(const QImage &image);

    uint m_nodeId = 0;
    std::function<QSGTexture *()> m_createNextTexture;
    QScopedPointer<PipeWireSourceStream> m_stream;
    QScopedPointer<QOpenGLTexture> m_texture;

    EGLImage m_image = nullptr;
    bool m_needsRecreateTexture = false;

    bool m_fixHeight = false;
    bool m_fixWidth = false;
    QSizeF m_paintedSize;
};

#endif // PIPEWIRESOURCEITEM_H
