/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wallpaperinterface.h"

#include "containmentinterface.h"

#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
#include <kdeclarative/configpropertymap.h>
#else
#include <KConfigPropertyMap>
#endif
#include <kdeclarative/qmlobjectsharedengine.h>

#include <KActionCollection>
#include <KConfigLoader>
#include <KDesktopFile>

#include <QDebug>
#include <QFile>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>
#include <kpackage/packageloader.h>

QHash<QObject *, WallpaperInterface *> WallpaperInterface::s_rootObjects = QHash<QObject *, WallpaperInterface *>();

WallpaperInterface::WallpaperInterface(ContainmentInterface *parent)
    : QQuickItem(parent)
    , m_containmentInterface(parent)
    , m_qmlObject(nullptr)
    , m_configuration(nullptr)
    , m_configLoader(nullptr)
{
    m_actions = new KActionCollection(this);

    // resize at the beginning to avoid as much resize events as possible
    if (parent) {
        setSize(QSizeF(parent->width(), parent->height()));
    }

    if (!m_containmentInterface->containment()->wallpaper().isEmpty()) {
        syncWallpaperPackage();
    }
    connect(m_containmentInterface->containment(), &Plasma::Containment::wallpaperChanged, this, &WallpaperInterface::syncWallpaperPackage);
    connect(m_containmentInterface->containment()->corona(),
            &Plasma::Corona::startupCompleted,
            this,
            std::bind(&WallpaperInterface::repaintNeeded, this, Qt::transparent));
}

WallpaperInterface::~WallpaperInterface()
{
    if (m_qmlObject) {
        s_rootObjects.remove(m_qmlObject->engine());
    }
}

QList<KPluginMetaData> WallpaperInterface::listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor)
{
    auto filter = [&mimetype, &formFactor](const KPluginMetaData &md) -> bool {
        if (!formFactor.isEmpty() && !md.value(QStringLiteral("X-Plasma-FormFactors")).contains(formFactor)) {
            return false;
        }
        return md.value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimetype);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Wallpaper"), QString(), filter);
}

KPackage::Package WallpaperInterface::kPackage() const
{
    return m_pkg;
}

QString WallpaperInterface::pluginName() const
{
    return m_wallpaperPlugin;
}

#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
KDeclarative::ConfigPropertyMap *WallpaperInterface::configuration() const
#else
KConfigPropertyMap *WallpaperInterface::configuration() const
#endif
{
    return m_configuration;
}

KConfigLoader *WallpaperInterface::configScheme()
{
    if (!m_configLoader) {
        // FIXME: do we need "mainconfigxml" in wallpaper packagestructures?
        const QString xmlPath = m_pkg.filePath("config", QStringLiteral("main.xml"));

        KConfigGroup cfg = m_containmentInterface->containment()->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");
        cfg = KConfigGroup(&cfg, m_wallpaperPlugin);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
        connect(m_configLoader, &KConfigLoader::configChanged, this, &WallpaperInterface::configurationChanged);
    }

    return m_configLoader;
}

