// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QPluginLoader>
#include <QQmlComponent>
#include <QScopedPointer>
#include <QUrl>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <QQmlIncubator>

#include <DPathBuf>

#include <private/qquickitem_p.h>
#include <private/qquicktransition_p.h>

#include "dapploader.h"
#include "dquickwindow.h"
#include "private/dquickapploaderitem_p.h"
#include "private/dqmlglobalobject_p.h"
#include "dqmlapppreloadinterface.h"
#include "dqmlappmainwindowinterface.h"
#include "private/dapploader_p.h"

DQUICK_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

#define APPLICATIONWINDOW_CONTENTDATA "contentData"
#define DIALOGWINDOW_CONTENTDATA "content"
#define WINDOW_CONTENTDATA "data"

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(appLoaderLog, "dtk.quick.apploader" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(appLoaderLog, "dtk.quick.apploader");
#endif

static const QQuickItemPrivate::ChangeTypes changedTypes = QQuickItemPrivate::Geometry;
DAppLoader *DAppLoader::self = nullptr;

static inline const bool heightValid(QQuickItemPrivate *item)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return item->heightValid;
#else
    return item->heightValid();
#endif
}
static inline bool widthValid(QQuickItemPrivate *item)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return item->widthValid;
#else
    return item->widthValid();
#endif
}

class DQmlComponentIncubator : public QQmlIncubator
{
public:
    DQmlComponentIncubator(QQmlComponent *component, DAppLoaderPrivate *d, IncubationMode mode = AsynchronousIfNested);

protected:
    virtual void statusChanged(Status) override;
    virtual void setInitialState(QObject *) override;

private:
    DAppLoaderPrivate *appLoader;
    QQmlComponent *compnent;
};

DQmlComponentIncubator::DQmlComponentIncubator(QQmlComponent *component, DAppLoaderPrivate *appLoader, QQmlIncubator::IncubationMode mode)
    : QQmlIncubator(mode)
    , appLoader(appLoader)
    , compnent(component)
{

}

void DQmlComponentIncubator::statusChanged(QQmlIncubator::Status status)
{
    if (status != QQmlIncubator::Ready) {
        if (status == QQmlIncubator::Error)
            qCWarning(appLoaderLog) << "DAppLoader: " << this->errors();
        return;
    }

    QObject *obj = object();
    QQuickItem *item = qmlobject_cast<QQuickItem *>(obj);
    if (item)
        item->setParentItem(appLoader->appRootItem);
    appLoader->_q_onComponentProgressChanged();
    appLoader->destoryIncubator(this);
}

void DQmlComponentIncubator::setInitialState(QObject *o)
{
    if (!appLoader->appRootItem)
        return;

    if (o) {
        QQmlContext *context = appLoader->itemContextMap.value(compnent);
        if (context)
            QQml_setParent_noEvent(context, o);
        QQml_setParent_noEvent(o, appLoader->appRootItem);
    }
}

DWindowOverlayTransitionManager::DWindowOverlayTransitionManager(DAppLoaderPrivate *loader)
    : appLoader(loader)
{

}

bool DWindowOverlayTransitionManager::transitionLoaded()
{
    if (!appLoader->loadingOverlay || !appLoader->exitedTransition)
        return false;
    transition(QList<QQuickStateAction>(), appLoader->exitedTransition, appLoader->loadingOverlay);
    return true;
}

void DWindowOverlayTransitionManager::finished()
{
    appLoader->clearOverlay();
    appLoader->appRootItem->setFocus(true);
    appLoader->exitedTransition->deleteLater();
    appLoader->exitedTransition = nullptr;
    appLoader->duringTransition = false;
}

