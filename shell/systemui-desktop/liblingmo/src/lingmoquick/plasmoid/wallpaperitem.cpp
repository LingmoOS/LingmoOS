/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wallpaperitem.h"
#include "appletcontext_p.h"

#include "containmentitem.h"
#include "sharedqmlengine.h"

#include <KConfigLoader>
#include <KConfigPropertyMap>
#include <KDesktopFile>

#include <QDebug>
#include <QFile>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>

#include <Lingmo/Corona>
#include <Lingmo/PluginLoader>
#include <kpackage/packageloader.h>
#include <qabstractitemmodel.h>
#include <qtmetamacros.h>

#include "debug_p.h"

WallpaperItem::WallpaperItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    // resize at the beginning to avoid as much resize events as possible
    if (parent) {
        setSize(QSizeF(parent->width(), parent->height()));
    }
}

WallpaperItem::~WallpaperItem()
{
}

void WallpaperItem::classBegin()
{
    QQuickItem::classBegin();
    LingmoQuick::AppletContext *ac = qobject_cast<LingmoQuick::AppletContext *>(QQmlEngine::contextForObject(this)->parentContext());
    Q_ASSERT(ac);
    m_containment = ac->applet()->containment();
    m_wallpaperPlugin = m_containment->wallpaperPlugin();
    m_qmlObject = ac->sharedQmlEngine();
    m_qmlObject->setParent(this);

    m_pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Wallpaper"));
    m_pkg.setPath(m_wallpaperPlugin);

    if (configScheme()) {
        m_configuration = new KConfigPropertyMap(configScheme(), this);
    }

    connect(m_containment->corona(), &Lingmo::Corona::startupCompleted, this, &WallpaperItem::accentColorChanged);
}

void WallpaperItem::componentComplete()
{
    QQuickItem::componentComplete();

    m_loading = false;
    Q_EMIT isLoadingChanged();
}

