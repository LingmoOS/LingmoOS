/*
    SPDX-FileCopyrightText: 2007 Matt Broadstone <mbroadst@gmail.com>
    SPDX-FileCopyrightText: 2007-2011 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "corona.h"
#include "private/corona_p.h"

#include <QDebug>
#include <QGuiApplication>
#include <QJSEngine>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include <KLocalizedString>

#include <cmath>

#include "containment.h"
#include "debug_p.h"
#include "pluginloader.h"
#include "private/applet_p.h"
#include "private/containment_p.h"

using namespace Lingmo;

namespace Lingmo
{
Corona::Corona(QObject *parent)
    : QObject(parent)
    , d(new CoronaPrivate(this))
{
    d->init();
}

Corona::~Corona()
{
    KConfigGroup trans(KSharedConfig::openConfig(), QStringLiteral("LingmoTransientsConfig"));
    trans.deleteGroup();

    delete d;
}

KPackage::Package Corona::kPackage() const
{
    return d->package;
}

void Corona::setKPackage(const KPackage::Package &package)
{
    d->package = package;
    Q_EMIT kPackageChanged(package);
}

void Corona::saveLayout(const QString &configName) const
{
    KSharedConfigPtr c;

    if (configName.isEmpty() || configName == d->configName) {
        c = config();
    } else {
        c = KSharedConfig::openConfig(configName, KConfig::SimpleConfig);
    }

    d->saveLayout(c);
}

void Corona::exportLayout(KConfigGroup &config, QList<Containment *> containments)
{
    const auto groupList = config.groupList();
    for (const QString &group : groupList) {
        KConfigGroup cg(&config, group);
        cg.deleteGroup();
    }

    // temporarily unlock so that removal works
    Types::ImmutabilityType oldImm = immutability();
    d->immutability = Types::Mutable;

    KConfigGroup dest(&config, QStringLiteral("Containments"));
    KConfigGroup dummy;
    for (Lingmo::Containment *c : std::as_const(containments)) {
        c->save(dummy);
        c->config().reparent(&dest);

        // ensure the containment is unlocked
        // this is done directly because we have to bypass any Types::SystemImmutable checks
        c->Applet::d->immutability = Types::Mutable;
        const auto lstApplet = c->applets();
        for (Applet *a : lstApplet) {
            a->d->immutability = Types::Mutable;
        }

        c->destroy();
    }

    // restore immutability
    d->immutability = oldImm;

    config.sync();
}

void Corona::requestConfigSync()
{
    // constant controlling how long between requesting a configuration sync
    // and one happening should occur. currently 10 seconds
    static const int CONFIG_SYNC_TIMEOUT = 10000;

    // TODO: should we check into our immutability before doing this?

    // NOTE: this is a pretty simplistic model: we simply save no more than CONFIG_SYNC_TIMEOUT
    //      after the first time this is called. not much of a heuristic for save points, but
    //      it should at least compress these activities a bit and provide a way for applet
    //      authors to ween themselves from the sync() disease. A more interesting/dynamic
    //      algorithm for determining when to actually sync() to disk might be better, though.
    if (!d->configSyncTimer->isActive()) {
        d->configSyncTimer->start(CONFIG_SYNC_TIMEOUT);
    }
}

void Corona::requireConfigSync()
{
    d->syncConfig();
}

void Corona::loadLayout(const QString &configName)
{
    if (!configName.isEmpty() && configName != d->configName) {
        // if we have a new config name passed in, then use that as the config file for this Corona
        d->config = nullptr;
        d->configName = configName;
    }

    KConfigGroup conf(config(), QString());
    if (!config()->groupList().isEmpty()) {
        d->importLayout(conf, false);
    } else {
        loadDefaultLayout();
        d->notifyContainmentsReady();
    }

    KConfigGroup cg(config(), QStringLiteral("General"));
    setImmutability((Lingmo::Types::ImmutabilityType)cg.readEntry("immutability", (int)Lingmo::Types::Mutable));
}

QList<Lingmo::Containment *> Corona::importLayout(const KConfigGroup &conf)
{
    return d->importLayout(conf, true);
}

Containment *Corona::containmentForScreen(int screen, const QString &activity, const QString &defaultPluginIfNonExistent, const QVariantList &defaultArgs)
{
    Containment *containment = nullptr;

    for (Containment *cont : std::as_const(d->containments)) {
        if (cont->lastScreen() == screen //
            && ((cont->activity().isEmpty() || activity.isEmpty()) || cont->activity() == activity)
            && (cont->containmentType() == Lingmo::Containment::Type::Desktop //
                || cont->containmentType() == Lingmo::Containment::Type::Custom || cont->containmentType() == Lingmo::Containment::Type::NoContainment)) {
            containment = cont;
        }
    }

    if (!containment && !defaultPluginIfNonExistent.isEmpty()) {
        // screen requests are allowed to bypass immutability
        if (screen >= 0) {
            Lingmo::Types::ImmutabilityType imm = d->immutability;
            d->immutability = Types::Mutable;
            containment = d->addContainment(defaultPluginIfNonExistent, defaultArgs, 0, screen, false);

            d->immutability = imm;
        }
    }

    if (containment) {
        containment->setActivity(activity);
    }
    return containment;
}

QList<Containment *> Corona::containmentsForActivity(const QString &activity)
{
    QList<Containment *> conts;

    if (activity.isEmpty()) {
        return conts;
    }

    std::copy_if(d->containments.begin(), d->containments.end(), std::back_inserter(conts), [activity](Containment *cont) {
        return cont->activity() == activity
            && (cont->containmentType() == Lingmo::Containment::Type::Desktop || cont->containmentType() == Lingmo::Containment::Type::Custom);
    });

    return conts;
}

QList<Containment *> Corona::containmentsForScreen(int screen)
{
    QList<Containment *> conts;

    if (screen < 0) {
        return conts;
    }

    std::copy_if(d->containments.begin(), d->containments.end(), std::back_inserter(conts), [screen](Containment *cont) {
        return cont->lastScreen() == screen
            && (cont->containmentType() == Lingmo::Containment::Type::Desktop //
                || cont->containmentType() == Lingmo::Containment::Type::Custom);
    });

    return conts;
}

QList<Containment *> Corona::containments() const
{
    return d->containments;
}

bool Corona::isStartupCompleted() const
{
    return d->containmentsStarting <= 0;
}

KSharedConfigPtr Corona::config() const
{
    if (!d->config) {
        d->config = KSharedConfig::openConfig(d->configName, KConfig::SimpleConfig);
    }

    return d->config;
}

Containment *Corona::createContainment(const QString &name, const QVariantList &args)
{
    if (d->immutability == Types::Mutable || args.contains(QVariant::fromValue(QStringLiteral("org.kde.lingmo:force-create")))) {
        return d->addContainment(name, args, 0, -1, false);
    }

    return nullptr;
}

Containment *Corona::createContainmentDelayed(const QString &name, const QVariantList &args)
{
    if (d->immutability == Types::Mutable) {
        return d->addContainment(name, args, 0, -1, true);
    }

    return nullptr;
}

int Corona::screenForContainment(const Containment *) const
{
    return -1;
}

int Corona::numScreens() const
{
    return 1;
}

QRegion Corona::availableScreenRegion(int id) const
{
    return QRegion(screenGeometry(id));
}

QRect Corona::availableScreenRect(int id) const
{
    return screenGeometry(id);
}

void Corona::loadDefaultLayout()
{
    // Default implementation does nothing
}

Types::ImmutabilityType Corona::immutability() const
{
    return d->immutability;
}

void Corona::setImmutability(const Types::ImmutabilityType immutable)
{
    if (d->immutability == immutable || d->immutability == Types::SystemImmutable) {
        return;
    }

#ifndef NDEBUG
    // qCDebug(LOG_LINGMO) << "setting immutability to" << immutable;
#endif
    d->immutability = immutable;
    d->updateContainmentImmutability();
    // tell non-containments that might care (like lingmoapp or a custom corona)
    Q_EMIT immutabilityChanged(immutable);

    // update our actions
    QAction *action = d->actions.value(QStringLiteral("lock widgets"));
    if (action) {
        if (d->immutability == Types::SystemImmutable) {
            action->setEnabled(false);
            action->setVisible(false);
        } else {
            bool unlocked = d->immutability == Types::Mutable;
            action->setText(unlocked ? i18n("Lock Widgets") : i18n("Unlock Widgets"));
            action->setIcon(QIcon::fromTheme(unlocked ? QStringLiteral("object-locked") : QStringLiteral("object-unlocked")));
            action->setEnabled(true);
            action->setVisible(true);
        }
    }

    action = d->actions.value(QStringLiteral("edit mode"));
    if (action) {
        switch (d->immutability) {
        case Types::UserImmutable:
            action->setEnabled(false);
            action->setVisible(true);
            break;
        case Types::SystemImmutable:
            action->setEnabled(false);
            action->setVisible(false);
            break;
        case Types::Mutable:
        default:
            action->setEnabled(true);
            action->setVisible(true);
            break;
        }
    }

    if (d->immutability != Types::SystemImmutable) {
        KConfigGroup cg(config(), QStringLiteral("General"));

        // we call the dptr member directly for locked since isImmutable()
        // also checks kiosk and parent containers
        cg.writeEntry("immutability", (int)d->immutability);
        requestConfigSync();
    }

    if (d->immutability != Types::Mutable) {
        setEditMode(false);
    }
}

void Corona::setEditMode(bool edit)
{
    if (edit == d->editMode || (edit && d->immutability != Lingmo::Types::Mutable)) {
        return;
    }

    QAction *editAction = d->actions.value(QStringLiteral("edit mode"));
    if (editAction) {
        if (edit) {
            editAction->setText(i18n("Exit Edit Mode"));
        } else {
            editAction->setText(i18n("Enter Edit Mode"));
        }
    }

    if (!edit) {
        requireConfigSync();
    }

    d->editMode = edit;
    Q_EMIT editModeChanged(edit);
}

bool Corona::isEditMode() const
{
    return d->editMode;
}

QList<Lingmo::Types::Location> Corona::freeEdges(int screen) const
{
    QList<Lingmo::Types::Location> freeEdges;
    /* clang-format off */
    freeEdges << Lingmo::Types::TopEdge
              << Lingmo::Types::BottomEdge
              << Lingmo::Types::LeftEdge
              << Lingmo::Types::RightEdge;
    /* clang-format on */

    const auto containments = this->containments();
    for (Containment *containment : containments) {
        if (containment->lastScreen() == screen && freeEdges.contains(containment->location())) {
            freeEdges.removeAll(containment->location());
        }
    }

    return freeEdges;
}