DAppLoaderPrivate::DAppLoaderPrivate(DAppLoader *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
    , engine (nullptr)
    , appRootItem(nullptr)
    , loadingOverlay(nullptr)
    , windowContentItem(nullptr)
    , exitedTransition(nullptr)
    , appWindow(nullptr)
    , mainComponent(nullptr)
    , overlayUsingParentSize(false)
    , duringTransition(false)
    , transitionManager(this)
{
    DAppLoader::self = qq;
}

void DAppLoaderPrivate::ensureLoadPreload()
{
    if (preloadInstance)
        return;
    preloadInstance.reset(loadInstance<DQmlAppPreloadInterface>());
    if (preloadInstance.isNull())
        qFatal("Preload plugin for \"%s\" is not found!", qPrintable(appid));
}

void DAppLoaderPrivate::ensureLoadMain()
{
    if (mainInstance)
        return;
    mainInstance.reset(loadInstance<DQmlAppMainWindowInterface>());
    if (mainInstance.isNull())
        qFatal("Main plugin for \"%s\" is not found!", qPrintable(appid));
}

template<class T>
T *DAppLoaderPrivate::loadInstance() const
{
    for (auto iter = pluginPaths.rbegin(); iter != pluginPaths.rend(); iter++) {
        const QDir dir(*iter);
        if (!dir.exists())
            continue;

        for (auto entry : dir.entryInfoList(QDir::Files | QDir::NoSymLinks)) {
            const auto path = entry.absoluteFilePath();
            if (!QLibrary::isLibrary(path))
                continue;

            QPluginLoader loader(path);
            const auto &metaData = loader.metaData();

            const QString iid = metaData["IID"].toString();
            if (iid != qobject_interface_iid<T *>())
                continue;

            if (appid != metaData["MetaData"]["appid"].toString())
                continue;

            if (auto instance = qobject_cast<T *>(loader.instance())) {
                qInfo("Load for %s 's plugin path: %s", qPrintable(appid), qPrintable(path));
                return instance;
            }
        }
    }

    qWarning() << "Can't load request plugin from those directories:" << pluginPaths;

    return nullptr;
}

void DAppLoaderPrivate::destoryIncubator(QQmlIncubator *incubator)
{
    incubators.removeAll(incubator);
    if (incubators.isEmpty()) {
        finishedLoaded();
        appRootItem->setLoaded(true);
    }
    incubator->clear();
    delete incubator;
}

QQmlContext *DAppLoaderPrivate::creationContext(QQmlComponent *component, QObject *obj)
{
    QQmlContext *creationContext = component->creationContext();
    if (!creationContext)
        creationContext = qmlContext(obj);
    QQmlContext *context = new QQmlContext(creationContext);
    context->setContextObject(obj);
    this->itemContextMap.insert(component, context);
    return context;
}

bool DAppLoaderPrivate::createObjects(const char *propertyName)
{
    Q_ASSERT(mainComponent);
    QQmlListReference pros(appWindow, propertyName);
    if (!pros.isValid())
        return false;

    appRootItem = qobject_cast<DQuickAppLoaderItem *>(mainComponent->beginCreate(creationContext(mainComponent, appWindow)));
    Q_ASSERT_X(appRootItem, "DAppLoader", "Must use the AppLoader item in main component.");
    appRootItem->setWindow(appWindow);
    mainComponent->completeCreate();
    DQuickWindowAttached *attached = qobject_cast<DQuickWindowAttached*>(qmlAttachedPropertiesObject<DQuickWindow>(appWindow, false));
    if (attached) {
        attached->setAppLoader(appRootItem);
    }
    pros.append(appRootItem);
    initRootItem(windowContentItem);
    createChildComponents();
    return true;
}

void DAppLoaderPrivate::createChildComponents()
{
    auto components = appRootItem->findChildren<QQmlComponent *>(QStringLiteral(""), Qt::FindDirectChildrenOnly);
    for (auto childCom : qAsConst(components)) {
        QObject::connect(childCom, SIGNAL(progressChanged(qreal)), q_func(), SLOT(_q_onComponentProgressChanged()));
        auto asyn = appRootItem->asynchronous() ? DQmlComponentIncubator::Asynchronous : DQmlComponentIncubator::AsynchronousIfNested;
        DQmlComponentIncubator *incubator = new DQmlComponentIncubator(childCom, this, asyn);
        this->incubators.append(incubator);
        childCom->create(*incubator, creationContext(childCom, appRootItem));
    }
}

void DAppLoaderPrivate::initRootItem(QQuickItem *item)
{
    QQuickItemPrivate *wp = QQuickItemPrivate::get(item);
    wp->addItemChangeListener(this, changedTypes);
    // The appRootItem always fills the window.
    updateRootItemSize(item);
}

void DAppLoaderPrivate::updateRootItemSize(QQuickItem *item)
{
    if (!appRootItem)
        return;

    QQuickItemPrivate *ip = QQuickItemPrivate::get(item);
    if (widthValid(ip)) {
        appRootItem->setWidth(item->width());
    } else {
        appRootItem->setWidth(appWindow->width());
    }

    if (heightValid(ip)) {
        appRootItem->setHeight(item->height());
    } else {
        appRootItem->setWidth(appWindow->height());
    }
}

void DAppLoaderPrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &oldGeometry)
{
    if (loadingOverlay && !duringTransition) {
        if (overlayUsingParentSize)
            loadingOverlay->setSize(item->size());
    }
    updateRootItemSize(item);
    QQuickItemChangeListener::itemGeometryChanged(item, change, oldGeometry);
}

