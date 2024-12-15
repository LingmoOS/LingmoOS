// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkdeclarative_global.h>
#include <DObject>
#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickBackdropBlitterPrivate;
class Q_DECL_EXPORT DQuickBackdropBlitter : public QQuickItem, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickBackdropBlitter)
    Q_PRIVATE_PROPERTY(DQuickBackdropBlitter::d_func(), QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_PROPERTY(QQuickItem* content READ content CONSTANT)
    Q_PROPERTY(bool offscreen READ offscreen WRITE setOffscreen NOTIFY offscreenChanged FINAL)
    Q_PROPERTY(bool blitterEnabled READ blitterEnabled WRITE setBlitterEnabled NOTIFY blitterEnabledChanged FINAL)
    QML_NAMED_ELEMENT(BackdropBlitter)

public:
    explicit DQuickBackdropBlitter(QQuickItem *parent = nullptr);
    ~DQuickBackdropBlitter();

    QQuickItem *content() const;

    bool offscreen() const;
    void setOffscreen(bool newOffscreen);

    bool blitterEnabled() const;
    void setBlitterEnabled(bool newBlitterEnabled);

Q_SIGNALS:
    void offscreenChanged();
    void blitterEnabledChanged();

private Q_SLOTS:
    void invalidateSceneGraph();

private:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void releaseResources() override;
};

DQUICK_END_NAMESPACE
