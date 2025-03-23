// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPPLOADER_P_H
#define DAPPLOADER_P_H

#include <QString>
#include <DObjectPrivate>
#include <QQmlIncubationController>

#include "dapploader.h"
#include <private/qsgrenderloop_p.h>
#include <private/qquickitemchangelistener_p.h>
#include <private/qquicktransitionmanager_p_p.h>

QT_BEGIN_NAMESPACE
class QQmlApplicationEngine;
class QQuickItem;
class QQuickTransition;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DAppLoaderPrivate;
class DWindowOverlayTransitionManager : public QQuickTransitionManager
{
public:
    DWindowOverlayTransitionManager(DAppLoaderPrivate *loader);
    bool transitionLoaded();
protected:
    virtual void finished() override;
private:
    DAppLoaderPrivate *appLoader;
};

class DQuickAppLoaderItem;
class DQmlAppPreloadInterface;
class DQmlAppMainWindowInterface;
class DAppLoaderPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate, public QQuickItemChangeListener
{
    friend class DQmlComponentIncubator;
public:
    DAppLoaderPrivate(DAppLoader *qq);
    void ensureLoadPreload();
    void ensureLoadMain();
    void destoryIncubator(QQmlIncubator *incubator);
    QQmlContext *creationContext(QQmlComponent *component, QObject *obj);
    bool createObjects(const char *propertyName);
    void createChildComponents();
    void initRootItem(QQuickItem *item);
    void updateRootItemSize(QQuickItem *item);
    virtual void itemGeometryChanged(QQuickItem *, QQuickGeometryChange, const QRectF &) override;
    void findWindowContentItem();
    void finishedLoaded();
    void createOverlay();
    void clearOverlay();
    void _q_onPreloadCreated(QObject *obj, const QUrl &);
    void _q_onMainComponentStatusChanged(QQmlComponent::Status status);
    void _q_onComponentProgressChanged();

    static QStringList buildinPluginPaths();

    QString appid;
    QStringList pluginPaths;
    QQmlApplicationEngine *engine;
    QList<QQmlIncubator *> incubators;
    DQuickAppLoaderItem *appRootItem;
    QQuickItem *loadingOverlay;
    QPointer<QQuickItem> windowContentItem;
    QQuickTransition *exitedTransition;
    QQuickWindow *appWindow;
    QQmlComponent *mainComponent;
    bool overlayUsingParentSize;
    bool duringTransition;
    DWindowOverlayTransitionManager transitionManager;
    QHash<QQmlComponent *, QQmlContext *> itemContextMap;
    QScopedPointer<DQmlAppPreloadInterface> preloadInstance;
    QScopedPointer<DQmlAppMainWindowInterface> mainInstance;
    QScopedPointer<QGuiApplication> app;
private:
    template<class T> T *loadInstance() const;

    D_DECLARE_PUBLIC(DAppLoader)
};

DQUICK_END_NAMESPACE

#endif //DAPPLOADER_P_H
