// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKAPPLOADERITEM_H
#define DQUICKAPPLOADERITEM_H

#include <dtkdeclarative_global.h>

#include <DObject>
#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE
class DQuickAppLoaderItemPrivate;
class DQuickAppLoaderItem : public QQuickItem, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickAppLoaderItem)
    Q_PROPERTY(QQuickWindow *window READ window NOTIFY windowChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(AppLoader)
#endif

public:
    explicit DQuickAppLoaderItem(QQuickItem *parentItem = nullptr);

    bool loaded() const;
    void setLoaded(bool loaded);

    qreal progress() const;
    void setProgress(qreal progress);

    QQuickWindow *window() const;
    void setWindow(QQuickWindow *w);

    bool asynchronous() const;
    void setAsynchronous(bool a);
Q_SIGNALS:
    void loadedChanged();
    void progressChanged();
    void windowChanged();
    void asynchronousChanged();
};

DQUICK_END_NAMESPACE
#endif  // DQUICKAPPLOADERITEM_H
