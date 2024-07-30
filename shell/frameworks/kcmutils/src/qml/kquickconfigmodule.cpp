/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickconfigmodule.h"
#include "kabstractconfigmodule.h"
#include "kcmutils_debug.h"
#include "sharedqmlengine_p.h"

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickItem>
#include <QResource>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include <memory>

class KQuickConfigModulePrivate
{
public:
    KQuickConfigModulePrivate(KQuickConfigModule *module)
        : q(module)
    {
    }

    KQuickConfigModule *q;
    SharedQmlEngine *engine = nullptr;
    std::shared_ptr<QQmlEngine> passedInEngine;
    QList<QQuickItem *> subPages;
    int columnWidth = -1;
    int currentIndex = 0;
    QString errorString;

    static QHash<QQmlContext *, KQuickConfigModule *> rootObjects;

    QString getResourcePath(const QString &file)
    {
        return QLatin1String("/kcm/") + q->metaData().pluginId() + QLatin1String("/") + file;
    }
    QUrl getResourceUrl(const QString &resourcePath)
    {
        return QUrl(QLatin1String("qrc:") + resourcePath);
    }
};

QHash<QQmlContext *, KQuickConfigModule *> KQuickConfigModulePrivate::rootObjects = QHash<QQmlContext *, KQuickConfigModule *>();

KQuickConfigModule::KQuickConfigModule(QObject *parent, const KPluginMetaData &metaData)
    : KAbstractConfigModule(parent, metaData)
    , d(new KQuickConfigModulePrivate(this))
{
}

void KQuickConfigModule::setInternalEngine(const std::shared_ptr<QQmlEngine> &engine)
{
    d->passedInEngine = engine;
}

KQuickConfigModule::~KQuickConfigModule()
{
    // in case mainUi was never called
    if (d->engine) {
        // delete the mainUi before removing the root object.
        // Otherwise, we get lots of console errors about trying to read properties of null objects
        delete d->engine->rootObject();
        KQuickConfigModulePrivate::rootObjects.remove(d->engine->rootContext());
    }
}

KQuickConfigModule *KQuickConfigModule::qmlAttachedProperties(QObject *object)
{
    // at the moment of the attached object creation, the root item is the only one that hasn't a parent
    // only way to avoid creation of this attached for everybody but the root item
    const QQmlEngine *engine = qmlEngine(object);
    QQmlContext *ctx = qmlContext(object);

    // Search the qml context that is the "root" for the sharedqmlobject,
    // which is an ancestor of qmlContext(object) and the direct child of the
    // engine's root context: we can do this assumption on the internals as
    // we are distributed on the same repo.
    while (ctx->parentContext() && ctx->parentContext() != engine->rootContext()) {
        ctx = ctx->parentContext();
    }

    if (!object->parent() && KQuickConfigModulePrivate::rootObjects.contains(ctx)) {
        return KQuickConfigModulePrivate::rootObjects.value(ctx);
    } else {
        return nullptr;
    }
}

QQuickItem *KQuickConfigModule::mainUi()
{
    Q_ASSERT(d->passedInEngine);
    if (d->engine) {
        return qobject_cast<QQuickItem *>(d->engine->rootObject());
    }

    d->errorString.clear();
    d->engine = new SharedQmlEngine(d->passedInEngine, this);

    const QString componentName = metaData().pluginId();
    KQuickConfigModulePrivate::rootObjects[d->engine->rootContext()] = this;
    d->engine->setTranslationDomain(componentName);
    d->engine->setInitializationDelayed(true);

    const QString resourcePath = d->getResourcePath(QStringLiteral("main.qml"));
    if (QResource r(resourcePath); !r.isValid()) {
        d->errorString = i18n("Could not find resource '%1'", resourcePath);
        qCWarning(KCMUTILS_LOG) << "Could not find resource" << resourcePath;
        return nullptr;
    }

    new QQmlFileSelector(d->engine->engine().get(), this);
    d->engine->setSource(d->getResourceUrl(resourcePath));
    d->engine->rootContext()->setContextProperty(QStringLiteral("kcm"), this);
    d->engine->completeInitialization();

    if (d->engine->isError()) {
        d->errorString = d->engine->errorString();
        return nullptr;
    }

    Q_EMIT mainUiReady();

    return qobject_cast<QQuickItem *>(d->engine->rootObject());
}

void KQuickConfigModule::push(const QString &fileName, const QVariantMap &initialProperties)
{
    // ensure main ui is created
    if (!mainUi()) {
        return;
    }

    const QString resourcePath = d->getResourcePath(fileName);
    if (QResource r(resourcePath); !r.isValid()) {
        qCWarning(KCMUTILS_LOG) << "Requested resource" << resourcePath << "does not exist";
    }
    QObject *object = d->engine->createObjectFromSource(d->getResourceUrl(resourcePath), d->engine->rootContext(), initialProperties);

    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        if (object) {
            object->deleteLater();
        }
        return;
    }

    d->subPages << item;
    Q_EMIT pagePushed(item);
    Q_EMIT depthChanged(depth());
    setCurrentIndex(d->currentIndex + 1);
}

void KQuickConfigModule::push(QQuickItem *item)
{
    // ensure main ui is created
    if (!mainUi()) {
        return;
    }

    d->subPages << item;
    Q_EMIT pagePushed(item);
    Q_EMIT depthChanged(depth());
    setCurrentIndex(d->currentIndex + 1);
}

void KQuickConfigModule::pop()
{
    if (QQuickItem *page = takeLast()) {
        page->deleteLater();
    }
}

QQuickItem *KQuickConfigModule::takeLast()
{
    if (d->subPages.isEmpty()) {
        return nullptr;
    }
    QQuickItem *page = d->subPages.takeLast();
    Q_EMIT pageRemoved();
    Q_EMIT depthChanged(depth());
    setCurrentIndex(qMin(d->currentIndex, depth() - 1));
    return page;
}

int KQuickConfigModule::columnWidth() const
{
    return d->columnWidth;
}

void KQuickConfigModule::setColumnWidth(int width)
{
    if (d->columnWidth == width) {
        return;
    }

    d->columnWidth = width;
    Q_EMIT columnWidthChanged(width);
}

int KQuickConfigModule::depth() const
{
    return d->subPages.count() + 1;
}

void KQuickConfigModule::setCurrentIndex(int index)
{
    if (index < 0 || index > d->subPages.count() || index == d->currentIndex) {
        return;
    }

    d->currentIndex = index;

    Q_EMIT currentIndexChanged(index);
}

int KQuickConfigModule::currentIndex() const
{
    return d->currentIndex;
}

std::shared_ptr<QQmlEngine> KQuickConfigModule::engine() const
{
    return d->engine->engine();
}

QString KQuickConfigModule::errorString() const
{
    return d->errorString;
}

QQuickItem *KQuickConfigModule::subPage(int index) const
{
    return d->subPages[index];
}

#include "moc_kquickconfigmodule.cpp"
