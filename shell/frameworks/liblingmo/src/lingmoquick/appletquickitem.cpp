/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletquickitem.h"
#include "applet.h"
#include "appletcontext_p.h"
#include "appletquickitem_p.h"
#include "configview.h"
#include "containment.h"
#include "debug_p.h"
#include "lingmo_version.h"
#include "plasmoid/containmentitem.h"
#include "plasmoid/plasmoiditem.h"
#include "plasmoid/wallpaperitem.h"
#include "plasmoidattached_p.h"
#include "sharedqmlengine.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickWindow>
#include <QRandomGenerator>

#include <QDebug>

#include <KLocalizedString>

#include <Lingmo/Applet>
#include <Lingmo/Containment>
#include <Lingmo/Corona>
#include <qquickitem.h>

namespace LingmoQuick
{

QHash<Lingmo::Applet *, AppletQuickItem *> AppletQuickItemPrivate::s_itemsForApplet = QHash<Lingmo::Applet *, AppletQuickItem *>();
AppletQuickItemPrivate::PreloadPolicy AppletQuickItemPrivate::s_preloadPolicy = AppletQuickItemPrivate::Uninitialized;

AppletQuickItemPrivate::AppletQuickItemPrivate(AppletQuickItem *item)
    : q(item)
    , switchWidth(-1)
    , switchHeight(-1)
    , initComplete(false)
    , compactRepresentationCheckGuard(false)
{
    if (s_preloadPolicy == Uninitialized) {
        // default as Adaptive
        s_preloadPolicy = Adaptive;

        if (qEnvironmentVariableIsSet("LINGMO_PRELOAD_POLICY")) {
            const QString policy = QString::fromUtf8(qgetenv("LINGMO_PRELOAD_POLICY")).toLower();
            if (policy == QLatin1String("aggressive")) {
                s_preloadPolicy = Aggressive;
            } else if (policy == QLatin1String("none")) {
                s_preloadPolicy = None;
            }
        }

        qCInfo(LOG_LINGMOQUICK) << "Applet preload policy set to" << s_preloadPolicy;
    }
}

int AppletQuickItemPrivate::preloadWeight() const
{
    int defaultWeight;
    const QStringList provides = applet->pluginMetaData().value(QStringLiteral("X-Lingmo-Provides"), QStringList());

    // some applet types we want a bigger weight
    if (provides.contains(QLatin1String("org.kde.lingmo.launchermenu"))) {
        defaultWeight = DefaultLauncherPreloadWeight;
    } else {
        defaultWeight = DefaultPreloadWeight;
    }
    // default widgets to be barely preloaded
    return qBound(0,
                  applet->config().readEntry(QStringLiteral("PreloadWeight"),
                                             qMax(defaultWeight, applet->pluginMetaData().value(QStringLiteral("X-Lingmo-PreloadWeight"), 0))),
                  100);
}

QObject *AppletQuickItemPrivate::searchLayoutAttached(QObject *parent) const
{
    QObject *layout = nullptr;
    // Search a child that has the needed Layout properties
    // HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    const auto lstChildren = parent->children();
    for (QObject *child : lstChildren) {
        // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        /* clang-format off */
        if (child->property("minimumWidth").isValid()
            && child->property("minimumHeight").isValid()
            && child->property("preferredWidth").isValid()
            && child->property("preferredHeight").isValid()
            && child->property("maximumWidth").isValid()
            && child->property("maximumHeight").isValid()
            && child->property("fillWidth").isValid()
            && child->property("fillHeight").isValid()) { /* clang-format on */
            layout = child;
            break;
        }
    }
    return layout;
}

void AppletQuickItemPrivate::connectLayoutAttached(QObject *item)
{
    // Extract the representation's Layout, if any
    if (!item) {
        return;
    }

    QObject *layout = searchLayoutAttached(item);

    // if the compact repr doesn't export a Layout.* attached property,
    // reset our own with default values
    if (!layout) {
        if (ownLayout) {
            ownLayout->setProperty("minimumWidth", 0);
            ownLayout->setProperty("minimumHeight", 0);
            ownLayout->setProperty("preferredWidth", -1);
            ownLayout->setProperty("preferredHeight", -1);
            ownLayout->setProperty("maximumWidth", std::numeric_limits<qreal>::infinity());
            ownLayout->setProperty("maximumHeight", std::numeric_limits<qreal>::infinity());
            ownLayout->setProperty("fillWidth", false);
            ownLayout->setProperty("fillHeight", false);
        }
        return;
    }

    // propagate all the size hints
    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");

    QObject *newOwnLayout = searchLayoutAttached(q);

    // this should never happen, since we ask to create it if doesn't exists
    if (!newOwnLayout) {
        return;
    }

    // if the representation didn't change, don't do anything
    if (representationLayout == layout) {
        return;
    }

    if (representationLayout) {
        QObject::disconnect(representationLayout, nullptr, q, nullptr);
    }

    // Here we can't use the new connect syntax because we can't link against QtQuick layouts
    QObject::connect(layout, SIGNAL(minimumWidthChanged()), q, SLOT(minimumWidthChanged()));
    QObject::connect(layout, SIGNAL(minimumHeightChanged()), q, SLOT(minimumHeightChanged()));

    QObject::connect(layout, SIGNAL(preferredWidthChanged()), q, SLOT(preferredWidthChanged()));
    QObject::connect(layout, SIGNAL(preferredHeightChanged()), q, SLOT(preferredHeightChanged()));

    QObject::connect(layout, SIGNAL(maximumWidthChanged()), q, SLOT(maximumWidthChanged()));
    QObject::connect(layout, SIGNAL(maximumHeightChanged()), q, SLOT(maximumHeightChanged()));

    QObject::connect(layout, SIGNAL(fillWidthChanged()), q, SLOT(fillWidthChanged()));
    QObject::connect(layout, SIGNAL(fillHeightChanged()), q, SLOT(fillHeightChanged()));

    representationLayout = layout;
    ownLayout = newOwnLayout;

    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");
}

void AppletQuickItemPrivate::propagateSizeHint(const QByteArray &layoutProperty)
{
    if (ownLayout && representationLayout) {
        ownLayout->setProperty(layoutProperty.constData(), representationLayout->property(layoutProperty.constData()).toReal());
    }
}

QQuickItem *AppletQuickItemPrivate::createCompactRepresentationItem()
{
    if (!compactRepresentation) {
        return nullptr;
    }

    if (compactRepresentationItem) {
        return compactRepresentationItem;
    }

    QVariantHash initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);
    initialProperties[QStringLiteral("plasmoidItem")] = QVariant::fromValue(q);

    compactRepresentationItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(compactRepresentation, qmlContext(q), initialProperties));

    Q_EMIT q->compactRepresentationItemChanged(compactRepresentationItem);

    return compactRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createFullRepresentationItem()
{
    if (fullRepresentationItem) {
        return fullRepresentationItem;
    }

    if (fullRepresentation && fullRepresentation != qmlObject->mainComponent()) {
        QVariantHash initialProperties;
        initialProperties[QStringLiteral("parent")] = QVariant();
        fullRepresentationItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(fullRepresentation, qmlContext(q), initialProperties));
    }

    if (!fullRepresentationItem) {
        return nullptr;
    }

    Q_EMIT q->fullRepresentationItemChanged(fullRepresentationItem);

    return fullRepresentationItem;
}

QQuickItem *AppletQuickItemPrivate::createCompactRepresentationExpanderItem()
{
    if (!compactRepresentationExpander) {
        return nullptr;
    }

    if (compactRepresentationExpanderItem) {
        return compactRepresentationExpanderItem;
    }

    compactRepresentationExpanderItem = qobject_cast<QQuickItem *>(qmlObject->createObjectFromComponent(compactRepresentationExpander, qmlContext(q)));

    if (!compactRepresentationExpanderItem) {
        return nullptr;
    }

    compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject *>(createCompactRepresentationItem()));
    compactRepresentationExpanderItem->setProperty("plasmoidItem", QVariant::fromValue(q));

    return compactRepresentationExpanderItem;
}

