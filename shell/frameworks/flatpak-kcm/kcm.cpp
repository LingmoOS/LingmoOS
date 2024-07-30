/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcm.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <QFile>
#include <algorithm>

K_PLUGIN_CLASS_WITH_JSON(KCMFlatpak, "kcm_flatpak.json")

KCMFlatpak::KCMFlatpak(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickManagedConfigModule(parent, data)
    , m_refsModel(new FlatpakReferencesModel(this))
{
    QString requestedReference;
    if (args.count() > 0) {
        const QVariant &arg0 = args.at(0);
        if (arg0.canConvert<QString>()) {
            const QString arg0str = arg0.toString();
            requestedReference = arg0str;
        }
    }

    constexpr const char *uri = "org.kde.lingmo.kcm.flatpakpermissions";

    qmlRegisterUncreatableType<KCMFlatpak>(uri, 1, 0, "KCMFlatpak", QString());
    qmlRegisterUncreatableType<FlatpakReference>(uri, 1, 0, "FlatpakReference", QStringLiteral("Should be obtained from a FlatpakReferencesModel"));
    qmlRegisterType<FlatpakPermissionModel>(uri, 1, 0, "FlatpakPermissionModel");
    qmlRegisterUncreatableType<FlatpakReferencesModel>(uri, 1, 0, "FlatpakReferencesModel", QStringLiteral("For enum access only"));
    qmlRegisterUncreatableType<FlatpakPermissionsSectionType>(uri, 1, 0, "FlatpakPermissionsSectionType", QStringLiteral("For enum access only"));

    connect(m_refsModel, &FlatpakReferencesModel::needsLoad, this, &KCMFlatpak::load);
    connect(m_refsModel, &FlatpakReferencesModel::settingsChanged, this, &KCMFlatpak::settingsChanged);

    if (!requestedReference.isEmpty()) {
        const auto &refs = m_refsModel->references();
        const auto it = std::find_if(refs.constBegin(), refs.constEnd(), [&](FlatpakReference *ref) {
            return ref->ref() == requestedReference;
        });
        if (it != refs.constEnd()) {
            const auto index = std::distance(refs.constBegin(), it);
            m_index = index;
        }
    }

    settingsChanged(); // Initialize Reset & Defaults buttons
}

void KCMFlatpak::load()
{
    m_refsModel->load(m_index);
    setNeedsSave(false);
}

void KCMFlatpak::save()
{
    m_refsModel->save(m_index);
}

void KCMFlatpak::defaults()
{
    m_refsModel->defaults(m_index);
}

bool KCMFlatpak::isSaveNeeded() const
{
    return m_refsModel->isSaveNeeded(m_index);
}

bool KCMFlatpak::isDefaults() const
{
    return m_refsModel->isDefaults(m_index);
}

void KCMFlatpak::setIndex(int index)
{
    m_index = index;
    settingsChanged(); // Because Apply, Reset & Defaults buttons depend on m_index.
}

int KCMFlatpak::currentIndex() const
{
    return m_index;
}

#include "kcm.moc"

#include "moc_kcm.cpp"
