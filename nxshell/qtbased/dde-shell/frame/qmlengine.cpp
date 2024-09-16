// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlengine.h"
#include "applet.h"

#include <dobject_p.h>
#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTimer>

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

class DQmlEnginePrivate : public DObjectPrivate
{
public:
    explicit DQmlEnginePrivate(DQmlEngine *qq)
        : DObjectPrivate(qq)
    {

    }
    DApplet *m_applet = nullptr;
    QQmlContext *m_context = nullptr;
    QQmlComponent *m_component = nullptr;
    QObject *m_rootObject = nullptr;
    QQmlEngine *engine()
    {
        static QQmlEngine *s_engine = nullptr;
        if (!s_engine) {
            s_engine = new QQmlEngine();
            auto paths = s_engine->importPathList();
            // high priority for builtin plugin.
            paths.prepend(DDE_SHELL_QML_INSTALL_DIR);
            const QString rootDir = QCoreApplication::applicationDirPath();
            QDir pluginDir(rootDir);
            if (pluginDir.cd("../plugins")) {
                paths.prepend(pluginDir.absolutePath());
            }
            s_engine->setImportPathList(paths);
            qCDebug(dsLog()) << "Engine importPaths" << s_engine->importPathList();
        }
        return s_engine;
    }
    void continueLoading()
    {
        D_Q(DQmlEngine);
        if (m_component->isReady()) {
            m_rootObject = m_component->beginCreate(m_context);
            Q_EMIT q->createFinished();
        } else if (m_component->isError()) {
            qCWarning(dsLog()) << "Loading url failed" << m_component->errorString();
            Q_EMIT q->createFinished();
        }
    }
    D_DECLARE_PUBLIC(DQmlEngine)
};

DQmlEngine::DQmlEngine(QObject *parent)
    : DQmlEngine(nullptr, parent)
{

}

DQmlEngine::DQmlEngine(DApplet *applet, QObject *parent)
    : QObject(parent)
    , DObject(*new DQmlEnginePrivate(this))
{
    D_D(DQmlEngine);
    d->m_applet = applet;
}

DQmlEngine::~DQmlEngine()
{
}

void DQmlEngine::completeCreate()
{
    D_D(DQmlEngine);
    if (!d->m_component)
        return;

    if (!d->m_component->isReady())
        return;

    d->m_component->completeCreate();
    Q_EMIT finished();
}

bool DQmlEngine::create()
{
    D_D(DQmlEngine);
    std::unique_ptr<QQmlComponent> component(new QQmlComponent(engine(), this));
    const QString url = d->m_applet->pluginMetaData().url();
    if (url.isEmpty())
        return true;

    component->loadUrl(url, QQmlComponent::Asynchronous);

    auto context = new QQmlContext(engine(), d->m_applet);
    context->setContextProperty("_ds_applet", d->m_applet);
    d->m_context = context;
    d->m_component = component.release();
    if (d->m_component->isLoading()) {
        QObject::connect(d->m_component, &QQmlComponent::statusChanged, this, [this]() {
            D_D(DQmlEngine);
            d->continueLoading();
        });
    } else {
        d->continueLoading();
    }
    return true;
}

QObject *DQmlEngine::createObject(const QUrl &url)
{
    return createObject(url, {});
}

QObject *DQmlEngine::createObject(const QUrl &url, const QVariantMap &initialProperties)
{
    QQmlEngine *engine = DQmlEngine().engine();
    std::unique_ptr<QQmlComponent> component(new QQmlComponent(engine));
    component->loadUrl(url);
    if (component->isError()) {
        qCWarning(dsLog()) << "Loading url failed" << component->errorString();
        return nullptr;
    }
    std::unique_ptr<QQmlContext> context(new QQmlContext(engine, engine->rootContext()));
    auto object = component->createWithInitialProperties(initialProperties, context.get());
    if (!object)
        return nullptr;
    component->completeCreate();
    context.release();
    return object;
}

QObject *DQmlEngine::rootObject() const
{
    D_DC(DQmlEngine);
    return d->m_rootObject;
}

QQmlEngine *DQmlEngine::engine()
{
    D_D(DQmlEngine);
    return d->engine();
}

DS_END_NAMESPACE
