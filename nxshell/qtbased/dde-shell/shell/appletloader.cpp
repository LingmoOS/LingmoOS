// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletloader.h"
#include "pluginloader.h"
#include "applet.h"
#include "containment.h"
#include "appletdata.h"
#include "qmlengine.h"

#include <dobject_p.h>

#include <QMap>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <QTranslator>
#include <QApplication>
#include <QFile>
#include <DWindowManagerHelper>

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE;
DGUI_USE_NAMESPACE

Q_LOGGING_CATEGORY(dsLoaderLog, "dde.shell.loader")

namespace {
    class Benchmark
    {
    public:
        explicit Benchmark(const QString &text)
            : m_text(text)
        {
            m_timer.start();
        }
        ~Benchmark()
        {
            const auto elasped = m_timer.elapsed();
            if (elasped >= m_timeout) {
                qCWarning(dsLoaderLog) << qPrintable(m_text) << ": elasped time [" << elasped << "].";
            }
        }
    private:
        QString m_text;
        int m_timeout = 100;
        QElapsedTimer m_timer;
    };
}

class DAppletLoaderPrivate : public DObjectPrivate
{
public:
    explicit DAppletLoaderPrivate(DAppletLoader *qq)
        : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
    {
    }
    QList<DAppletData> groupList(DApplet *applet, const DAppletData &data) const
    {
        if (!data.groupList().isEmpty())
            return data.groupList();

        QList<DAppletData> groups;
        const auto children = DPluginLoader::instance()->childrenPlugin(applet->pluginMetaData().pluginId());
        for (const auto &item : children) {
            groups << DAppletData::fromPluginMetaData(item);
        }
        return groups;
    }

    ~DAppletLoaderPrivate()
    {
        for (const auto &tl : std::as_const(m_pluginTranslators)) {
            tl->deleteLater();
        }
    }

    bool doLoad(DApplet *applet);
    void doCreateRootObject(DApplet *applet);
    bool doInit(DApplet *applet);

    bool load(DApplet *applet);
    void createRootObject(DApplet *applet);
    bool init(DApplet *applet);

    void createChildren(DApplet *applet);

    void loadTranslation(const DPluginMetaData &pluginData);
    void removeTranslation(const QString &pluginId);

    QPointer<DApplet> m_applet = nullptr;
    QMap<QString, QTranslator *> m_pluginTranslators;

    D_DECLARE_PUBLIC(DAppletLoader);
};

DAppletLoader::DAppletLoader(class DApplet *applet, QObject *parent)
    : QObject(parent)
    , DObject(*new DAppletLoaderPrivate(this))
{
    D_D(DAppletLoader);
    d->m_applet = applet;
}

DAppletLoader::~DAppletLoader()
{

}

void DAppletLoader::exec()
{
    D_D(DAppletLoader);
    d->loadTranslation(d->m_applet->pluginMetaData());

    if (!d->load(d->m_applet))
        return;

    d->createRootObject(d->m_applet);

    if (!d->init(d->m_applet))
        return;
}

DApplet *DAppletLoader::applet() const
{
    D_DC(DAppletLoader);
    return d->m_applet;
}

void DAppletLoaderPrivate::doCreateRootObject(DApplet *applet)
{
    if (applet->pluginMetaData().url().isEmpty())
        return;

    DQmlEngine *engine = new DQmlEngine(applet, applet);
    QObject::connect(engine, &DQmlEngine::createFinished, applet, [this, applet, engine]() {
        auto rootObject = engine->rootObject();
        applet->setRootObject(rootObject);
        engine->completeCreate();
        if (!rootObject) {
            D_Q(DAppletLoader);
            qCWarning(dsLoaderLog) << "Create root failed:" << applet->pluginId();
            Q_EMIT q->failed(applet->pluginId());
        } else {
            qCDebug(dsLoaderLog) << "Created rootObject for the applet:" << applet->pluginId();
        }
    });

    qCDebug(dsLoaderLog) << "Begin to create rootObject the applet:" << applet->pluginId();
    if (!engine->create()) {
        engine->deleteLater();
    }
}

