/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configview.h"
#include "Lingmo/Applet"
#include "Lingmo/Containment"
#include "appletcontext_p.h"
#include "appletquickitem.h"
#include "configcategory_p.h"
#include "configmodel.h"

#include <QDebug>
#include <QDir>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickItem>

#include <KAuthorized>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KPackage/Package>

#include <Lingmo/Corona>
#include <Lingmo/PluginLoader>
#include <qqmlengine.h>

// Unfortunately QWINDOWSIZE_MAX is not exported
#define DIALOGSIZE_MAX ((1 << 24) - 1)

namespace LingmoQuick
{
//////////////////////////////ConfigView

class ConfigViewPrivate
{
public:
    ConfigViewPrivate(Lingmo::Applet *appl, ConfigView *view);
    ~ConfigViewPrivate() = default;

    void init();

    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();
    void updateTitle();
    void mainItemLoaded();

    ConfigView *q;
    QPointer<Lingmo::Applet> applet;
    ConfigModel *configModel;
    ConfigModel *kcmConfigModel;
    Lingmo::Corona *corona;
    AppletContext *rootContext;
    QQmlEngine *engine = nullptr;
    QQuickItem *rootItem = nullptr;

    // Attached Layout property of mainItem, if any
    QPointer<QObject> mainItemLayout;
};

ConfigViewPrivate::ConfigViewPrivate(Lingmo::Applet *appl, ConfigView *view)
    : q(view)
    , applet(appl)
    , corona(nullptr)
{
    engine = new QQmlEngine(q);
}

void ConfigViewPrivate::init()
{
    if (!applet) {
        qWarning() << "Null applet passed to constructor";
        return;
    }
    if (!applet.data()->pluginMetaData().isValid()) {
        qWarning() << "Invalid applet passed to constructor";
        if (applet->containment()) {
            corona = applet->containment()->corona();
        }
        return;
    }

    rootContext = new AppletContext(q->engine(), applet, nullptr);
    rootContext->setParent(q->engine());

    applet.data()->setUserConfiguring(true);

    KLocalizedContext *localizedContextObject = new KLocalizedContext(q->engine());
    localizedContextObject->setTranslationDomain(applet->translationDomain());
    rootContext->setContextObject(localizedContextObject);

    // FIXME: problem on nvidia, all windows should be transparent or won't show
    q->setColor(Qt::transparent);
    updateTitle();

    // systray case
    if (!applet.data()->containment()->corona()) {
        Lingmo::Applet *a = qobject_cast<Lingmo::Applet *>(applet.data()->containment()->parent());
        if (a) {
            corona = a->containment()->corona();
        }
    } else {
        if (!applet.data()->containment()->corona()->kPackage().isValid()) {
            qWarning() << "Invalid home screen package";
        }
        corona = applet.data()->containment()->corona();
    }
    if (!corona) {
        qWarning() << "Cannot find a Corona, this should never happen!";
        return;
    }

    const auto pkg = corona->kPackage();
    if (pkg.isValid()) {
        new QQmlFileSelector(q->engine(), q->engine());
    }

    if (!qEnvironmentVariableIntValue("LINGMO_NO_CONTEXTPROPERTIES")) {
        rootContext->setContextProperties({QQmlContext::PropertyPair{QStringLiteral("plasmoid"), QVariant::fromValue(applet.data())},
                                           QQmlContext::PropertyPair{QStringLiteral("configDialog"), QVariant::fromValue(q)}});
    }

    // config model local of the applet
    QQmlComponent component(q->engine(), applet.data()->configModel());
    QObject *object = component.create(rootContext);
    configModel = qobject_cast<ConfigModel *>(object);

    if (configModel) {
        configModel->setApplet(applet.data());
        configModel->setParent(q);
    } else {
        delete object;
    }

    QStringList kcms = applet.data()->pluginMetaData().value(QStringLiteral("X-Lingmo-ConfigPlugins"), QStringList());

    // filter out non-authorized KCMs
    // KAuthorized expects KCMs with .desktop suffix, so we can't just pass everything
    // to KAuthorized::authorizeControlModules verbatim
    kcms.erase(std::remove_if(kcms.begin(),
                              kcms.end(),
                              [](const QString &kcm) {
                                  return !KAuthorized::authorizeControlModule(kcm + QLatin1String(".desktop"));
                              }),
               kcms.end());

    if (!kcms.isEmpty()) {
        if (!configModel) {
            configModel = new ConfigModel(q);
        }

        for (const QString &kcm : std::as_const(kcms)) {
            // Only look for KCMs in the "kcms_" folder where new QML KCMs live
            // because we don't support loading QWidgets KCMs
            KPluginMetaData md(QLatin1String("kcms/") + kcm);

            if (!md.isValid()) {
                qWarning() << "Could not find" << kcm
                           << "requested by X-Lingmo-ConfigPlugins. Ensure that it exists, is a QML KCM, and lives in the 'kcms/' subdirectory.";
                continue;
            }

            configModel->appendCategory(md.iconName(), md.name(), QString(), QLatin1String("kcms/") + kcm);
        }
    }
}

void ConfigViewPrivate::updateMinimumWidth()
{
    if (mainItemLayout) {
        q->setMinimumWidth(mainItemLayout.data()->property("minimumWidth").toInt());
        // Sometimes setMinimumWidth doesn't actually resize: Qt bug?

        q->setWidth(qMax(q->width(), q->minimumWidth()));
    } else {
        q->setMinimumWidth(-1);
    }
}

void ConfigViewPrivate::updateMinimumHeight()
{
    if (mainItemLayout) {
        q->setMinimumHeight(mainItemLayout.data()->property("minimumHeight").toInt());
        // Sometimes setMinimumHeight doesn't actually resize: Qt bug?

        q->setHeight(qMax(q->height(), q->minimumHeight()));
    } else {
        q->setMinimumHeight(-1);
    }
}

void ConfigViewPrivate::updateMaximumWidth()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumWidth").toInt();