QAction *Corona::action(const QString &name) const
{
    return d->actions.value(name);
}

void Corona::setAction(const QString &name, QAction *action)
{
    if (name.isEmpty()) {
        return;
    }
    action->setObjectName(name);
    QAction *oldAction = d->actions.value(name);
    if (oldAction && QJSEngine::objectOwnership(oldAction) == QJSEngine::CppOwnership) {
        delete oldAction;
    }
    connect(action, &QObject::destroyed, this, [this, name]() {
        d->actions.remove(name);
    });
    d->actions[name] = action;
}

void Corona::removeAction(const QString &name)
{
    QAction *action = d->actions.value(name);
    if (action && QJSEngine::objectOwnership(action) == QJSEngine::CppOwnership) {
        delete action;
    }
    d->actions.remove(name);
}

QList<QAction *> Corona::actions() const
{
    return d->actions.values();
}

CoronaPrivate::CoronaPrivate(Corona *corona)
    : q(corona)
    , immutability(Types::Mutable)
    , config(nullptr)
    , configSyncTimer(new QTimer(corona))
    , containmentsStarting(0)
{
    // TODO: make Package path configurable

    if (QCoreApplication::instance()) {
        configName = QCoreApplication::instance()->applicationName() + QStringLiteral("-appletsrc");
    } else {
        configName = QStringLiteral("lingmo-appletsrc");
    }
}