bool AppletQuickItemPrivate::appletShouldBeExpanded() const
{
    if (applet->isContainment()) {
        return true;

    } else {
        if (!fullRepresentation) {
            // If a full representation wasn't specified, the onle and only representation of the plasmoid are our
            // direct contents, so we consider it always expanded
            return true;
        }
        if (switchWidth > 0 && switchHeight > 0) {
            // This code checks against the following edge case:
            // The compact representation preferred size is bigger than both the switch
            // size, and the full representation preferred size.
            // this can cause in the panel (when is quite big) an infinite resize loop, because
            // the applet size is bigger than the switch size, then it switches to full
            // representaiton that has a smaller hint. this causes a resize that will make it
            // switch to compact representation, making it grow again and switch again
            if (compactRepresentationItem && fullRepresentationItem) {
                QObject *compactLayout = searchLayoutAttached(compactRepresentationItem);
                QObject *fullLayout = searchLayoutAttached(fullRepresentationItem);
                if (compactLayout && fullLayout) {
                    QSizeF compactPreferred = {compactLayout->property("preferredWidth").toReal(), compactLayout->property("preferredHeight").toReal()};
                    QSizeF fullPreferred = {fullLayout->property("preferredWidth").toReal(), fullLayout->property("preferredHeight").toReal()};

                    if ((compactPreferred.width() > fullPreferred.width() && compactPreferred.width() > switchWidth) ||
                        (compactPreferred.height() > fullPreferred.height()  && compactPreferred.height() > switchHeight)) {
                        return false;
                    }
                }
            }
            return q->width() > switchWidth && q->height() > switchHeight;

            // if a size to switch wasn't set, determine what representation to always chose
        } else {
            // preferred representation set?
            if (preferredRepresentation) {
                return preferredRepresentation == fullRepresentation;
                // Otherwise, base on FormFactor
            } else {
                return (applet->formFactor() != Lingmo::Types::Horizontal && applet->formFactor() != Lingmo::Types::Vertical);
            }
        }
    }
}

void AppletQuickItemPrivate::preloadForExpansion()
{
    qint64 time = 0;
    if (QLoggingCategory::defaultCategory()->isInfoEnabled()) {
        time = QDateTime::currentMSecsSinceEpoch();
    }

    if (!createFullRepresentationItem()) {
        return;
    }

    // When not already expanded, also preload the expander
    if (!appletShouldBeExpanded() && !applet->isContainment() && (!preferredRepresentation || preferredRepresentation != fullRepresentation)) {
        createCompactRepresentationExpanderItem();
    }

    if (!appletShouldBeExpanded() && compactRepresentationExpanderItem) {
        compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant::fromValue<QObject *>(createFullRepresentationItem()));
    } else if (fullRepresentationItem) {
        fullRepresentationItem->setProperty("parent", QVariant::fromValue<QObject *>(q));
    }

    // preallocate nodes
    if (fullRepresentationItem && fullRepresentationItem->window()) {
        fullRepresentationItem->window()->create();
    }

    qCDebug(LOG_LINGMOQUICK) << "Applet" << applet->title() << "loaded after" << (QDateTime::currentMSecsSinceEpoch() - time) << "msec";
}

void AppletQuickItemPrivate::anchorsFillParent(QQuickItem *item, QQuickItem *parent)
{
    if (item->parentItem() != parent) {
        return;
    }
    // just set once, don't bind
    QQmlProperty::write(item, QStringLiteral("anchors.fill"), QVariant::fromValue<QObject *>(parent));
}

