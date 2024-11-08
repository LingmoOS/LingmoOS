/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "applet.h"
#include "private/applet_p.h"

#include "config-lingmo.h"

#include <QDebug>
#include <QFile>
#include <QJSEngine>
#include <QList>
#include <QMetaEnum>

#include <KAuthorized>
#include <KConfigLoader>
#include <KConfigPropertyMap>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPackage/Package>

#include "containment.h"
#include "corona.h"
#include "lingmo.h"
#include "pluginloader.h"

#include "debug_p.h"
#include "private/containment_p.h"

#include <cmath>
#include <limits>

namespace Lingmo
{
Applet::Applet(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args)
    : QObject(parentObject)
    , d(new AppletPrivate(data, args.count() > 1 ? args[1].toInt() : 0, this))
{
    if (!args.isEmpty()) {
        const QVariant first = args.first();
        if (first.canConvert<KPackage::Package>()) {
            d->package = first.value<KPackage::Package>();
        }
    }
    d->icon = d->appletDescription.iconName();

    if (args.contains(QVariant::fromValue(QStringLiteral("org.kde.lingmo:force-create")))) {
        setProperty("org.kde.lingmo:force-create", true);
    }

    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    d->init(args.mid(2));
}

Applet::~Applet()
{
    for (QAction *a : d->actions.values()) {
        disconnect(a, nullptr, this, nullptr);
    }
    for (QAction *a : d->contextualActions) {
        disconnect(a, nullptr, this, nullptr);
    }

    // let people know that i will die
    Q_EMIT appletDeleted(this);

    if (d->transient) {
        d->resetConfigurationObject();
    }

    // ConfigLoader is deleted when AppletPrivate closes not Applet
    // It saves on closure and emits a signal.
    // disconnect early to avoid a crash. See  411221
    if (d->configLoader) {
        disconnect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
    }
    delete d;
}

void Applet::init()
{
    // Don't implement anything here, it will be overridden by subclasses
}

uint Applet::id() const
{
    return d->appletId;
}

QVariantList Applet::startupArguments() const
{
    return d->startupArguments;
}

void Applet::save(KConfigGroup &g) const
{
    if (d->transient || !d->appletDescription.isValid()) {
        return;
    }

    KConfigGroup group = g;
    if (!group.isValid()) {
        group = *d->mainConfigGroup();
    }

    // qCDebug(LOG_LINGMO) << "saving" << pluginName() << "to" << group.name();
    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    group.writeEntry("immutability", (int)d->immutability);
    group.writeEntry("plugin", d->appletDescription.pluginId());

    if (!d->started) {
        return;
    }

    KConfigGroup appletConfigGroup(&group, QStringLiteral("Configuration"));
    saveState(appletConfigGroup);

    if (d->configLoader) {
        // we're saving so we know its changed, we don't need or want the configChanged
        // signal bubbling up at this point due to that
        disconnect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
        d->configLoader->save();
        connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
    }
}

void Applet::restore(KConfigGroup &group)
{
    setImmutability((Types::ImmutabilityType)group.readEntry("immutability", (int)Types::Mutable));

    KConfigGroup shortcutConfig(&group, QStringLiteral("Shortcuts"));
    QString shortcutText = shortcutConfig.readEntryUntranslated("global", QString());
    if (!shortcutText.isEmpty()) {
        setGlobalShortcut(QKeySequence(shortcutText));
        /*
        #ifndef NDEBUG
        // qCDebug(LOG_LINGMO) << "got global shortcut for" << name() << "of" << QKeySequence(shortcutText);
        #endif
        #ifndef NDEBUG
        // qCDebug(LOG_LINGMO) << "set to" << d->activationAction->objectName()
        #endif
                 << d->activationAction->globalShortcut().primary();
                 */
    }

    // User background hints
    // TODO support flags in the config
    QByteArray hintsString = config().readEntry("UserBackgroundHints", QString()).toUtf8();
    QMetaEnum hintEnum = QMetaEnum::fromType<Lingmo::Types::BackgroundHints>();
    bool ok;
    int value = hintEnum.keyToValue(hintsString.constData(), &ok);
    if (ok) {
        d->userBackgroundHints = Lingmo::Types::BackgroundHints(value);
        d->userBackgroundHintsInitialized = true;
        Q_EMIT userBackgroundHintsChanged();
        if (d->backgroundHints & Lingmo::Types::ConfigurableBackground) {
            Q_EMIT effectiveBackgroundHintsChanged();
        }
    }
}

void Applet::setLaunchErrorMessage(const QString &message)
{
    if (message == d->launchErrorMessage) {
        return;
    }

    d->failed = true;
    d->launchErrorMessage = message;
}

void Applet::saveState(KConfigGroup &group) const
{
    if (group.config()->name() != config().config()->name()) {
        // we're being saved to a different file!
        // let's just copy the current values in our configuration over
        KConfigGroup c = config();
        c.copyTo(&group);
    }
}

KConfigGroup Applet::config() const
{
    if (d->transient) {
        return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("LingmoTransientsConfig"));
    }