CoronaPrivate::~CoronaPrivate()
{
    qDeleteAll(containments);
}

void CoronaPrivate::init()
{
    desktopDefaultsConfig = KConfigGroup(KSharedConfig::openConfig(package.filePath("defaults")), QStringLiteral("Desktop"));

    configSyncTimer->setSingleShot(true);
    QObject::connect(configSyncTimer, SIGNAL(timeout()), q, SLOT(syncConfig()));

    QAction *lockAction = new QAction(q);
    q->setAction(QStringLiteral("lock widgets"), lockAction);
    QObject::connect(lockAction, SIGNAL(triggered(bool)), q, SLOT(toggleImmutability()));
    lockAction->setText(i18n("Lock Widgets"));
    lockAction->setAutoRepeat(true);
    lockAction->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
    lockAction->setShortcutContext(Qt::ApplicationShortcut);

    // fake containment/applet actions
    auto containmentActions = AppletPrivate::defaultActions(q); // containment has to start with applet stuff
    ContainmentPrivate::addDefaultActions(containmentActions, nullptr, q); // now it's really containment
    actions.insert(containmentActions);

    QAction *editAction = new QAction(q);
    q->setAction(QStringLiteral("edit mode"), editAction);
    QObject::connect(editAction, &QAction::triggered, q, [this]() {
        q->setEditMode(!q->isEditMode());
    });
    editAction->setText(i18n("Enter Edit Mode"));
    editAction->setAutoRepeat(true);
    editAction->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    editAction->setShortcut(QKeySequence(QStringLiteral("alt+d, e")));
    editAction->setShortcutContext(Qt::ApplicationShortcut);
}

