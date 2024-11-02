/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMO_TASK_MANAGER_WINDOW_THUMBNAIL_H
#define LINGMO_TASK_MANAGER_WINDOW_THUMBNAIL_H

#include <QAbstractNativeEventFilter>
#include <QPointer>
#include <QQuickItem>
#include <QSGSimpleTextureNode>
#include <QSGTextureProvider>
#include <QWindow>
#include <QSizeF>
#include <QX11Info>
#include <xcb/damage.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

class WindowTextureProvider;
class XWindowThumbnail : public QQuickItem, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_PROPERTY(uint winId READ winId WRITE setWinId NOTIFY winIdChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
    Q_PROPERTY(bool thumbnailAvailable READ thumbnailAvailable NOTIFY thumbnailAvailableChanged)
    Q_PROPERTY(bool fixHeight READ fixHeight WRITE setFixHeight NOTIFY fixHeightChanged)
    Q_PROPERTY(bool fixWidth READ fixWidth WRITE setFixWidth NOTIFY fixWidthChanged)

public:
    explicit XWindowThumbnail(QQuickItem *parent = nullptr);
    ~XWindowThumbnail() override;
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;

    qreal paintedWidth() const;
    qreal paintedHeight() const;
    bool thumbnailAvailable() const;

    uint32_t winId() const;
    void setWinId(uint32_t winId);

    bool fixHeight() const;
    void setFixHeight(bool fixHeight);

    bool fixWidth() const;
    void setFixWidth(bool fixWidth);

Q_SIGNALS:
    void winIdChanged();
    void fixHeightChanged();
    void fixWidthChanged();
    void paintedSizeChanged();
    void thumbnailAvailableChanged();

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void releaseResources() override;

private:
    void iconToTexture(WindowTextureProvider *textureProvider);
    void windowToTexture(WindowTextureProvider *textureProvider);
    bool windowToTextureGLX(WindowTextureProvider *textureProvider);
    bool xcbWindowToTextureEGL(WindowTextureProvider *textureProvider);
    void bindEGLTexture();
    void resolveEGLFunctions();
    void resolveGLXFunctions();
    bool loadGLXTexture();
    void bindGLXTexture();
    xcb_pixmap_t pixmapForWindow();
    void setThumbnailAvailable(bool thumbnailAvailable);
    bool startRedirecting();
    void stopRedirecting();
    void resetDamaged();
    void sceneVisibilityChanged(bool visible);

    QPointer<QWindow> m_scene;
    uint32_t m_winId = 0;
    bool m_fixHeight = false;
    bool m_fixWidth = false;
    QSizeF m_paintedSize;
    bool m_thumbnailAvailable = false;
    bool m_xcb = false;
    bool m_composite = false;
    bool m_damaged = false;
    int m_depth = 0;
    bool m_redirecting = false;
    WindowTextureProvider *m_textureProvider = nullptr;
    uint8_t m_damageEventBase = 0;
    xcb_damage_damage_t m_damage;
    xcb_pixmap_t m_pixmap;
    bool m_openGLFunctionsResolved = false;
    bool m_eglFunctionsResolved = false;
    QFunctionPointer m_bindTexImage = nullptr;
    QFunctionPointer m_releaseTexImage = nullptr;
    xcb_pixmap_t m_glxPixmap;
    xcb_visualid_t m_visualid = 0;
    uint m_texture = 0;
    EGLImageKHR m_image;
    QFunctionPointer m_eglCreateImageKHR{};
    QFunctionPointer m_eglDestroyImageKHR{};
    QFunctionPointer m_glEGLImageTargetTexture2DOES{};
};
class WindowTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    QSGTexture *texture() const override;
    void setTexture(QSGTexture *texture);

private:
    std::unique_ptr<QSGTexture> m_texture;
};
#endif //LINGMO_TASK_MANAGER_WINDOW_THUMBNAIL_H