        if (hint > 0) {
            q->setMaximumWidth(hint);
        } else {
            q->setMaximumWidth(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumWidth(DIALOGSIZE_MAX);
    }
}

void ConfigViewPrivate::updateMaximumHeight()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumHeight").toInt();

        if (hint > 0) {
            q->setMaximumHeight(hint);
        } else {
            q->setMaximumHeight(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumHeight(DIALOGSIZE_MAX);
    }
}

void ConfigViewPrivate::updateTitle()
{
    QVariant itemTitle = rootItem ? rootItem->property("title") : QVariant();
    q->setTitle(itemTitle.canConvert<QString>() ? i18n("%1 — %2 Settings", itemTitle.toString(), applet.data()->title())
                                                : i18n("%1 Settings", applet.data()->title()));
}

void ConfigViewPrivate::mainItemLoaded()
{
    if (applet) {
        KConfigGroup cg = applet.data()->config();
        cg = KConfigGroup(&cg, QStringLiteral("ConfigDialog"));
        q->resize(cg.readEntry("DialogWidth", q->width()), cg.readEntry("DialogHeight", q->height()));

        if (rootItem->property("title").isValid()) {
            QObject::connect(rootItem, SIGNAL(titleChanged()), q, SLOT(updateTitle()));
            updateTitle();
        }
    }

    // Extract the representation's Layout, if any
    QObject *layout = nullptr;

    // Search a child that has the needed Layout properties
    // HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    const auto children = rootItem->children();
    for (QObject *child : children) {
        // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() && child->property("preferredWidth").isValid()
            && child->property("preferredHeight").isValid() && child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid()
            && child->property("fillWidth").isValid() && child->property("fillHeight").isValid()) {
            layout = child;
            break;
        }
    }
    mainItemLayout = layout;

    if (layout) {
        QObject::connect(layout, SIGNAL(minimumWidthChanged()), q, SLOT(updateMinimumWidth()));
        QObject::connect(layout, SIGNAL(minimumHeightChanged()), q, SLOT(updateMinimumHeight()));
        QObject::connect(layout, SIGNAL(maximumWidthChanged()), q, SLOT(updateMaximumWidth()));
        QObject::connect(layout, SIGNAL(maximumHeightChanged()), q, SLOT(updateMaximumHeight()));

        updateMinimumWidth();
        updateMinimumHeight();
        updateMaximumWidth();
        updateMaximumHeight();
    }
}