void CoronaPrivate::toggleImmutability()
{
    if (immutability == Types::Mutable) {
        q->setImmutability(Types::UserImmutable);
    } else {
        q->setImmutability(Types::Mutable);
    }
}

void CoronaPrivate::saveLayout(KSharedConfigPtr cg) const
{
    KConfigGroup containmentsGroup(cg, QStringLiteral("Containments"));
    for (const Containment *containment : containments) {
        QString cid = QString::number(containment->id());
        KConfigGroup containmentConfig(&containmentsGroup, cid);
        containment->save(containmentConfig);
    }
}

void CoronaPrivate::updateContainmentImmutability()
{
    for (Containment *c : std::as_const(containments)) {
        // we need to tell each containment that immutability has been altered
        c->updateConstraints(Applet::ImmutableConstraint);
    }
}

void CoronaPrivate::containmentDestroyed(QObject *obj)
{
    // we do a static_cast here since it really isn't an Containment by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Containment *containment = static_cast<Lingmo::Containment *>(obj);
    int index = containments.indexOf(containment);

    if (index > -1) {
        containments.removeAt(index);
        q->requestConfigSync();
    }
}

void CoronaPrivate::syncConfig()
{
    q->config()->sync();
    Q_EMIT q->configSynced();
}

Containment *CoronaPrivate::addContainment(const QString &name, const QVariantList &args, uint id, int lastScreen, bool delayedInit)
{
    QString pluginName = name;
    Containment *containment = nullptr;
    Applet *applet = nullptr;

    // qCDebug(LOG_LINGMO) << "Loading" << name << args << id;

    if (pluginName.isEmpty() || pluginName == QLatin1String("default")) {
        // default to the desktop containment
        pluginName = desktopDefaultsConfig.readEntry("Containment", "org.kde.desktopcontainment");
    }

    bool loadingNull = pluginName == QLatin1String("null");
    if (!loadingNull) {
        applet = PluginLoader::self()->loadApplet(pluginName, id, args);
        containment = dynamic_cast<Containment *>(applet);
        if (containment) {
            containment->setParent(q);
        }
    }

    if (!containment) {
        if (!loadingNull) {
#ifndef NDEBUG
            // qCDebug(LOG_LINGMO) << "loading of containment" << name << "failed.";
#endif
        }
        // in case we got a non-Containment from Applet::loadApplet or
        // a null containment was requested
        if (applet) {
            // the applet probably doesn't know what's hit it, so let's pretend it can be
            // initialized to make assumptions in the applet's dtor safer
            applet->init();
            delete applet;
        }
        applet = containment = new Containment(q, KPluginMetaData(), QVariantList{QVariant(), id});
        if (lastScreen >= 0) {
            containment->d->lastScreen = lastScreen;
        }
        // if it's a dummy containment, just say its ui is ready, not blocking the corona
        applet->updateConstraints(Applet::UiReadyConstraint);

        // we want to provide something and don't care about the failure to launch
        containment->setFormFactor(Lingmo::Types::Planar);
    }

    // if this is a new containment, we need to ensure that there are no stale
    // configuration data around
    if (id == 0) {
        KConfigGroup conf(q->config(), QStringLiteral("Containments"));
        conf = KConfigGroup(&conf, QString::number(containment->id()));
        conf.deleteGroup();
    }

    // make sure the containments are sorted by id
    auto position = std::lower_bound(containments.begin(), containments.end(), containment, [](Lingmo::Containment *c1, Lingmo::Containment *c2) {
        return c1->id() < c2->id();
    });
    containments.insert(position, containment);

    QObject::connect(containment, SIGNAL(destroyed(QObject *)), q, SLOT(containmentDestroyed(QObject *)));
    QObject::connect(containment, &Applet::configNeedsSaving, q, &Corona::requestConfigSync);
    QObject::connect(containment, &Containment::screenChanged, q, &Corona::screenOwnerChanged);

    if (!delayedInit) {
        containment->init();
        KConfigGroup cg = containment->config();
        containment->restore(cg);
        containment->updateConstraints(Applet::StartupCompletedConstraint);
        containment->save(cg);
        q->requestConfigSync();
        containment->flushPendingConstraintsEvents();
        Q_EMIT q->containmentAdded(containment);
        // if id = 0 a new containment has been created, not restored
        if (id == 0) {
            Q_EMIT q->containmentCreated(containment);
        }
    }

    return containment;
}