    if (isContainment()) {
        return *(d->mainConfigGroup());
    }

    return KConfigGroup(d->mainConfigGroup(), QStringLiteral("Configuration"));
}

KConfigGroup Applet::globalConfig() const
{
    KConfigGroup globalAppletConfig;
    QString group = isContainment() ? QStringLiteral("ContainmentGlobals") : QStringLiteral("AppletGlobals");

    Containment *cont = containment();
    Corona *corona = nullptr;
    if (cont) {
        corona = cont->corona();
    }
    if (corona) {
        KSharedConfig::Ptr coronaConfig = corona->config();
        globalAppletConfig = KConfigGroup(coronaConfig, group);
    } else {
        globalAppletConfig = KConfigGroup(KSharedConfig::openConfig(), group);
    }

    return KConfigGroup(&globalAppletConfig, d->globalName());
}

void Applet::destroy()
{
    if (immutability() != Types::Mutable || d->transient || !d->started) {
        return; // don't double delete
    }

    d->setDestroyed(true);
    // FIXME: an animation on leave if !isContainment() would be good again .. which should be handled by the containment class
    d->cleanUpAndDelete();
}

bool Applet::destroyed() const
{
    return d->transient;
}

KConfigLoader *Applet::configScheme() const
{
    if (!d->configLoader) {
        const QString xmlPath = d->package.isValid() ? d->package.filePath("mainconfigxml") : QString();
        KConfigGroup cfg = config();
        if (xmlPath.isEmpty()) {
            d->configLoader = new KConfigLoader(cfg, nullptr);
        } else {
            QFile file(xmlPath);
            d->configLoader = new KConfigLoader(cfg, &file);
            QObject::connect(d->configLoader, SIGNAL(configChanged()), this, SLOT(propagateConfigChanged()));
        }
    }

    return d->configLoader;
}

KConfigPropertyMap *Applet::configuration()
{
    if (!d->configPropertyMap) {
        d->configPropertyMap = new KConfigPropertyMap(configScheme(), this);
        connect(d->configPropertyMap, &KConfigPropertyMap::valueChanged, this, [this]() {
            d->scheduleModificationNotification();
        });
    }
    return d->configPropertyMap;
}

void Applet::updateConstraints(Constraints constraints)
{
    d->scheduleConstraintsUpdate(constraints);
}

void Applet::constraintsEvent(Constraints constraints)
{
    // NOTE: do NOT put any code in here that reacts to constraints updates
    //      as it will not get called for any applet that reimplements constraintsEvent
    //      without calling the Applet:: version as well, which it shouldn't need to.
    //      INSTEAD put such code into flushPendingConstraintsEvents
    Q_UNUSED(constraints)
    // qCDebug(LOG_LINGMO) << constraints << "constraints are FormFactor: " << formFactor()
    //         << ", Location: " << location();
}

QString Applet::title() const
{
    if (!d->customTitle.isEmpty()) {
        return d->customTitle;
    }

    if (d->appletDescription.isValid()) {
        return d->appletDescription.name();
    }

    return i18n("Unknown");
}

