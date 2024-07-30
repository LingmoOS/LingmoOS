/*
    SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickmanagedconfigmodule.h"

#include <KCoreConfigSkeleton>
#include <QPointer>
#include <QTimer>

class KQuickManagedConfigModulePrivate
{
public:
    KQuickManagedConfigModulePrivate(KQuickManagedConfigModule *mod)
    {
        QTimer::singleShot(0, mod, [mod]() {
            const auto skeletons = mod->findChildren<KCoreConfigSkeleton *>();
            for (auto *skeleton : skeletons) {
                mod->registerSettings(skeleton);
            }
        });
    }

    QList<QPointer<KCoreConfigSkeleton>> _skeletons;
};

KQuickManagedConfigModule::KQuickManagedConfigModule(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData)
    , d(new KQuickManagedConfigModulePrivate(this))
{
}

KQuickManagedConfigModule::~KQuickManagedConfigModule() = default;

void KQuickManagedConfigModule::load()
{
    for (const auto &skeleton : std::as_const(d->_skeletons)) {
        if (skeleton) {
            skeleton->load();
        }
    }
}

void KQuickManagedConfigModule::save()
{
    for (const auto &skeleton : std::as_const(d->_skeletons)) {
        if (skeleton) {
            skeleton->save();
        }
    }
}

void KQuickManagedConfigModule::defaults()
{
    for (const auto &skeleton : std::as_const(d->_skeletons)) {
        if (skeleton) {
            skeleton->setDefaults();
        }
    }
}

bool KQuickManagedConfigModule::isSaveNeeded() const
{
    return false;
}

bool KQuickManagedConfigModule::isDefaults() const
{
    return true;
}

void KQuickManagedConfigModule::settingsChanged()
{
    bool needsSave = false;
    bool representsDefaults = true;
    for (const auto &skeleton : std::as_const(d->_skeletons)) {
        if (skeleton) {
            needsSave |= skeleton->isSaveNeeded();
            representsDefaults &= skeleton->isDefaults();
        }
    }

    if (!needsSave) {
        needsSave = isSaveNeeded();
    }

    if (representsDefaults) {
        representsDefaults = isDefaults();
    }

    setRepresentsDefaults(representsDefaults);
    setNeedsSave(needsSave);
}

void KQuickManagedConfigModule::registerSettings(KCoreConfigSkeleton *skeleton)
{
    if (!skeleton || d->_skeletons.contains(skeleton)) {
        return;
    }

    d->_skeletons.append(skeleton);

    auto settingsChangedSlotIndex = metaObject()->indexOfMethod("settingsChanged()");
    auto settingsChangedSlot = metaObject()->method(settingsChangedSlotIndex);

    QObject::connect(skeleton, &KCoreConfigSkeleton::configChanged, this, &KQuickManagedConfigModule::settingsChanged);

    const auto items = skeleton->items();
    for (auto item : items) {
        const auto itemHasSignals = dynamic_cast<KConfigCompilerSignallingItem *>(item) || dynamic_cast<KPropertySkeletonItem *>(item);
        if (!itemHasSignals) {
            continue;
        }

        auto name = item->name();
        if (name.at(0).isUpper()) {
            name[0] = name[0].toLower();
        }

        const auto metaObject = skeleton->metaObject();
        const auto propertyIndex = metaObject->indexOfProperty(name.toUtf8().constData());
        const auto property = metaObject->property(propertyIndex);
        if (!property.hasNotifySignal()) {
            continue;
        }

        const auto changedSignal = property.notifySignal();
        QObject::connect(skeleton, changedSignal, this, settingsChangedSlot);
    }

    auto toRemove = std::remove_if(d->_skeletons.begin(), d->_skeletons.end(), [](const QPointer<KCoreConfigSkeleton> &value) {
        return value.isNull();
    });
    d->_skeletons.erase(toRemove, d->_skeletons.end());

    QMetaObject::invokeMethod(this, "settingsChanged", Qt::QueuedConnection);
}

#include "moc_kquickmanagedconfigmodule.cpp"