void AppletQuickItemPrivate::compactRepresentationCheck()
{
    if (!initComplete) {
        return;
    }

    // ignore 0 sizes;
    if (q->width() <= 0 || q->height() <= 0) {
        return;
    }

    // ignore if this widget is being checked somewhere above
    if (compactRepresentationCheckGuard) {
        return;
    }

    bool full = appletShouldBeExpanded();

    if ((full && fullRepresentationItem && fullRepresentationItem == currentRepresentationItem)
        || (!full && compactRepresentationItem && compactRepresentationItem == currentRepresentationItem)) {
        return;
    }

    compactRepresentationCheckGuard = true;

    // Expanded
    if (full) {
        QQuickItem *item = createFullRepresentationItem();

        if (item) {
            // unwire with the expander
            if (compactRepresentationExpanderItem) {
                compactRepresentationExpanderItem->setProperty("fullRepresentation", QVariant());
                compactRepresentationExpanderItem->setProperty("compactRepresentation", QVariant());
                compactRepresentationExpanderItem->setVisible(false);
            }

            const bool fullRepresentationWasVisible = fullRepresentationItem->parentItem() == q;

            // the fullrepresentation being the complete AppletItem is actually allowed when the main ui
            // is child of the root item (like many panel applets)
            if (item != q) {
                item->setParentItem(q);
                anchorsFillParent(item, q);
            }

            if (compactRepresentationItem) {
                compactRepresentationItem->setVisible(false);
            }

            currentRepresentationItem = item;
            connectLayoutAttached(item);

            if (!expanded && !fullRepresentationWasVisible) {
                expanded = true;
                Q_EMIT q->expandedChanged(true);
            }
        }

    } else {
        // Icon
        QQuickItem *compactItem = createCompactRepresentationItem();
        QQuickItem *compactExpanderItem = createCompactRepresentationExpanderItem();

        if (compactItem && compactExpanderItem) {
            // set the root item as the main visible item
            compactItem->setVisible(true);
            compactExpanderItem->setParentItem(q);
            compactExpanderItem->setVisible(true);
            anchorsFillParent(compactExpanderItem, q);

            // only reparent full representation to null if it was parented to the applet
            // if it was already in the expander, leave it where it is
            const bool fullRepresentationWasVisible = fullRepresentationItem && fullRepresentationItem->parentItem() == q;
            if (fullRepresentationItem && fullRepresentationWasVisible) {
                fullRepresentationItem->setProperty("parent", QVariant());
            }

            compactExpanderItem->setProperty("compactRepresentation", QVariant::fromValue<QObject *>(compactItem));
            // The actual full representation will be connected when created
            compactExpanderItem->setProperty("fullRepresentation", QVariant());

            currentRepresentationItem = compactItem;
            connectLayoutAttached(compactItem);

            // set Expanded to false only if the expanded cause was the full representation
            // in the applet item, not if the full representation was in the popup and the popup was open
            if (expanded && fullRepresentationWasVisible) {
                expanded = false;
                Q_EMIT q->expandedChanged(false);
            }
        }
    }

    compactRepresentationCheckGuard = false;
}

void AppletQuickItemPrivate::minimumWidthChanged()
{
    propagateSizeHint("minimumWidth");
}

void AppletQuickItemPrivate::minimumHeightChanged()
{
    propagateSizeHint("minimumHeight");
}

void AppletQuickItemPrivate::preferredWidthChanged()
{
    propagateSizeHint("preferredWidth");
}

void AppletQuickItemPrivate::preferredHeightChanged()
{
    propagateSizeHint("preferredHeight");
}

void AppletQuickItemPrivate::maximumWidthChanged()
{
    propagateSizeHint("maximumWidth");
}

void AppletQuickItemPrivate::maximumHeightChanged()
{
    propagateSizeHint("maximumHeight");
}

void AppletQuickItemPrivate::fillWidthChanged()
{
    propagateSizeHint("fillWidth");
}

void AppletQuickItemPrivate::fillHeightChanged()
{
    propagateSizeHint("fillHeight");
}

AppletQuickItem::AppletQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
    , d(new AppletQuickItemPrivate(this))
{
}

AppletQuickItem::~AppletQuickItem()
{
    AppletQuickItemPrivate::s_itemsForApplet.remove(d->applet);
    // decrease weight
    if (d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive) {
        d->applet->config().writeEntry(QStringLiteral("PreloadWeight"), qMax(0, d->preloadWeight() - AppletQuickItemPrivate::PreloadWeightDecrement));
    }

    // Here the order is important
    delete d->compactRepresentationItem;
    delete d->fullRepresentationItem;
    delete d->compactRepresentationExpanderItem;
    delete d;
}

bool AppletQuickItem::hasItemForApplet(Lingmo::Applet *applet)
{
    return AppletQuickItemPrivate::s_itemsForApplet.contains(applet);
}