void Applet::setTitle(const QString &title)
{
    if (title == d->customTitle) {
        return;
    }

    d->customTitle = title;
    Q_EMIT titleChanged(title);
}

QString Applet::icon() const
{
    return d->icon;
}

void Applet::setIcon(const QString &icon)
{
    if (icon == d->icon) {
        return;
    }

    d->icon = icon;
    Q_EMIT iconChanged(icon);
}

bool Applet::isBusy() const
{
    return d->busy;
}

void Applet::setBusy(bool busy)
{
    if (busy == d->busy) {
        return;
    }

    d->busy = busy;
    Q_EMIT busyChanged(busy);
}

Lingmo::Types::BackgroundHints Applet::backgroundHints() const
{
    return d->backgroundHints;
}

void Applet::setBackgroundHints(Lingmo::Types::BackgroundHints hint)
{
    if (d->backgroundHints == hint) {
        return;
    }

    Lingmo::Types::BackgroundHints oldeffectiveHints = effectiveBackgroundHints();

    d->backgroundHints = hint;
    Q_EMIT backgroundHintsChanged();

    if (oldeffectiveHints != effectiveBackgroundHints()) {
        Q_EMIT effectiveBackgroundHintsChanged();
    }
}

Lingmo::Types::BackgroundHints Applet::effectiveBackgroundHints() const
{
    if (d->userBackgroundHintsInitialized && (d->backgroundHints & Lingmo::Types::ConfigurableBackground)) {
        return d->userBackgroundHints;
    } else {
        return d->backgroundHints;
    }
}

Lingmo::Types::BackgroundHints Applet::userBackgroundHints() const
{
    return d->userBackgroundHints;
}

void Applet::setUserBackgroundHints(Lingmo::Types::BackgroundHints hint)
{
    if (d->userBackgroundHints == hint && d->userBackgroundHintsInitialized) {
        return;
    }

    d->userBackgroundHints = hint;
    d->userBackgroundHintsInitialized = true;
    QMetaEnum hintEnum = QMetaEnum::fromType<Lingmo::Types::BackgroundHints>();
    config().writeEntry("UserBackgroundHints", hintEnum.valueToKey(d->userBackgroundHints));
    if (containment() && containment()->corona()) {
        containment()->corona()->requestConfigSync();
    }

    Q_EMIT userBackgroundHintsChanged();

    if (d->backgroundHints & Lingmo::Types::ConfigurableBackground) {
        Q_EMIT effectiveBackgroundHintsChanged();
    }
}

KPluginMetaData Applet::pluginMetaData() const
{
    return d->appletDescription;
}

QString Applet::pluginName() const
{
    return d->appletDescription.isValid() ? d->appletDescription.pluginId() : QString();
}

Types::ImmutabilityType Applet::immutability() const
{
    // if this object is itself system immutable, then just return that; it's the most
    // restrictive setting possible and will override anything that might be happening above it
    // in the Corona->Containment->Applet hierarchy
    if (d->transient || (d->mainConfig && d->mainConfig->isImmutable())) {
        return Types::SystemImmutable;
    }

    // Returning the more strict immutability between the applet immutability, Containment and Corona
    Types::ImmutabilityType upperImmutability = Types::Mutable;

    if (isContainment()) {
        Corona *cor = static_cast<Containment *>(const_cast<Applet *>(this))->corona();
        if (cor) {
            upperImmutability = cor->immutability();
        }
    } else {
        const Containment *cont = containment();
        if (cont) {
            if (cont->corona()) {
                upperImmutability = cont->corona()->immutability();
            } else {
                upperImmutability = cont->immutability();
            }
        }
    }

    if (upperImmutability != Types::Mutable) {
        // it's either system or user immutable, and we already check for local system immutability,
        // so upperImmutability is guaranteed to be as or more severe as this object's immutability
        return upperImmutability;
    } else {
        return d->immutability;
    }
}

void Applet::setImmutability(const Types::ImmutabilityType immutable)
{
    if (d->immutability == immutable || immutable == Types::SystemImmutable) {
        // we do not store system immutability in d->immutability since that gets saved
        // out to the config file; instead, we check with
        // the config group itself for this information at all times. this differs from
        // corona, where SystemImmutability is stored in d->immutability.
        return;
    }

    d->immutability = immutable;
    updateConstraints(ImmutableConstraint);
}

