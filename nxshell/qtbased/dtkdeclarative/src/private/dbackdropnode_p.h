// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkdeclarative_global.h>
#include <QSGRenderNode>
#include <QPointer>
#include <QImage>

QT_BEGIN_NAMESPACE
class QQuickItem;
class QSGTexture;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DBackdropNode : public QSGRenderNode {
public:
    inline QSizeF size() const {
        return m_size;
    }
    QSGTexture *texture() const;

    static DBackdropNode *createRhiNode(QQuickItem *item);
    static DBackdropNode *createSoftwareNode(QQuickItem *item);

    QRectF rect() const override;
    RenderingFlags flags() const override;

    void resize(const QSizeF &size);
    void setContentItem(QQuickItem *item);

    typedef void(*TextureChangedNotifer)(DBackdropNode *node, void *data);
    void setTextureChangedCallback(TextureChangedNotifer callback, void *data);
    inline void doNotifyTextureChanged() {
        if (!m_renderCallback)
            return;
        m_renderCallback(this, m_callbackData);
    }
    virtual QImage toImage() const { return QImage(); }

    QQuickWindow *renderWindow() const;
    qreal effectiveDevicePixelRatio() const;

protected:
    DBackdropNode(QQuickItem *item, QSGTexture *texture);

    QPointer<QQuickItem> m_item;
    QPointer<QQuickItem> m_content;
    QSizeF m_size;
    QRectF m_rect;
    QScopedPointer<QSGTexture> m_texture;
    TextureChangedNotifer m_renderCallback = nullptr;
    void *m_callbackData = nullptr;
};

DQUICK_END_NAMESPACE