ConfigView::ConfigView(Lingmo::Applet *applet, QWindow *parent)
    : QQuickWindow(parent)
    , d(new ConfigViewPrivate(applet, this))
{
    setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    // Only register types once
    [[maybe_unused]] static int configModelRegisterResult = qmlRegisterType<ConfigModel>("org.kde.lingmo.configuration", 2, 0, "ConfigModel");
    [[maybe_unused]] static int configCategoryRegisterResult = qmlRegisterType<ConfigCategory>("org.kde.lingmo.configuration", 2, 0, "ConfigCategory");
    d->init();
    connect(applet, &QObject::destroyed, this, &ConfigView::close);
}

ConfigView::~ConfigView()
{
    if (d->applet) {
        d->applet.data()->setUserConfiguring(false);
        if (d->applet.data()->containment() && d->applet.data()->containment()->corona()) {
            d->applet.data()->containment()->corona()->requestConfigSync();
        }
    }
    delete d->rootItem;
}

QQmlEngine *ConfigView::engine()
{
    return d->engine;
}

QQmlContext *ConfigView::rootContext()
{
    return d->rootContext;
}

void ConfigView::setSource(const QUrl &src)
{
    QQmlComponent uiComponent(engine(), src);
    if (uiComponent.isError()) {
        for (const auto &error : uiComponent.errors()) {
            qWarning() << error;
        }
    }

    std::unique_ptr<QObject> object(uiComponent.createWithInitialProperties({{QStringLiteral("parent"), QVariant::fromValue(contentItem())}}, d->rootContext));
    d->rootItem = qobject_cast<QQuickItem *>(object.get());
    if (!d->rootItem) {
        return;
    }
    Q_UNUSED(object.release());
    d->mainItemLoaded();

    if (d->rootItem->implicitHeight() > 0 || d->rootItem->implicitWidth() > 0) {
        resize(QSize(d->rootItem->implicitWidth(), d->rootItem->implicitHeight()));
    }
    d->rootItem->setSize(QSizeF(width(), height()));

    connect(d->rootItem, &QQuickItem::implicitWidthChanged, this, [this]() {
        setWidth(d->rootItem->implicitWidth());
    });
    connect(d->rootItem, &QQuickItem::implicitHeightChanged, this, [this]() {
        setWidth(d->rootItem->implicitHeight());
    });
}

QQuickItem *ConfigView::rootObject()
{
    return d->rootItem;
}

void ConfigView::init()
{
    setSource(d->corona->kPackage().fileUrl("appletconfigurationui"));
}

Lingmo::Applet *ConfigView::applet()
{
    return d->applet.data();
}

ConfigModel *ConfigView::configModel() const
{
    return d->configModel;
}

QString ConfigView::appletGlobalShortcut() const
{
    if (!d->applet) {
        return QString();
    }

    return d->applet.data()->globalShortcut().toString();
}

void ConfigView::setAppletGlobalShortcut(const QString &shortcut)
{
    if (!d->applet || d->applet.data()->globalShortcut().toString().toLower() == shortcut.toLower()) {
        return;
    }

    d->applet.data()->setGlobalShortcut(shortcut);
    Q_EMIT appletGlobalShortcutChanged();
}

// To emulate Qt::WA_DeleteOnClose that QWindow doesn't have
void ConfigView::hideEvent(QHideEvent *ev)
{
    QQuickWindow::hideEvent(ev);
    deleteLater();
}

void ConfigView::resizeEvent(QResizeEvent *re)
{
    if (!d->rootItem) {
        return;
    }

    d->rootItem->setSize(re->size());

    if (d->applet) {
        KConfigGroup cg = d->applet.data()->config();
        cg = KConfigGroup(&cg, QStringLiteral("ConfigDialog"));
        cg.writeEntry("DialogWidth", re->size().width());
        cg.writeEntry("DialogHeight", re->size().height());
    }

    QQuickWindow::resizeEvent(re);
}

}

#include "moc_configview.cpp"