AppletQuickItem *AppletQuickItem::itemForApplet(Lingmo::Applet *applet)
{
    if (!applet) {
        return nullptr;
    }

    // TODO: move somewhere else? in lingmocore import?
    if (AppletQuickItemPrivate::s_itemsForApplet.isEmpty()) {
        const char *uri = "org.kde.lingmo.plasmoid";
        qmlRegisterExtendedType<Lingmo::Applet, PlasmoidAttached>(uri, 2, 0, "Plasmoid");
        qmlRegisterExtendedType<Lingmo::Containment, ContainmentAttached>(uri, 2, 0, "Containment");

        qmlRegisterType<PlasmoidItem>(uri, 2, 0, "PlasmoidItem");
        qmlRegisterType<ContainmentItem>(uri, 2, 0, "ContainmentItem");
        qmlRegisterType<WallpaperItem>(uri, 2, 0, "WallpaperItem");
        qmlRegisterAnonymousType<Lingmo::Corona>("org.kde.lingmo.plasmoid", 1);
    }
    auto it = AppletQuickItemPrivate::s_itemsForApplet.constFind(applet);
    if (it != AppletQuickItemPrivate::s_itemsForApplet.constEnd()) {
        return it.value();
    }

    // Don't try to create applet items when the app is closing
    if (qApp->closingDown() || applet->destroyed()) {
        return nullptr;
    }

    Lingmo::Containment *pc = qobject_cast<Lingmo::Containment *>(applet);
    auto *qmlObject = new LingmoQuick::SharedQmlEngine(applet, applet);
    qmlObject->engine()->setProperty("_lingmouiTheme", QStringLiteral("LingmoUILingmoStyle"));
    qmlObject->setInitializationDelayed(true);
    qmlObject->setTranslationDomain(applet->translationDomain());

    AppletQuickItem *item = nullptr;
    qmlObject->setSource(applet->mainScript());
    if (pc && pc->isContainment()) {
        item = qobject_cast<ContainmentItem *>(qmlObject->rootObject());
        if (!item && qmlObject->mainComponent() && !qmlObject->mainComponent()->isError()) {
            applet->setLaunchErrorMessage(i18n("The root item of %1 must be of type ContainmentItem", applet->mainScript().toString()));
        }
    } else {
        item = qobject_cast<PlasmoidItem *>(qmlObject->rootObject());
        if (!item && qmlObject->mainComponent() && !qmlObject->mainComponent()->isError()) {
            applet->setLaunchErrorMessage(i18n("The root item of %1 must be of type PlasmoidItem", applet->mainScript().toString()));
        }
    }

    if (!item || !qmlObject->mainComponent() || qmlObject->mainComponent()->isError() || applet->failedToLaunch()) {
        QString reason;
        QString compactReason;
        QJsonObject errorData;
        errorData[QStringLiteral("appletName")] = i18n("Unknown Applet");
        errorData[QStringLiteral("isDebugMode")] = qEnvironmentVariableIntValue("LINGMO_ENABLE_QML_DEBUG") != 0;

        if (applet->sourceValid()) {
            const QString versionString = applet->pluginMetaData().value(QStringLiteral("X-Lingmo-API-Minimum-Version"));
            QVersionNumber version;
            if (!versionString.isEmpty()) {
                version = QVersionNumber::fromString(versionString);
            }

            bool versionMismatch = false;
            const int lingmo_version_major = 6; // TODO: as soon LINGMO_VERSION_MAJOR is actually 6, use directly that
            if (version.isNull()) {
                reason = i18n(
                    "This Widget was written for an unknown older version of Lingmo and is not compatible with Lingmo %1. Please contact the widget's author for "
                    "an updated version.",
                    lingmo_version_major);
                compactReason = i18n("%1 is not compatible with Lingmo %2", applet->pluginMetaData().name(), lingmo_version_major);
                versionMismatch = true;
            } else if (version.majorVersion() < lingmo_version_major) {
                reason =
                    i18n("This Widget was written for Lingmo %1 and is not compatible with Lingmo %2. Please contact the widget's author for an updated version.",
                        version.majorVersion(),
                        lingmo_version_major);
                compactReason = i18n("%1 is not compatible with Lingmo %2", applet->pluginMetaData().name(), lingmo_version_major);
                versionMismatch = true;
            } else if (version.majorVersion() > lingmo_version_major || version.minorVersion() > LINGMO_VERSION_MINOR) {
                reason = i18n("This Widget was written for Lingmo %1 and is not compatible with Lingmo %2. Please update Lingmo in order to use the widget.",
                            versionString,
                            lingmo_version_major);
                compactReason = i18n("%1 is not compatible with Lingmo %2", applet->pluginMetaData().name(), lingmo_version_major);
                versionMismatch = true;
            } else if (applet->failedToLaunch()) {
                reason = applet->launchErrorMessage();
                compactReason = reason;
            } else {
                compactReason = i18n("Sorry! There was an error loading %1.", applet->pluginMetaData().name());
            }
            errorData[QStringLiteral("errors")] = QJsonArray::fromStringList({reason});
            if (compactReason != QString()) {
                errorData[QStringLiteral("compactError")] = compactReason;
            }

            if (!versionMismatch) {
                const auto errors = qmlObject->mainComponent()->errors();
                QStringList errorList;
                for (const QQmlError &error : errors) {
                    reason += error.toString() + QLatin1Char('\n');
                    errorList << error.toString();
                }
                errorData[QStringLiteral("errors")] = QJsonArray::fromStringList(errorList);
            }
            errorData[QStringLiteral("appletName")] = applet->pluginMetaData().name();
            reason += i18n("Error loading QML file: %1 %2", qmlObject->mainComponent()->url().toString(), reason);
        } else {
            const auto pluginId = applet->pluginMetaData().pluginId();
            reason = i18n("Error loading Applet: package %1 does not exist.", pluginId);
            errorData[QStringLiteral("errors")] = QJsonArray::fromStringList({reason});
            compactReason = i18n("Sorry! There was an error loading %1.", pluginId);
            errorData[QStringLiteral("compactError")] = compactReason;
        }

        qCWarning(LOG_LINGMOQUICK) << "error when loading applet" << applet->pluginMetaData().pluginId()
                                   << errorData[QStringLiteral("errors")].toVariant().toStringList();

        qmlObject->setSource(applet->containment()->corona()->kPackage().fileUrl("appleterror"));

        applet->setHasConfigurationInterface(false);
        // even the error message QML may fail
        if (qmlObject->mainComponent()->isError()) {
            return nullptr;
        }

        item = qobject_cast<PlasmoidItem *>(qmlObject->rootObject());

        applet->setLaunchErrorMessage(reason);
        if (item) {
            item->setProperty("errorInformation", errorData);
        } else {
            // In this case the error message loaded correctly, but was not a PlasmoidItem, bail out
            qCWarning(LOG_LINGMOQUICK) << "Applet Error message is not of type PlasmoidItem"
                                       << applet->containment()->corona()->kPackage().fileUrl("appleterror");
            return nullptr;
        }
    }

    AppletQuickItemPrivate::s_itemsForApplet[applet] = item;
    qmlObject->setInitializationDelayed(false);
    qmlObject->completeInitialization();

    // A normal applet has UI ready as soon as is loaded, a containment, only when also the wallpaper is loaded
    if (!pc || !pc->isContainment()) {
        applet->updateConstraints(Lingmo::Applet::UiReadyConstraint);
        applet->flushPendingConstraintsEvents();
    }

    item->setProperty("_lingmo_applet", QVariant::fromValue(applet));
    item->d->applet = applet;
    item->d->qmlObject = qmlObject;

    if (!qEnvironmentVariableIntValue("LINGMO_NO_CONTEXTPROPERTIES")) {
        qmlObject->rootContext()->setContextProperty(QStringLiteral("plasmoid"), applet);
    }

    QObject::connect(applet, &Lingmo::Applet::appletDeleted, item, [qmlObject](Lingmo::Applet *applet) {
        // Deleting qmlObject will also delete the instantiated plasmoidItem
        // deleteing just the plasmoiditem will cause a double deletion when qmlObject
        // gets deleted by applet deletion
        if (qmlObject->parent() == applet) {
            // appletDelete can also be emitted by a containment for one of its children
            delete qmlObject;
            AppletQuickItemPrivate::s_itemsForApplet.remove(applet);
        }
    });

    applet->setProperty("_plasmoid", QVariant::fromValue(item));
    return item;
}