void DAppLoaderPrivate::finishedLoaded()
{
    D_Q(DAppLoader);
    Q_EMIT q->loadFinished();
    duringTransition = transitionManager.transitionLoaded();
    if (!duringTransition) {
        clearOverlay();
        appRootItem->setFocus(true);
    }
    mainInstance->finishedLoading(engine);
}

void DAppLoaderPrivate::createOverlay()
{
    if (loadingOverlay)
        return;
    DQuickWindowAttached *attached = qobject_cast<DQuickWindowAttached*>(qmlAttachedPropertiesObject<DQuickWindow>(appWindow, false));
    if (!attached)
        return;
    QQmlComponent *loComponent = attached->loadingOverlay();
    if (!loComponent)
        return;
    QQmlContext *context = creationContext(loComponent, windowContentItem);
    loadingOverlay = qobject_cast<QQuickItem *>(loComponent->beginCreate(context));
    if (!loadingOverlay)
        return;
    loadingOverlay->setParentItem(windowContentItem);
    loadingOverlay->setZ(DQMLGlobalObject::TopOrder);
    exitedTransition = attached->overlayExited();
    if (exitedTransition)
        engine->setObjectOwnership(exitedTransition, QQmlEngine::CppOwnership);
    loComponent->completeCreate();
    QQuickItemPrivate *overlayPrivate = QQuickItemPrivate::get(loadingOverlay);
    if (!widthValid(overlayPrivate) || !heightValid(overlayPrivate)) {
        overlayUsingParentSize = true;
        if (qFuzzyCompare(windowContentItem->width(), 0) || qFuzzyCompare(windowContentItem->height(), 0)) {
            loadingOverlay->setSize(appWindow->size());
        } else {
            loadingOverlay->setSize(windowContentItem->size());
        }
    }
    engine->setObjectOwnership(loadingOverlay, QQmlEngine::CppOwnership);
    return;
}

void DAppLoaderPrivate::clearOverlay()
{
    if (!loadingOverlay)
        return;
    loadingOverlay->setParentItem(nullptr);
    loadingOverlay->setVisible(false);
    loadingOverlay->deleteLater();
    loadingOverlay = nullptr;
    DQuickWindowAttached *attached = qobject_cast<DQuickWindowAttached*>(qmlAttachedPropertiesObject<DQuickWindow>(appWindow, false));
    if (attached) {
        QQmlContext *context = itemContextMap.take(attached->loadingOverlay());
        delete context;
    }
}

void DAppLoaderPrivate::findWindowContentItem()
{
    if (!appWindow)
        return;

    do {
        QVariant dataVariant = appWindow->property(APPLICATIONWINDOW_CONTENTDATA);
        if (dataVariant.isValid()) {
            // ApplicationWindow
            auto objectsData = dataVariant.value<QQmlListProperty<QObject>>();
            windowContentItem = qmlobject_cast<QQuickItem *>(objectsData.object);
            break;
        }

        dataVariant = appWindow->property(DIALOGWINDOW_CONTENTDATA);
        if (dataVariant.isValid()) {
            // DialogWindow
            auto itemsData = dataVariant.value<QQmlListProperty<QQuickItem>>();
            windowContentItem = qmlobject_cast<QQuickItem *>(itemsData.object);
            break;
        }
    } while (false);

    // Window
    if (!windowContentItem)
        windowContentItem = appWindow->contentItem();
}

void DAppLoaderPrivate::_q_onPreloadCreated(QObject *obj, const QUrl &)
{
    QQuickWindow *window = qmlobject_cast<QQuickWindow *>(obj);
    if (!window)
        return;

    appWindow = window;
    findWindowContentItem();
    Q_ASSERT(appWindow);
    Q_ASSERT(windowContentItem);
    // ensure the overlay shown and start the loading animation.
    createOverlay();

    ensureLoadMain();
    mainInstance->initialize(engine);
    // Insert component into preload
    const QUrl &mainComponentPath = mainInstance->mainComponentPath();
    mainComponent = new QQmlComponent(engine, mainComponentPath, QQmlComponent::Asynchronous);
    if (mainComponent->isLoading()) {
        QObject::connect(mainComponent, SIGNAL(statusChanged(QQmlComponent::Status)), q_func(), SLOT(_q_onMainComponentStatusChanged(QQmlComponent::Status)));
    } else {
        _q_onMainComponentStatusChanged(mainComponent->status());
    }
}