QList<KPluginMetaData> WallpaperItem::listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor)
{
    auto filter = [&mimetype, &formFactor](const KPluginMetaData &md) -> bool {
        if (!formFactor.isEmpty() && !md.value(QStringLiteral("X-Lingmo-FormFactors")).contains(formFactor)) {
            return false;
        }
        return md.value(QStringLiteral("X-Lingmo-DropMimeTypes"), QStringList()).contains(mimetype);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Wallpaper"), QString(), filter);
}

KPackage::Package WallpaperItem::kPackage() const
{
    return m_pkg;
}

QString WallpaperItem::pluginName() const
{
    return m_wallpaperPlugin;
}

KConfigPropertyMap *WallpaperItem::configuration() const
{
    return m_configuration;
}

KConfigLoader *WallpaperItem::configScheme()
{
    if (!m_configLoader) {
        // FIXME: do we need "mainconfigxml" in wallpaper packagestructures?
        const QString xmlPath = m_pkg.filePath("config", QStringLiteral("main.xml"));

        KConfigGroup cfg = m_containment->config();
        cfg = KConfigGroup(&cfg, QStringLiteral("Wallpaper"));
        cfg = KConfigGroup(&cfg, m_wallpaperPlugin);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }

    return m_configLoader;
}

void WallpaperItem::requestOpenUrl(const QUrl &url)
{
    Q_EMIT openUrlRequested(url);
}

WallpaperItem *WallpaperItem::loadWallpaper(ContainmentItem *containmentItem)
{
    if (containmentItem->containment()->wallpaperPlugin().isEmpty()) {
        return nullptr;
    }
    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Wallpaper"));
    pkg.setPath(containmentItem->containment()->wallpaperPlugin());
    if (!pkg.isValid()) {
        qWarning() << "Error loading the wallpaper, no valid package loaded";
        return nullptr;
    }

    LingmoQuick::SharedQmlEngine *qmlObject = new LingmoQuick::SharedQmlEngine(containmentItem->containment());
    qmlObject->setInitializationDelayed(true);

    const QString rootPath = pkg.metadata().value(QStringLiteral("X-Lingmo-RootPath"));
    if (!rootPath.isEmpty()) {
        qmlObject->setTranslationDomain(QLatin1String("lingmo_wallpaper_") + rootPath);
    } else {
        qmlObject->setTranslationDomain(QLatin1String("lingmo_wallpaper_") + pkg.metadata().pluginId());
    }

    /*
     * The initialization is delayed, so it's fine to setSource first.
     * This also prevents many undefined wallpaper warnings caused by "wallpaper" being set
     * when the old wallpaper plugin still exists.
     */
    qmlObject->setSource(pkg.fileUrl("mainscript"));
    WallpaperItem *wallpaper = qobject_cast<WallpaperItem *>(qmlObject->rootObject());
    if (!wallpaper) {
        if (qmlObject->mainComponent() && qmlObject->mainComponent()->isError()) {
            qWarning() << "Error loading the wallpaper" << qmlObject->mainComponent()->errors();
        } else if (qmlObject->rootObject()) {
            qWarning() << "Root item of wallpaper" << containmentItem->containment()->wallpaperPlugin() << "not a WallpaperItem instance, instead is"
                       << qmlObject->rootObject();
        }
        qmlObject->completeInitialization();
        delete qmlObject->rootObject();
        return nullptr;
    }

    if (!qEnvironmentVariableIntValue("LINGMO_NO_CONTEXTPROPERTIES")) {
        qmlObject->rootContext()->setContextProperty(QStringLiteral("wallpaper"), wallpaper);
    }

    // initialize with our size to avoid as much resize events as possible
    QVariantHash props;
    props[QStringLiteral("parent")] = QVariant::fromValue(containmentItem);
    props[QStringLiteral("width")] = containmentItem->width();
    props[QStringLiteral("height")] = containmentItem->height();
    qmlObject->completeInitialization(props);
    return wallpaper;
}

QList<QAction *> WallpaperItem::contextualActions() const
{
    return m_contextualActions;
}

QQmlListProperty<QAction> WallpaperItem::qmlContextualActions()
{
    return QQmlListProperty<QAction>(this,
                                     nullptr,
                                     WallpaperItem::contextualActions_append,
                                     WallpaperItem::contextualActions_count,
                                     WallpaperItem::contextualActions_at,
                                     WallpaperItem::contextualActions_clear,
                                     WallpaperItem::contextualActions_replace,
                                     WallpaperItem::contextualActions_removeLast);
}

bool WallpaperItem::supportsMimetype(const QString &mimetype) const
{
    return m_pkg.metadata().value(QStringLiteral("X-Lingmo-DropMimeTypes"), QStringList()).contains(mimetype);
}

bool WallpaperItem::isLoading() const
{
    return m_loading;
}

QColor WallpaperItem::accentColor() const
{
    return m_accentColor.value_or(QColor(Qt::transparent));
}

void WallpaperItem::setAccentColor(const QColor &newColor)
{
    if (m_accentColor.has_value() && m_accentColor == newColor) {
        return;
    }

    m_accentColor = newColor;
    Q_EMIT accentColorChanged();
}

void WallpaperItem::resetAccentColor()
{
    if (!m_accentColor.has_value()) {
        return;
    }

    m_accentColor.reset();
    Q_EMIT accentColorChanged();
}

void WallpaperItem::contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    w->m_contextualActions.append(action);
    QObject::connect(action, &QObject::destroyed, w, [w, action]() {
        w->m_contextualActions.removeAll(action);
        Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
    });
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
};

qsizetype WallpaperItem::contextualActions_count(QQmlListProperty<QAction> *prop)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    return w->m_contextualActions.count();
}

QAction *WallpaperItem::contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    return w->m_contextualActions.value(idx);
}

void WallpaperItem::contextualActions_clear(QQmlListProperty<QAction> *prop)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    w->m_contextualActions.clear();
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

void WallpaperItem::contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    w->m_contextualActions.replace(idx, action);
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

void WallpaperItem::contextualActions_removeLast(QQmlListProperty<QAction> *prop)
{
    WallpaperItem *w = static_cast<WallpaperItem *>(prop->object);
    w->m_contextualActions.pop_back();
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

#include "moc_wallpaperitem.cpp"
