/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sharedqmlengine_p.h"

#include <KLocalizedContext>
#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlIncubator>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickItem>
#include <QResource>
#include <QTimer>

#include "kcmutils_debug.h"

using namespace Qt::StringLiterals;

class SharedQmlEnginePrivate
{
public:
    SharedQmlEnginePrivate(const std::shared_ptr<QQmlEngine> &engine, SharedQmlEngine *parent)
        : q(parent)
        , component(nullptr)
        , delay(false)
        , m_engine(engine)
    {
        executionEndTimer.setInterval(0);
        executionEndTimer.setSingleShot(true);
        QObject::connect(&executionEndTimer, &QTimer::timeout, q, [this]() {
            scheduleExecutionEnd();
        });
    }

    ~SharedQmlEnginePrivate()
    {
        delete incubator.object();
    }

    void errorPrint(QQmlComponent *component, QQmlIncubator *incubator = nullptr);
    void execute(const QUrl &source);
    void scheduleExecutionEnd();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();
    void checkInitializationCompleted();

    SharedQmlEngine *q;

    QUrl source;

    QQmlIncubator incubator;
    QQmlComponent *component;
    QTimer executionEndTimer;
    KLocalizedContext *context{nullptr};
    QQmlContext *rootContext;
    bool delay;
    std::shared_ptr<QQmlEngine> m_engine;
};

void SharedQmlEnginePrivate::errorPrint(QQmlComponent *component, QQmlIncubator *incubator)
{
    QList<QQmlError> errors;
    if (component && component->isError()) {
        errors = component->errors();
    } else if (incubator && incubator->isError()) {
        errors = incubator->errors();
    } else {
        return;
    }

    qCWarning(KCMUTILS_LOG).noquote() << "Error loading QML file" << component->url().toString();
    for (const auto &error : errors) {
        constexpr const QLatin1String indent("    ");
        qCWarning(KCMUTILS_LOG).noquote().nospace() << indent << error;
    }
}

void SharedQmlEnginePrivate::execute(const QUrl &source)
{
    Q_ASSERT(!source.isEmpty());
    delete component;
    component = new QQmlComponent(m_engine.get(), q);
    delete incubator.object();

    m_engine->addImportPath(QStringLiteral("qrc:/"));
    component->loadUrl(source);

    if (delay) {
        executionEndTimer.start(0);
    } else {
        scheduleExecutionEnd();
    }
}

void SharedQmlEnginePrivate::scheduleExecutionEnd()
{
    if (component->isReady() || component->isError()) {
        q->completeInitialization();
    } else {
        QObject::connect(component, &QQmlComponent::statusChanged, q, [this]() {
            q->completeInitialization();
        });
    }
}

SharedQmlEngine::SharedQmlEngine(const std::shared_ptr<QQmlEngine> &engine, QObject *parent)
    : QObject(parent)
    , d(new SharedQmlEnginePrivate(engine, this))
{
    d->rootContext = new QQmlContext(engine.get());
    d->rootContext->setParent(this); // Delete the context when deleting the shared engine

    d->context = new KLocalizedContext(d->rootContext);
    d->rootContext->setContextObject(d->context);
}

SharedQmlEngine::~SharedQmlEngine() = default;

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
    d->source = source;
    d->execute(source);
}

QUrl SharedQmlEngine::source() const
{
    return d->source;
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
    if (d->incubator.isLoading()) {
        qCWarning(KCMUTILS_LOG) << "Trying to use rootObject before initialization is completed, whilst using setInitializationDelayed. Forcing completion";
        d->incubator.forceCompletion();
    }
    return d->incubator.object();
}

QQmlComponent *SharedQmlEngine::mainComponent() const
{
    return d->component;
}

QQmlContext *SharedQmlEngine::rootContext() const
{
    return d->rootContext;
}

bool SharedQmlEngine::isError() const
{
    return !d->m_engine || !d->component || d->component->isError() || d->incubator.isError();
}

static QString qmlErrorsToString(const QList<QQmlError> &errors)
{
    QString ret;
    for (const auto &e : errors) {
        ret += e.url().toString() + QLatin1Char(':') + QString::number(e.line()) + QLatin1Char(' ') + e.description() + QLatin1Char('\n');
    }
    return ret;
}

QString SharedQmlEngine::errorString() const
{
    if (d->component && d->component->isError()) {
        return d->component->errorString();
    } else if (d->incubator.isError()) {
        return qmlErrorsToString(d->incubator.errors());
    } else {
        return {};
    }
}

void SharedQmlEnginePrivate::checkInitializationCompleted()
{
    if (!incubator.isReady() && !incubator.isError()) {
        QTimer::singleShot(0, q, [this]() {
            checkInitializationCompleted();
        });
        return;
    }

    if (!incubator.object()) {
        errorPrint(component, &incubator);
    }

    Q_EMIT q->finished();
}

void SharedQmlEngine::completeInitialization(const QVariantMap &initialProperties)
{
    d->executionEndTimer.stop();
    if (d->incubator.object()) {
        return;
    }

    if (!d->component) {
        qCWarning(KCMUTILS_LOG) << "No component for" << source();
        return;
    }

    if (!d->component->isReady()) {
        d->errorPrint(d->component);
        return;
    }

    d->incubator.setInitialProperties(initialProperties);
    d->component->create(d->incubator, d->rootContext);

    if (d->delay) {
        d->checkInitializationCompleted();
    } else {
        d->incubator.forceCompletion();

        if (!d->incubator.object()) {
            d->errorPrint(d->component, &d->incubator);
        }
        Q_EMIT finished();
    }
}

QObject *SharedQmlEngine::createObjectFromSource(const QUrl &source, QQmlContext *context, const QVariantMap &initialProperties)
{
    QQmlComponent *component = new QQmlComponent(d->m_engine.get(), this);
    component->loadUrl(source);

    return createObjectFromComponent(component, context, initialProperties);
}

QObject *SharedQmlEngine::createObjectFromComponent(QQmlComponent *component, QQmlContext *context, const QVariantMap &initialProperties)
{
    QObject *object = component->createWithInitialProperties(initialProperties, context ? context : d->rootContext);

    if (!component->isError() && object) {
        // memory management
        const auto root = rootObject();
        object->setParent(root);
        component->setParent(object);

        // visually reparent to root object if wasn't specified otherwise by initialProperties
        if (!initialProperties.contains(QLatin1String("parent")) && root && root->isQuickItemType()) {
            object->setProperty("parent", QVariant::fromValue(root));
        }
        return object;
    } else {
        d->errorPrint(component);
        delete object;
        return nullptr;
    }
}

#include "moc_sharedqmlengine_p.cpp"