Lingmo::Applet *AppletQuickItem::applet() const
{
    return d->applet;
}

void AppletQuickItem::init()
{
    if (!d->applet) {
        // This can happen only if the client QML code declares a PlasmoidItem somewhere else than the root object
        return;
    }
    if (d->initComplete) {
        return;
    }

    if (d->applet->containment()) {
        if (d->applet->containment()->corona()) {
            d->coronaPackage = d->applet->containment()->corona()->kPackage();
        }
    }

    // Initialize the main QML file
    QQmlEngine *engine = d->qmlObject->engine().get();

    // If no fullRepresentation was defined, we won't create compact and expander either.
    // The only representation available are whatever items defined directly inside PlasmoidItem {}
    // default compactRepresentation is a simple icon provided by the shell package
    if (!d->compactRepresentation && d->fullRepresentation) {
        d->compactRepresentation = new QQmlComponent(engine, this);
        d->compactRepresentation->loadUrl(d->coronaPackage.fileUrl("defaultcompactrepresentation"));
        Q_EMIT compactRepresentationChanged(d->compactRepresentation);
    }

    // default compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!d->compactRepresentationExpander && d->fullRepresentation) {
        d->compactRepresentationExpander = new QQmlComponent(engine, this);
        QUrl compactExpanderUrl = d->applet->containment()->compactApplet();
        if (compactExpanderUrl.isEmpty()) {
            compactExpanderUrl = d->coronaPackage.fileUrl("compactapplet");
        }

        d->compactRepresentationExpander->loadUrl(compactExpanderUrl);
    }

    d->initComplete = true;
    d->compactRepresentationCheck();
    qmlObject()->engine()->rootContext()->setBaseUrl(qmlObject()->source());

    // if we're expanded we don't care about preloading because it will already be the case
    // as well as for containments
    if (d->applet->isContainment() || d->expanded || d->preferredRepresentation == d->fullRepresentation) {
        return;
    }

    if (!d->applet->isContainment() && d->applet->containment()) {
        connect(d->applet->containment(), &Lingmo::Containment::uiReadyChanged, this, [this](bool uiReady) {
            if (uiReady && d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive) {
                const int preloadWeight = d->preloadWeight();
                qCDebug(LOG_LINGMOQUICK) << "New Applet " << d->applet->title() << "with a weight of" << preloadWeight;

                // don't preload applets less then a certain weight
                if (d->s_preloadPolicy >= AppletQuickItemPrivate::Aggressive || preloadWeight >= AppletQuickItemPrivate::DelayedPreloadWeight) {
                    // spread the creation over a random delay to make it look
                    // lingmo started already, and load the popup in the background
                    // without big noticeable freezes, the bigger the weight the smaller is likely
                    // to be the delay, smaller minimum walue, smaller spread
                    const int min = (100 - preloadWeight) * 20;
                    const int max = (100 - preloadWeight) * 100;
                    const int delay = QRandomGenerator::global()->bounded((max + 1) - min) + min;
                    QTimer::singleShot(delay, this, [this, delay]() {
                        qCDebug(LOG_LINGMOQUICK) << "Delayed preload of " << d->applet->title() << "after" << (qreal)delay / 1000 << "seconds";
                        d->preloadForExpansion();
                    });
                }
            }
        });
    }
}

