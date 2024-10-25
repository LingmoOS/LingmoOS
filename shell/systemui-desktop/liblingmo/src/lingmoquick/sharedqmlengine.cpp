/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sharedqmlengine.h"
#include "appletcontext_p.h"

#include <KLocalizedContext>
#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickItem>
#include <QTimer>

#include <Lingmo/Applet>

#include "debug_p.h"

namespace LingmoQuick
{

class SharedQmlEnginePrivate
{
public:
    SharedQmlEnginePrivate(SharedQmlEngine *parent)
        : q(parent)
        , component(nullptr)
        , delay(false)
        , m_engine(engine())
    {
        executionEndTimer = new QTimer(q);
        executionEndTimer->setInterval(0);
        executionEndTimer->setSingleShot(true);
        QObject::connect(executionEndTimer, &QTimer::timeout, q, [this]() {
            scheduleExecutionEnd();
        });
    }

    ~SharedQmlEnginePrivate() = default;

    void errorPrint(QQmlComponent *component);
    void beginExecute(const QUrl &source);
    void beginExecute(QAnyStringView module, QAnyStringView type);
    void endExecute();
    void scheduleExecutionEnd();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();

    SharedQmlEngine *q;

    QPointer<QObject> rootObject;
    std::unique_ptr<QQmlComponent> component;
    QTimer *executionEndTimer;
    KLocalizedContext *context{nullptr};
    QQmlContext *rootContext;
    bool delay;
    std::shared_ptr<QQmlEngine> m_engine;

private:
    static std::shared_ptr<QQmlEngine> engine()
    {
        if (auto locked = s_engine.lock()) {
            return locked;
        }
        auto createdEngine = std::make_shared<QQmlEngine>();
        s_engine = createdEngine;
        return createdEngine;
    }