QList<Lingmo::Containment *> CoronaPrivate::importLayout(const KConfigGroup &conf, bool mergeConfig)
{
    if (!conf.isValid()) {
        return QList<Containment *>();
    }

    QList<Lingmo::Containment *> newContainments;
    QSet<uint> containmentsIds;

    for (Containment *containment : std::as_const(containments)) {
        containmentsIds.insert(containment->id());
    }

    KConfigGroup containmentsGroup(&conf, QStringLiteral("Containments"));
    QStringList groups = containmentsGroup.groupList();
    std::sort(groups.begin(), groups.end());

    for (const QString &group : std::as_const(groups)) {
        KConfigGroup containmentConfig(&containmentsGroup, group);

        if (containmentConfig.entryMap().isEmpty()) {
            continue;
        } else if (containmentConfig.readEntry(QStringLiteral("transient"), false)) {
            containmentConfig.deleteGroup();
            continue;
        }

        uint cid = group.toUInt();
        if (containmentsIds.contains(cid)) {
            cid = ++AppletPrivate::s_maxAppletId;
        } else if (cid > AppletPrivate::s_maxAppletId) {
            AppletPrivate::s_maxAppletId = cid;
        }

        if (mergeConfig) {
            KConfigGroup realConf(q->config(), QStringLiteral("Containments"));
            realConf = KConfigGroup(&realConf, QString::number(cid));
            // in case something was there before us
            realConf.deleteGroup();
            containmentConfig.copyTo(&realConf);
        }

        // qCDebug(LOG_LINGMO) << "got a containment in the config, trying to make a" << containmentConfig.readEntry("plugin", QString()) << "from" << group;
#ifndef NDEBUG
        // qCDebug(LOG_LINGMO) << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Adding Containment" << containmentConfig.readEntry("plugin",
        // QString());
#endif
        Containment *c = addContainment(containmentConfig.readEntry("plugin", QString()), QVariantList(), cid, -1);
        if (!c) {
            continue;
        }

        newContainments.append(c);
        containmentsIds.insert(c->id());

#ifndef NDEBUG
//         qCDebug(LOG_LINGMO) << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Restored Containment" << c->pluginName();
#endif
    }

    if (!mergeConfig) {
        notifyContainmentsReady();
    }

    return newContainments;
}

void CoronaPrivate::notifyContainmentsReady()
{
    containmentsStarting = 0;
    for (Containment *containment : std::as_const(containments)) {
        if (!containment->isUiReady() && containment->screen() >= 0) {
            ++containmentsStarting;
            QObject::connect(containment, &Lingmo::Containment::uiReadyChanged, q, [this](bool ready) {
                containmentReady(ready);
            });
        }
    }

    if (containmentsStarting <= 0) {
        Q_EMIT q->startupCompleted();
    }
}

void CoronaPrivate::containmentReady(bool ready)
{
    if (!ready) {
        return;
    }
    --containmentsStarting;
    if (containmentsStarting <= 0) {
        Q_EMIT q->startupCompleted();
    }
}

} // namespace Lingmo

#include "moc_corona.cpp"