void AppletQuickItem::classBegin()
{
    QQuickItem::classBegin();
    AppletContext *ac = qobject_cast<AppletContext *>(QQmlEngine::contextForObject(this)->parentContext());
    if (!ac) {
        qCWarning(LOG_LINGMOQUICK) << "Detected a PlasmoidItem which is not the root QML item: this is not supported.";
        return;
    }
    d->applet = ac->applet();
    d->qmlObject = ac->sharedQmlEngine();
}

void AppletQuickItem::componentComplete()
{
    QQuickItem::componentComplete();
    init();
}

int AppletQuickItem::switchWidth() const
{
    return d->switchWidth;
}

void AppletQuickItem::setSwitchWidth(int width)
{
    if (d->switchWidth == width) {
        return;
    }

    d->switchWidth = width;
    d->compactRepresentationCheck();
    Q_EMIT switchWidthChanged(width);
}

int AppletQuickItem::switchHeight() const
{
    return d->switchHeight;
}

void AppletQuickItem::setSwitchHeight(int height)
{
    if (d->switchHeight == height) {
        return;
    }

    d->switchHeight = height;
    d->compactRepresentationCheck();
    Q_EMIT switchHeightChanged(height);
}

QQmlComponent *AppletQuickItem::compactRepresentation()
{
    return d->compactRepresentation;
}

