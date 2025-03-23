// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLITFRAMEBUFFERNODE_H
#define DBLITFRAMEBUFFERNODE_H

#include <dtkdeclarative_global.h>

#include <QSGTextureProvider>
#include <QSGRenderNode>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QSGPlainTexture;
class QQuickItem;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class ProxyAtlasTexture;
class DBlitFramebufferNode : public QSGRenderNode {
public:
    inline QSizeF size() const {
        return m_size;
    }
    QSGTexture *texture() const;

#ifndef QT_NO_OPENGL
    static DBlitFramebufferNode *createOpenGLNode(QQuickItem *item,
                                                  bool shareBuffer = false,
                                                  bool useAtlasTexture = false);
#endif
    static DBlitFramebufferNode *createSoftwareNode(QQuickItem *item,
                                                    bool shareBuffer = false,
                                                    bool useAtlasTexture = false);

    QRectF rect() const override;

    void resize(const QSizeF &size);
    void setMargins(const QMarginsF &margin);

    typedef void(*RenderCallback)(DBlitFramebufferNode *node, void *data);
    void setRenderCallback(RenderCallback callback, void *data);
    inline void doRenderCallback() {
        if (!m_renderCallback)
            return;
        m_renderCallback(this, m_callbackData);
    }
    virtual QImage toImage() const { return QImage(); }

protected:
    DBlitFramebufferNode(QQuickItem *item);

    QPointer<QQuickItem> m_item;
    QSizeF m_size;
    QMarginsF m_margins;
    QRectF m_rect;
    QScopedPointer<ProxyAtlasTexture> m_texture;
    RenderCallback m_renderCallback = nullptr;
    void *m_callbackData = nullptr;
    bool shareBuffer = false;
    bool useAtlasTexture = false;
};

DQUICK_END_NAMESPACE

#endif // DBLITFRAMEBUFFERNODE_H