void DAppLoaderPrivate::_q_onMainComponentStatusChanged(QQmlComponent::Status status)
{
    if (status != QQmlComponent::Ready) {
        if (status == QQmlComponent::Error)
            qCWarning(appLoaderLog) << "DAppLoader: " << mainComponent->errors() << " " << mainComponent->errorString();
        return;
    }

    do {
        // For ApplicationWindow.
        if (createObjects(APPLICATIONWINDOW_CONTENTDATA))
            break;

        // For DialogWindow.
        if (createObjects(DIALOGWINDOW_CONTENTDATA))
            break;

        // For Window.
        if (createObjects(WINDOW_CONTENTDATA))
            break;
    } while (false);

    mainComponent->deleteLater();
    mainComponent = nullptr;
}

void DAppLoaderPrivate::_q_onComponentProgressChanged()
{
    qreal progress = 0;
    auto components = appRootItem->findChildren<QQmlComponent *>();
    for (auto childCom : qAsConst(components)) {
        progress += childCom->progress();
    }

    appRootItem->setProgress(progress / components.count());
}

QStringList DAppLoaderPrivate::buildinPluginPaths()
{
    QStringList result;
    // 'DTK_QML_PLUGIN_PATH' directory.
    const auto dtkPluginPath = qgetenv("DTK_QML_PLUGIN_PATH");
    if (!dtkPluginPath.isEmpty())
        result.append(dtkPluginPath);

#ifdef DTK_QML_APP_PLUGIN_SUBPATH
    const auto ldPath = qgetenv("LD_LIBRARY_PATH");
    if (!ldPath.isEmpty()) {
        // fallback to $LD_LIBRARY_PATH/'DTK_QML_APP_PLUGIN_SUBPATH'.
        for (const auto &i : ldPath.split(':')) {
            const DPathBuf dir(i);
            result.append((dir / DTK_QML_APP_PLUGIN_SUBPATH).toString());
        }
    }
#endif

#ifdef DTK_QML_APP_PLUGIN_PATH
    // dtkdeclarative runtime directory.
    result.append(DTK_QML_APP_PLUGIN_PATH);
#endif

    return result;
}

/*!
 * \~chinese \brief DAppLoader::DAppLoader　用于加载DTk QML应用插件
 * \~chinese \param appid　　应用插件的名字
 * \~chinese \param appPath　　应用插件的安装目录
 */
DAppLoader::DAppLoader(const QString &appid, const QString &appPath, QObject *parent)
    : QObject (parent)
    , DTK_CORE_NAMESPACE::DObject(*new DAppLoaderPrivate(this))
{
    D_D(DAppLoader);

    d->appid = appid;

    const auto &paths = d->buildinPluginPaths();
    for (auto iter = paths.rbegin(); iter != paths.rend(); iter++)
        addPluginPath(*iter);
    addPluginPath(appPath);
}

DAppLoader::~DAppLoader()
{
    D_D(DAppLoader);
    qDeleteAll(d->itemContextMap.values());
    delete d->engine;
    self = nullptr;
}

void DAppLoader::addPluginPath(const QString &dir)
{
    D_D(DAppLoader);
    if (dir.isEmpty())
        return;
    d->pluginPaths.append(dir);
}

/*!
 * \~chinese \brief DAppLoader::pluginPaths high priority at the front.
 */
QStringList DAppLoader::pluginPaths() const
{
    D_DC(DAppLoader);
    QStringList paths;
    std::reverse_copy(d->pluginPaths.begin(), d->pluginPaths.end(), std::back_inserter(paths));
    return paths;
}

// it will enter the eventloop directly.
int DAppLoader::exec(int &argc, char **argv)
{
    D_D(DAppLoader);

    d->ensureLoadPreload();
    d->app.reset(d->preloadInstance->creatApplication(argc, argv));

    if (!d->app)
        return -1;
    if (!d->engine)
        d->engine = new QQmlApplicationEngine(this);
    auto graphics = d->preloadInstance->graphicsApi();
    if (graphics == QSGRendererInterface::Unknown) {
#if defined __aarch64__
        const char *renderName = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
#else
        const char *renderName = reinterpret_cast<const char *>(glGetString(GL_RENDER));
#endif
        if (renderName) {
            if (renderName != QByteArrayLiteral("LLVMPIPE")
                    || renderName != QByteArrayLiteral("SWRAST"))
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#else
                QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#endif
        }
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QQuickWindow::setSceneGraphBackend(graphics);
#else
        QQuickWindow::setGraphicsApi(graphics);
#endif
    }
    d->preloadInstance->aboutToPreload(d->engine);
    QObject::connect(d->engine, SIGNAL(objectCreated(QObject *, const QUrl &)), this, SLOT(_q_onPreloadCreated(QObject *, const QUrl &)));
    d->engine->load(d->preloadInstance->preloadComponentPath());
    if (d->engine->rootObjects().isEmpty())
        return -1;
    return d->app->exec();
}

DQUICK_END_NAMESPACE
#include "moc_dapploader.cpp"