bool Applet::immutable() const
{
    return immutability() != Types::Mutable;
}

QString Applet::launchErrorMessage() const
{
    return d->launchErrorMessage;
}

bool Applet::failedToLaunch() const
{
    return d->failed;
}

bool Applet::configurationRequired() const
{
    return d->needsConfig;
}

QString Applet::configurationRequiredReason() const
{
    return d->configurationRequiredReason;
}

void Applet::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    if (d->needsConfig == needsConfig && reason == d->configurationRequiredReason) {
        return;
    }

    d->needsConfig = needsConfig;
    d->configurationRequiredReason = reason;

    Q_EMIT configurationRequiredChanged(needsConfig, reason);
}

void Applet::setConstraintHints(ConstraintHints constraintHints)
{
    if (d->constraintHints == constraintHints) {
        return;
    }

    d->constraintHints = constraintHints;
    Q_EMIT constraintHintsChanged(constraintHints);
}

Applet::ConstraintHints Applet::constraintHints() const
{
    return d->constraintHints;
}

bool Applet::isUserConfiguring() const
{
    return d->userConfiguring;
}

void Applet::setUserConfiguring(bool configuring)
{
    if (configuring == d->userConfiguring) {
        return;
    }

    d->userConfiguring = configuring;
    Q_EMIT userConfiguringChanged(configuring);
}

Types::ItemStatus Applet::status() const
{
    return d->itemStatus;
}

void Applet::setStatus(const Types::ItemStatus status)
{
    if (status == d->itemStatus) {
        return;
    }
    d->itemStatus = status;
    Q_EMIT statusChanged(status);
}

void Applet::flushPendingConstraintsEvents()
{
    if (d->pendingConstraints == NoConstraint) {
        return;
    }

    if (d->constraintsTimer.isActive()) {
        d->constraintsTimer.stop();
    }

    // qCDebug(LOG_LINGMO) << "flushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Constraints c = d->pendingConstraints;
    d->pendingConstraints = NoConstraint;

    if (c & UiReadyConstraint) {
        d->setUiReady();
    }

    if (c & StartupCompletedConstraint) {
        // common actions
        bool unlocked = immutability() == Types::Mutable;
        QAction *closeApplet = d->actions.value(QStringLiteral("remove"));
        if (closeApplet) {
            closeApplet->setEnabled(unlocked);
            closeApplet->setVisible(unlocked);
            connect(closeApplet, SIGNAL(triggered(bool)), this, SLOT(askDestroy()), Qt::UniqueConnection);
        }

        QAction *configAction = d->actions.value(QStringLiteral("configure"));
        if (configAction) {
            if (d->hasConfigurationInterface) {
                bool canConfig = unlocked || KAuthorized::authorize(QStringLiteral("lingmo/allow_configure_when_locked"));
                configAction->setVisible(canConfig);
                configAction->setEnabled(canConfig);
            }
        }
    }

    if (c & ImmutableConstraint) {
        bool unlocked = immutability() == Types::Mutable;
        QAction *action = d->actions.value(QStringLiteral("remove"));
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }

        action = d->actions.value(QStringLiteral("configure"));
        if (action && d->hasConfigurationInterface) {
            bool canConfig = unlocked || KAuthorized::authorize(QStringLiteral("lingmo/allow_configure_when_locked"));
            action->setVisible(canConfig);
            action->setEnabled(canConfig);
        }

        // an immutable constraint will always happen at startup
        // make sure don't emit a change signal for nothing
        if (d->oldImmutability != immutability()) {
            Q_EMIT immutabilityChanged(immutability());
        }
        d->oldImmutability = immutability();
    }

    // now take care of constraints in special subclass: Containment
    Containment *containment = qobject_cast<Lingmo::Containment *>(this);
    if (containment) {
        containment->d->containmentConstraintsEvent(c);
    }

    // pass the constraint on to the actual subclass
    constraintsEvent(c);

    if (c & StartupCompletedConstraint) {
        // start up is done, we can now go do a mod timer
        if (d->modificationsTimer) {
            if (d->modificationsTimer->isActive()) {
                d->modificationsTimer->stop();
            }
        } else {
            d->modificationsTimer = new QBasicTimer;
        }
    }

    if (c & FormFactorConstraint) {
        Q_EMIT formFactorChanged(formFactor());
    }

    if (c & LocationConstraint) {
        Q_EMIT locationChanged(location());
    }
}

