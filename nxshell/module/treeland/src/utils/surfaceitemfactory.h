// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <QQuickItem>

#include <wsurfaceitem.h>

using Waylib::Server::WSurfaceItem;
class SurfaceItemFactory: public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(SurfaceItemFactory)
    Q_PROPERTY(WSurfaceItem* surfaceItem MEMBER m_surfaceItem CONSTANT)
    WSurfaceItem *m_surfaceItem;
public:
    explicit SurfaceItemFactory(QQuickItem *parent = nullptr);
    void classBegin() override;
};
