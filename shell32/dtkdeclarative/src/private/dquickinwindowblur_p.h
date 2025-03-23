// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKINWINDOWBLUR_P_H
#define DQUICKINWINDOWBLUR_P_H

#include <dtkdeclarative_global.h>

#include <QQuickItem>

QT_BEGIN_NAMESPACE
class QSGPlainTexture;
#ifndef QT_NO_OPENGL
class QOpenGLFramebufferObject;
#endif
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DQuickInWindowBlendBlurPrivate;
class InWindowBlurTextureProvider;
class DSGBlurNode;
class DQuickInWindowBlur : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(bool offscreen READ offscreen WRITE setOffscreen NOTIFY offscreenChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(InWindowBlurImpl)
#endif

public:
    explicit DQuickInWindowBlur(QQuickItem *parent = nullptr);
    ~DQuickInWindowBlur() override;

    qreal radius() const;
    void setRadius(qreal newRadius);

    bool offscreen() const;
    void setOffscreen(bool newOffscreen);

    bool isTextureProvider() const override { return true; }
    QSGTextureProvider *textureProvider() const override;

Q_SIGNALS:
    void radiusChanged();
    void offscreenChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
    void releaseResources() override;

private Q_SLOTS:
    void invalidateSceneGraph();

private:
    qreal m_radius = 20;
    bool m_offscreen = false;
    mutable InWindowBlurTextureProvider *m_tp = nullptr;
    friend void onRender(DSGBlurNode *, void *);
};

DQUICK_END_NAMESPACE

#endif // DQUICKINWINDOWBLUR_P_H
