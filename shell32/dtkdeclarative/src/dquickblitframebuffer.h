// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKBLITFRAMEBUFFER_H
#define DQUICKBLITFRAMEBUFFER_H

#include <QQuickItem>
#include <dtkdeclarative_global.h>
#include <DObject>

DQUICK_BEGIN_NAMESPACE

class DQuickBlitFramebufferPrivate;
class Q_DECL_EXPORT DQuickBlitFramebuffer : public QQuickItem, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickBlitFramebuffer)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(BlitFramebuffer)
#endif
public:
    explicit DQuickBlitFramebuffer(QQuickItem *parent = nullptr);
    ~DQuickBlitFramebuffer();

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;

private Q_SLOTS:
    void invalidateSceneGraph();

private:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
    void releaseResources() override;
};

DQUICK_END_NAMESPACE

#endif // DQUICKBLITFRAMEBUFFER_H