bool DAppletLoaderPrivate::doLoad(DApplet *applet)
{
    D_Q(DAppletLoader);
    Benchmark benchmark(QString("Load applet %1").arg(applet->pluginId()));
    Q_UNUSED(benchmark);
    if (!applet->load()) {
        qCWarning(dsLoaderLog) << "Plugin load failed:" << applet->pluginId();
        if (auto containment = qobject_cast<DContainment *>(applet->parentApplet())) {
            containment->removeApplet(applet);
        }
        Q_EMIT q->failed(applet->pluginId());
        return false;
    }
    return true;
}

bool DAppletLoaderPrivate::doInit(DApplet *applet)
{
    D_Q(DAppletLoader);
    Benchmark benchmark(QString("Init applet %1").arg(applet->pluginId()));
    Q_UNUSED(benchmark);
    if (!applet->init()) {
        qCWarning(dsLoaderLog) << "Plugin init failed:" << applet->pluginId();
        if (auto containment = qobject_cast<DContainment *>(applet->parentApplet())) {
            containment->removeApplet(applet);
        }
        Q_EMIT q->failed(applet->pluginId());
        return false;
    }
    return true;
}

void DAppletLoaderPrivate::createChildren(DApplet *applet)
{
    if (auto containment = qobject_cast<DContainment *>(applet)) {
        const auto data = applet->appletData();
        auto groups = groupList(applet, data);
        for (const auto &item : std::as_const(groups)) {

            auto child = containment->createApplet(item);
            if (!child) {
                continue;
            }
        }
    }
}

bool DAppletLoaderPrivate::load(DApplet *applet)
{
    if (!doLoad(applet)) {
        return false;
    }

    createChildren(applet);

    if (auto containment = qobject_cast<DContainment *>(applet)) {
        auto applets = containment->applets();
        for (const auto &child : std::as_const(applets)) {

            load(child);
        }
    }
    return true;
}

void DAppletLoaderPrivate::createRootObject(DApplet *applet)
{
    doCreateRootObject(applet);
    if (auto containment = qobject_cast<DContainment *>(applet)) {
        auto applets = containment->applets();
        for (const auto &child : std::as_const(applets)) {

            createRootObject(child);
        }
    }
}

bool DAppletLoaderPrivate::init(DApplet *applet)
{
    if (!doInit(applet))
        return false;

    if (auto containment = qobject_cast<DContainment *>(applet)) {
        QList<DApplet *> applets = containment->applets();
        for (const auto &child : std::as_const(applets)) {
            init(child);
        }
    }
    return true;
}

void DAppletLoaderPrivate::loadTranslation(const DPluginMetaData &pluginData)
{
    const QString baseDir = pluginData.pluginDir();
    const QString pluginId = pluginData.pluginId();

    QString pluginTransFilePath = QString(baseDir + "/translations/" + pluginId + "_%1.qm").arg(QLocale::system().name());
    if (QFile::exists(pluginTransFilePath)) {
        auto translator = new QTranslator(qApp);
        if (translator->load(pluginTransFilePath)) {
            m_pluginTranslators[pluginId] = translator;
            qApp->installTranslator(translator);
            qInfo(dsLoaderLog) << "Loaded translation:" << pluginTransFilePath;
        } else {
            qCWarning(dsLoaderLog) << "Failed to load translation:" << pluginTransFilePath;
            translator->deleteLater();
        }
    }

    const auto children = DPluginLoader::instance()->childrenPlugin(pluginId);
    for (const auto &childPluginData : children) {
        loadTranslation(childPluginData);
    }
}

void DAppletLoaderPrivate::removeTranslation(const QString &pluginId)
{
    if (m_pluginTranslators.contains(pluginId)) {
        qApp->removeTranslator(m_pluginTranslators.value(pluginId));
        m_pluginTranslators.value(pluginId)->deleteLater();
        m_pluginTranslators.remove(pluginId);
    }
}

DS_END_NAMESPACE