QList<QAction *> Applet::contextualActions()
{
    return d->contextualActions;
}

QQmlListProperty<QAction> Applet::qmlContextualActions()
{
    return QQmlListProperty<QAction>(this,
                                     nullptr,
                                     AppletPrivate::contextualActions_append,
                                     AppletPrivate::contextualActions_count,
                                     AppletPrivate::contextualActions_at,
                                     AppletPrivate::contextualActions_clear,
                                     AppletPrivate::contextualActions_replace,
                                     AppletPrivate::contextualActions_removeLast);
}

void Applet::setInternalAction(const QString &name, QAction *action)
{
    if (name.isEmpty()) {
        return;
    }

    action->setObjectName(name);
    QAction *oldAction = d->actions.value(name);
    if (oldAction && QJSEngine::objectOwnership(oldAction) == QJSEngine::CppOwnership) {
        delete oldAction;
    }

    d->actions[name] = action;

    QObject::connect(action, &QObject::destroyed, this, [this, name]() {
        d->actions.remove(name);
        Q_EMIT internalActionsChanged(d->actions.values());
    });

    Q_EMIT internalActionsChanged(d->actions.values());
}

QAction *Applet::internalAction(const QString &name) const
{
    return d->actions.value(name);
}

void Applet::removeInternalAction(const QString &name)
{
    QAction *action = d->actions.value(name);

    if (action && QJSEngine::objectOwnership(action) == QJSEngine::CppOwnership) {
        disconnect(action, &QObject::destroyed, this, nullptr); // Avoid emitting signal again
        delete action;
    }

    d->actions.remove(name);

    Q_EMIT internalActionsChanged(d->actions.values());
}

QList<QAction *> Applet::internalActions() const
{
    return d->actions.values();
}

Types::FormFactor Applet::formFactor() const
{
    Containment *c = containment();
    QObject *pw = qobject_cast<QObject *>(parent());
    Lingmo::Applet *parentApplet = qobject_cast<Lingmo::Applet *>(pw);
    // assumption: this loop is usually is -really- short or doesn't run at all
    while (!parentApplet && pw && pw->parent()) {
        pw = pw->parent();
        parentApplet = qobject_cast<Lingmo::Applet *>(pw);
    }

    return c ? c->d->formFactor : Lingmo::Types::Planar;
}

Types::ContainmentDisplayHints Applet::containmentDisplayHints() const
{
    Containment *c = containment();

    return c ? c->d->containmentDisplayHints : Lingmo::Types::NoContainmentDisplayHint;
}

Containment *Applet::containment() const
{
    Containment *c = qobject_cast<Containment *>(const_cast<Applet *>(this));
    if (c && c->isContainment()) {
        return c;
    } else {
        c = nullptr;
    }

    QObject *parent = this->parent();

    while (parent) {
        Containment *possibleC = qobject_cast<Containment *>(parent);

        if (possibleC && possibleC->isContainment()) {
            c = possibleC;
            break;
        }
        parent = parent->parent();
    }

    return c;
}

void Applet::setGlobalShortcut(const QKeySequence &shortcut)
{
    if (!d->activationAction) {
        d->activationAction = new QAction(this);
        d->activationAction->setText(i18n("Activate %1 Widget", title()));
        d->activationAction->setObjectName(QStringLiteral("activate widget %1").arg(id())); // NO I18N
        connect(d->activationAction, &QAction::triggered, this, &Applet::activated);
        connect(KGlobalAccel::self(), &KGlobalAccel::globalShortcutChanged, this, [this](QAction *action, const QKeySequence &shortcut) {
            if (action == d->activationAction) {
                d->activationAction->setShortcut(shortcut);
                d->globalShortcutChanged();
            }
        });
    } else if (d->activationAction->shortcut() == shortcut) {
        return;
    }

    d->activationAction->setShortcut(shortcut);
    d->globalShortcutEnabled = true;
    QList<QKeySequence> seqs{shortcut};
    KGlobalAccel::self()->setShortcut(d->activationAction, seqs, KGlobalAccel::NoAutoloading);
    d->globalShortcutChanged();

    Q_EMIT globalShortcutChanged(shortcut);
}