    static std::weak_ptr<QQmlEngine> s_engine;
};

std::weak_ptr<QQmlEngine> SharedQmlEnginePrivate::s_engine = {};

void SharedQmlEnginePrivate::errorPrint(QQmlComponent *component)
{
    QString errorStr = QStringLiteral("Error loading QML file.\n");
    if (component->isError()) {
        const QList<QQmlError> errors = component->errors();
        for (const QQmlError &error : errors) {
            errorStr +=
                (error.line() > 0 ? QString(QString::number(error.line()) + QLatin1String(": ")) : QLatin1String("")) + error.description() + QLatin1Char('\n');
        }
    }
    qWarning(LOG_LINGMOQUICK) << component->url().toString() << '\n' << errorStr;
}

void SharedQmlEnginePrivate::beginExecute(const QUrl &source)
{
    if (source.isEmpty()) {
        qWarning(LOG_LINGMOQUICK) << "File name empty!";
        return;
    }

    component = std::make_unique<QQmlComponent>(m_engine.get());
    // Important! Some parts of Lingmo are extremely sensitive to status changed
    // signal being emit in exactly the same way QQmlComponent does it. So this
    // connection needs to happen before any loading of the component happens.
    QObject::connect(component.get(), &QQmlComponent::statusChanged, q, &SharedQmlEngine::statusChanged, Qt::QueuedConnection);
    component->loadUrl(source);

    endExecute();
}

void SharedQmlEnginePrivate::beginExecute(QAnyStringView module, QAnyStringView type)
{
    if (module.isEmpty() || type.isEmpty()) {
        qWarning(LOG_LINGMOQUICK) << "No module or type specified";
        return;
    }

    component = std::make_unique<QQmlComponent>(m_engine.get());
    // Important! Some parts of Lingmo are extremely sensitive to status changed
    // signal being emit in exactly the same way QQmlComponent does it. So this
    // connection needs to happen before any loading of the component happens.
    QObject::connect(component.get(), &QQmlComponent::statusChanged, q, &SharedQmlEngine::statusChanged, Qt::QueuedConnection);
    component->loadFromModule(module, type);

    endExecute();
}

void SharedQmlEnginePrivate::endExecute()
{
    rootObject = component->beginCreate(rootContext);

    if (delay) {
        executionEndTimer->start(0);
    } else {
        scheduleExecutionEnd();
    }
}

void SharedQmlEnginePrivate::scheduleExecutionEnd()
{
    if (component->isReady() || component->isError()) {
        q->completeInitialization();
    } else {
        QObject::connect(component.get(), &QQmlComponent::statusChanged, q, [this]() {
            q->completeInitialization();
        });
    }
}

SharedQmlEngine::SharedQmlEngine(QObject *parent)
    : QObject(parent)
    , d(new SharedQmlEnginePrivate(this))
{
    d->rootContext = new QQmlContext(engine().get());
    d->rootContext->setParent(this); // Delete the context when deleting the shared engine

    d->context = new KLocalizedContext(d->rootContext);
    d->rootContext->setContextObject(d->context);
}

SharedQmlEngine::SharedQmlEngine(Lingmo::Applet *applet, QObject *parent)
    : QObject(parent)
    , d(new SharedQmlEnginePrivate(this))
{
    d->rootContext = new AppletContext(engine().get(), applet, this);

    d->context = new KLocalizedContext(d->rootContext);
    d->rootContext->setContextObject(d->context);
}

SharedQmlEngine::~SharedQmlEngine()
{
    if (QJSEngine::objectOwnership(d->rootObject) == QJSEngine::CppOwnership) {
        delete d->rootObject;
    }
}

void SharedQmlEngine::setTranslationDomain(const QString &translationDomain)
{
    d->context->setTranslationDomain(translationDomain);
}

QString SharedQmlEngine::translationDomain() const
{
    return d->context->translationDomain();
}

void SharedQmlEngine::setSource(const QUrl &source)
{
    d->beginExecute(source);
}

void SharedQmlEngine::setSourceFromModule(QAnyStringView module, QAnyStringView type)
{
    d->beginExecute(module, type);
}

QUrl SharedQmlEngine::source() const
{
    if (d->component) {
        return d->component->url();
    }
    return QUrl{};
}

void SharedQmlEngine::setInitializationDelayed(const bool delay)
{
    d->delay = delay;
}

bool SharedQmlEngine::isInitializationDelayed() const
{
    return d->delay;
}

std::shared_ptr<QQmlEngine> SharedQmlEngine::engine()
{
    return d->m_engine;
}

QObject *SharedQmlEngine::rootObject() const
{
    return d->rootObject;
}

QQmlComponent *SharedQmlEngine::mainComponent() const
{
    return d->component.get();
}

QQmlContext *SharedQmlEngine::rootContext() const
{
    return d->rootContext;
}

QQmlComponent::Status SharedQmlEngine::status() const
{
    if (!d->m_engine) {
        return QQmlComponent::Error;
    }

    if (!d->component) {
        return QQmlComponent::Null;
    }

    return QQmlComponent::Status(d->component->status());
}

void SharedQmlEngine::completeInitialization(const QVariantHash &initialProperties)
{
    d->executionEndTimer->stop();

    if (!d->component) {
        qWarning(LOG_LINGMOQUICK) << "No component for" << source();
        return;
    }

    if (d->component->status() != QQmlComponent::Ready || d->component->isError()) {
        d->errorPrint(d->component.get());
        return;
    }

    for (auto it = initialProperties.constBegin(); it != initialProperties.constEnd(); ++it) {
        d->rootObject->setProperty(it.key().toUtf8().data(), it.value());
    }

    d->component->completeCreate();
    Q_EMIT finished();
}

QObject *SharedQmlEngine::createObjectFromSource(const QUrl &source, QQmlContext *context, const QVariantHash &initialProperties)
{
    QQmlComponent *component = new QQmlComponent(d->m_engine.get(), this);
    component->loadUrl(source);

    return createObjectFromComponent(component, context, initialProperties);
}

QObject *SharedQmlEngine::createObjectFromComponent(QQmlComponent *component, QQmlContext *context, const QVariantHash &initialProperties)
{
    QObject *object = component->beginCreate(context ? context : d->rootContext);

    for (auto it = initialProperties.constBegin(); it != initialProperties.constEnd(); ++it) {
        object->setProperty(it.key().toUtf8().data(), it.value());
    }
    component->completeCreate();

    if (!component->isError() && object) {
        // memory management
        component->setParent(object);
        // reparent to root object if wasn't specified otherwise by initialProperties
        if (!initialProperties.contains(QLatin1String("parent"))) {
            if (qobject_cast<QQuickItem *>(rootObject())) {
                object->setProperty("parent", QVariant::fromValue(rootObject()));
            } else {
                object->setParent(rootObject());
            }
        }

        return object;

    } else {
        d->errorPrint(component);
        delete object;
        return nullptr;
    }
}
}

#include "moc_sharedqmlengine.cpp"