void WallpaperInterface::syncWallpaperPackage()
{
    if (m_wallpaperPlugin == m_containmentInterface->containment()->wallpaper() && m_qmlObject->rootObject()) {
        return;
    }

    m_wallpaperPlugin = m_containmentInterface->containment()->wallpaper();

    if (!m_qmlObject) {
        m_qmlObject = new KDeclarative::QmlObjectSharedEngine(this);
        s_rootObjects[m_qmlObject->engine()] = this;
        m_qmlObject->setInitializationDelayed(true);
        connect(m_qmlObject, &KDeclarative::QmlObject::finished, this, &WallpaperInterface::loadFinished);
    }

    m_actions->clear();
    setProperty("contextualActions", QVariant::fromValue(contextualActions()));
    m_pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper"));
    m_pkg.setPath(m_wallpaperPlugin);
    if (!m_pkg.isValid()) {
        qWarning() << "Error loading the wallpaper, no valid package loaded";
        return;
    }

    if (m_configLoader) {
        m_configLoader->deleteLater();
    }
    if (m_configuration) {
        m_configuration->deleteLater();
    }
    m_configLoader = nullptr;
    m_configuration = nullptr;
    if (configScheme()) {
#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
        m_configuration = new KDeclarative::ConfigPropertyMap(configScheme(), this);
#else
        m_configuration = new KConfigPropertyMap(configScheme(), this);
#endif
    }

    /*
     * The initialization is delayed, so it's fine to setSource first.
     * This also prevents many undefined wallpaper warnings caused by "wallpaper" being set
     * when the old wallpaper plugin still exists.
     */
    m_qmlObject->setSource(m_pkg.fileUrl("mainscript"));
    if (!qEnvironmentVariableIntValue("PLASMA_NO_CONTEXTPROPERTIES")) {
        m_qmlObject->rootContext()->setContextProperty(QStringLiteral("wallpaper"), this);
    }

    const QString rootPath = m_pkg.metadata().value(QStringLiteral("X-Plasma-RootPath"));
    if (!rootPath.isEmpty()) {
        m_qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + rootPath);
    } else {
        m_qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + m_pkg.metadata().pluginId());
    }

    // initialize with our size to avoid as much resize events as possible
    QVariantHash props;
    props[QStringLiteral("width")] = width();
    props[QStringLiteral("height")] = height();
    m_qmlObject->completeInitialization(props);
    Q_EMIT repaintNeeded();
}

void WallpaperInterface::loadFinished()
{
    if (m_qmlObject->mainComponent() //
        && m_qmlObject->rootObject() //
        && !m_qmlObject->mainComponent()->isError()) {
        m_qmlObject->rootObject()->setProperty("z", -1000);
        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

        // set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), QStringLiteral("parent"));
        QQmlProperty prop(m_qmlObject->rootObject(), QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());

    } else if (m_qmlObject->mainComponent()) {
        qWarning() << "Error loading the wallpaper" << m_qmlObject->mainComponent()->errors();
        s_rootObjects.remove(m_qmlObject->engine());
        m_qmlObject->deleteLater();
        m_qmlObject = nullptr;

    } else {
        qWarning() << "Error loading the wallpaper, package not found";
    }

    Q_EMIT packageChanged();
    Q_EMIT configurationChanged();
}

QList<QAction *> WallpaperInterface::contextualActions() const
{
    return m_actions->actions();
}

bool WallpaperInterface::supportsMimetype(const QString &mimetype) const
{
    return m_pkg.metadata().value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimetype);
}

void WallpaperInterface::setUrl(const QUrl &url)
{
    if (m_qmlObject->rootObject()) {
        QMetaObject::invokeMethod(m_qmlObject->rootObject(), "setUrl", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(url)));
    }
}

void WallpaperInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    QAction *action = m_actions->action(name);

    if (action) {
        action->setText(text);
    } else {
        Q_ASSERT(!m_actions->action(name));
        action = new QAction(text, this);
        m_actions->addAction(name, action);

        connect(action, &QAction::triggered, this, [this, name] {
            executeAction(name);
        });
    }

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }

    action->setObjectName(name);
    setProperty("contextualActions", QVariant::fromValue(contextualActions()));
}

void WallpaperInterface::removeAction(const QString &name)
{
    QAction *action = m_actions->action(name);

    if (action) {
        m_actions->removeAction(action);
    }
    setProperty("contextualActions", QVariant::fromValue(contextualActions()));
}

QAction *WallpaperInterface::action(QString name) const
{
    return m_actions->action(name);
}

void WallpaperInterface::executeAction(const QString &name)
{
    if (m_qmlObject->rootObject()) {
        const QByteArray actionName("action_" + name.toUtf8());
        QMetaObject::invokeMethod(m_qmlObject->rootObject(), actionName.constData(), Qt::DirectConnection);
    }
}

WallpaperInterface *WallpaperInterface::qmlAttachedProperties(QObject *object)
{
    // at the moment of the attached object creation, the root item is the only one that hasn't a parent
    // only way to avoid creation of this attached for everybody but the root item
    return object->parent() ? nullptr : s_rootObjects.value(qmlEngine(object));
}

bool WallpaperInterface::isLoading() const
{
    return m_loading;
}

#include "moc_wallpaperinterface.cpp"