QKeySequence Applet::globalShortcut() const
{
    if (d->activationAction) {
        QList<QKeySequence> shortcuts = KGlobalAccel::self()->shortcut(d->activationAction);
        if (!shortcuts.isEmpty()) {
            return shortcuts.first();
        }
    }

    return QKeySequence();
}

Types::Location Applet::location() const
{
    Containment *c = containment();
    return c ? c->d->location : Lingmo::Types::Desktop;
}

bool Applet::hasConfigurationInterface() const
{
    return d->hasConfigurationInterface;
}

void Applet::setHasConfigurationInterface(bool hasInterface)
{
    if (hasInterface == d->hasConfigurationInterface) {
        return;
    }

    QAction *configAction = d->actions.value(QStringLiteral("configure"));
    if (configAction) {
        bool enable = hasInterface;
        if (enable) {
            const bool unlocked = immutability() == Types::Mutable;
            enable = unlocked || KAuthorized::authorize(QStringLiteral("lingmo/allow_configure_when_locked"));
        }
        configAction->setEnabled(enable);
    }

    d->hasConfigurationInterface = hasInterface;
    Q_EMIT hasConfigurationInterfaceChanged(hasInterface);
}

void Applet::configChanged()
{
    if (d->configLoader) {
        d->configLoader->load();
    }
}

QUrl Applet::fileUrl(const QByteArray &key, const QString &filename) const
{
    if (d->package.isValid()) {
        return d->package.fileUrl(key, filename);
    }
    return QUrl();
}

QUrl Applet::mainScript() const
{
    if (d->package.isValid()) {
        return d->package.fileUrl("mainscript");
    }
    return QUrl();
}

QUrl Applet::configModel() const
{
    if (d->package.isValid()) {
        return d->package.fileUrl("configmodel");
    }

    return QUrl();
}

bool Applet::sourceValid() const
{
    return d->package.isValid();
}

void Applet::timerEvent(QTimerEvent *event)
{
    if (d->transient) {
        d->constraintsTimer.stop();
        if (d->modificationsTimer) {
            d->modificationsTimer->stop();
        }
        return;
    }

    if (event->timerId() == d->constraintsTimer.timerId()) {
        d->constraintsTimer.stop();

        // Don't flushPendingConstraints if we're just starting up
        // flushPendingConstraints will be called by Corona
        if (!(d->pendingConstraints & StartupCompletedConstraint)) {
            flushPendingConstraintsEvents();
        }
    } else if (d->modificationsTimer && event->timerId() == d->modificationsTimer->timerId()) {
        d->modificationsTimer->stop();
        // invalid group, will result in save using the default group
        KConfigGroup cg;

        save(cg);
        Q_EMIT configNeedsSaving();
    }
}

bool Applet::isContainment() const
{
    // HACK: this is a special case for the systray
    // containment in an applet that is not a containment
    Applet *pa = qobject_cast<Applet *>(parent());
    if (pa && !pa->isContainment()) {
        return true;
    }
    // normal "acting as a containment" condition
    return qobject_cast<const Containment *>(this) && qobject_cast<Corona *>(parent());
}

QString Applet::translationDomain() const
{
    const QString rootPath = d->appletDescription.value(QStringLiteral("X-Lingmo-RootPath"));
    if (!rootPath.isEmpty()) {
        return QLatin1String("lingmo_applet_") + rootPath;
    } else {
        return QLatin1String("lingmo_applet_") + d->appletDescription.pluginId();
    }
}

} // Lingmo namespace

#include "moc_applet.cpp"