void AppletQuickItem::setCompactRepresentation(QQmlComponent *component)
{
    if (d->compactRepresentation == component) {
        return;
    }

    d->compactRepresentation = component;
    Q_EMIT compactRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::fullRepresentation()
{
    return d->fullRepresentation;
}

void AppletQuickItem::setFullRepresentation(QQmlComponent *component)
{
    if (d->fullRepresentation == component) {
        return;
    }

    d->fullRepresentation = component;
    Q_EMIT fullRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::preferredRepresentation()
{
    return d->preferredRepresentation;
}

void AppletQuickItem::setPreferredRepresentation(QQmlComponent *component)
{
    if (d->preferredRepresentation == component) {
        return;
    }

    d->preferredRepresentation = component;
    Q_EMIT preferredRepresentationChanged(component);
    d->compactRepresentationCheck();
}

bool AppletQuickItem::isExpanded() const
{
    return d->applet->isContainment() || !d->fullRepresentation || d->expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (d->expanded == expanded) {
        return;
    }

    if (expanded) {
        d->preloadForExpansion();
        // increase on open, ignore containments
        if (d->s_preloadPolicy >= AppletQuickItemPrivate::Adaptive && !d->applet->isContainment()) {
            const int newWeight = qMin(d->preloadWeight() + AppletQuickItemPrivate::PreloadWeightIncrement, 100);
            d->applet->config().writeEntry(QStringLiteral("PreloadWeight"), newWeight);
            qCDebug(LOG_LINGMOQUICK) << "Increasing score for" << d->applet->title() << "to" << newWeight;
        }
    }

    d->expanded = expanded;

    Q_EMIT expandedChanged(expanded);
}

bool AppletQuickItem::isActivationTogglesExpanded() const
{
    return d->activationTogglesExpanded;
}

void AppletQuickItem::setActivationTogglesExpanded(bool activationTogglesExpanded)
{
    if (d->activationTogglesExpanded == activationTogglesExpanded) {
        return;
    }
    d->activationTogglesExpanded = activationTogglesExpanded;
    Q_EMIT activationTogglesExpandedChanged(activationTogglesExpanded);
}

bool AppletQuickItem::hideOnWindowDeactivate() const
{
    return d->hideOnWindowDeactivate;
}

void AppletQuickItem::setHideOnWindowDeactivate(bool hide)
{
    if (d->hideOnWindowDeactivate == hide) {
        return;
    }
    d->hideOnWindowDeactivate = hide;
    Q_EMIT hideOnWindowDeactivateChanged(hide);
}

bool AppletQuickItem::preloadFullRepresentation() const
{
    return d->preloadFullRepresentation;
}

void AppletQuickItem::setPreloadFullRepresentation(bool preload)
{
    if (d->preloadFullRepresentation == preload) {
        return;
    }

    d->preloadFullRepresentation = preload;
    d->createFullRepresentationItem();

    Q_EMIT preloadFullRepresentationChanged(preload);
}

////////////Internals

LingmoQuick::SharedQmlEngine *AppletQuickItem::qmlObject()
{
    return d->qmlObject;
}

QQuickItem *AppletQuickItem::compactRepresentationItem()
{
    return d->compactRepresentationItem;
}

QQuickItem *AppletQuickItem::fullRepresentationItem()
{
    return d->fullRepresentationItem;
}

void AppletQuickItem::childEvent(QChildEvent *event)
{
    // Added child may be QQuickLayoutAttached
    if (event->added() && !d->ownLayout && d->currentRepresentationItem) {
        // Child has not yet finished initialization at this point
        QTimer::singleShot(0, this, [this]() {
            if (!d->ownLayout) {
                d->connectLayoutAttached(d->currentRepresentationItem);
            }
        });
    }

    QQuickItem::childEvent(event);
}

void AppletQuickItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    d->compactRepresentationCheck();
}
}

#include "moc_appletquickitem.cpp"
