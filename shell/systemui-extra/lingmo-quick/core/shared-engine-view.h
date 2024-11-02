/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_SHARED_ENGINE_VIEW_H
#define LINGMO_QUICK_ITEMS_SHARED_ENGINE_VIEW_H

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>

namespace LingmoUIQuick {

class SharedEngineViewPrivate;

class SharedEngineView : public QQuickWindow
{
    Q_OBJECT
    friend class SharedEngineViewPrivate;
public:
    enum ResizeMode { SizeViewToRootObject, SizeRootObjectToView };

    static QQmlEngine *sharedEngine();

    explicit SharedEngineView(QWindow *parent = nullptr);
    QQmlEngine *engine() const;
    QQmlContext *rootContext() const;
    QQuickItem *rootObject() const;
    void setResizeMode(SharedEngineView::ResizeMode resizeMode);
    SharedEngineView::ResizeMode resizeMode() const;

public Q_SLOTS:
    void setSource(const QUrl &source) const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void continueCreate() const;
    void updateViewSize() const;

private:
    SharedEngineViewPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_SHARED_ENGINE_VIEW_H
