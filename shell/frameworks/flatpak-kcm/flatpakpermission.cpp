/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "flatpakpermission.h"
#include "flatpakcommon.h"
#include "flatpakhelper.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QChar>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMetaEnum>
#include <QQmlEngine>
#include <QTemporaryFile>
#include <QUrl>

#include <algorithm>
#include <array>
#include <optional>
#include <variant>

namespace
{

/**
 * Type of QList mapped with function f over its items.
 */
template<typename T, typename F>
using MappedList = QList<typename std::invoke_result_t<F, const T &>::value_type>;

/**
 * Map QList with a function that returns optional values.
 */
template<typename T, typename F>
MappedList<T, F> filter_map(const QList<T> &iter, F func)
{
    MappedList<T, F> succeeded;

    for (const auto &item : iter) {
        const auto optional = func(item);
        if (optional.has_value()) {
            succeeded.append(optional.value());
        }
    }

    return succeeded;
}

/**
 * Map QList with a function that returns optional values, but also returns a
 * QList of original items that were failed to map.
 */
template<typename T, typename F>
std::pair<QList<T>, MappedList<T, F>> try_filter_map(const QList<T> &iter, F func)
{
    QList<T> failed;
    MappedList<T, F> succeeded;

    for (const auto &item : iter) {
        const auto optional = func(item);
        if (optional.has_value()) {
            succeeded.append(optional.value());
        } else {
            failed.append(item);
        }
    }

    return {failed, succeeded};
}

const QLatin1String SUFFIX_RO = QLatin1String(":ro");
const QLatin1String SUFFIX_RW = QLatin1String(":rw");
const QLatin1String SUFFIX_CREATE = QLatin1String(":create");
const QLatin1Char PREFIX_DENY = QLatin1Char('!');

using FilesystemPrefix = FlatpakFilesystemsEntry::FilesystemPrefix;
using PathMode = FlatpakFilesystemsEntry::PathMode;

constexpr FlatpakFilesystemsEntry::TableEntry makeRequiredPath(FilesystemPrefix prefix, const char *prefixString)
{
    return FlatpakFilesystemsEntry::TableEntry{prefix, PathMode::Required, QLatin1String(), QLatin1String(prefixString)};
}

constexpr FlatpakFilesystemsEntry::TableEntry makeOptionalPath(FilesystemPrefix prefix, const char *fixedString, const char *prefixString)
{
    return FlatpakFilesystemsEntry::TableEntry{prefix, PathMode::Optional, QLatin1String(fixedString), QLatin1String(prefixString)};
}

constexpr FlatpakFilesystemsEntry::TableEntry makeInvalidPath(FilesystemPrefix prefix, const char *fixedString)
{
    return FlatpakFilesystemsEntry::TableEntry{prefix, PathMode::NoPath, QLatin1String(fixedString), QLatin1String()};
}

const auto s_filesystems = {
    //
    makeRequiredPath(FilesystemPrefix::Absolute, "/"),
    //
    makeOptionalPath(FilesystemPrefix::Home, "~", "~/"),
    makeOptionalPath(FilesystemPrefix::Home, "home", "home/"),
    //
    makeInvalidPath(FilesystemPrefix::Host, "host"),
    makeInvalidPath(FilesystemPrefix::HostOs, "host-os"),
    makeInvalidPath(FilesystemPrefix::HostEtc, "host-etc"),
    //
    makeOptionalPath(FilesystemPrefix::XdgDesktop, "xdg-desktop", "xdg-desktop/"),
    makeOptionalPath(FilesystemPrefix::XdgDocuments, "xdg-documents", "xdg-documents/"),
    makeOptionalPath(FilesystemPrefix::XdgDownload, "xdg-download", "xdg-download/"),
    makeOptionalPath(FilesystemPrefix::XdgMusic, "xdg-music", "xdg-music/"),
    makeOptionalPath(FilesystemPrefix::XdgPictures, "xdg-pictures", "xdg-pictures/"),
    makeOptionalPath(FilesystemPrefix::XdgPublicShare, "xdg-public-share", "xdg-public-share/"),
    makeOptionalPath(FilesystemPrefix::XdgVideos, "xdg-videos", "xdg-videos/"),
    makeOptionalPath(FilesystemPrefix::XdgTemplates, "xdg-templates", "xdg-templates/"),
    //
    makeOptionalPath(FilesystemPrefix::XdgCache, "xdg-cache", "xdg-cache/"),
    makeOptionalPath(FilesystemPrefix::XdgConfig, "xdg-config", "xdg-config/"),
    makeOptionalPath(FilesystemPrefix::XdgData, "xdg-data", "xdg-data/"),
    //
    makeRequiredPath(FilesystemPrefix::XdgRun, "xdg-run/"),
    //
    makeRequiredPath(FilesystemPrefix::Unknown, ""),
};

} // namespace

FlatpakSimpleEntry::FlatpakSimpleEntry(const QString &name, bool enabled)
    : m_name(name)
    , m_enabled(enabled)
{
}

std::optional<FlatpakSimpleEntry> FlatpakSimpleEntry::parse(QStringView entry)
{
    bool enabled = true;
    if (entry.startsWith(PREFIX_DENY)) {
        entry = entry.mid(1);
        enabled = false;
    }

    // For now we don't do any additional validation
    if (entry.isEmpty()) {
        return std::nullopt;
    }

    const auto name = entry.toString();
    return std::optional(FlatpakSimpleEntry(name, enabled));
}

std::pair<QStringList, QList<FlatpakSimpleEntry>> FlatpakSimpleEntry::getCategory(const KConfigGroup &group, const QString &category)
{
    return try_filter_map(group.readXdgListEntry(category), [](const QString &entry) {
        return FlatpakSimpleEntry::parse(entry);
    });
}

QList<FlatpakSimpleEntry> FlatpakSimpleEntry::getCategorySkippingInvalidEntries(const KConfigGroup &group, const QString &category)
{
    return filter_map(group.readXdgListEntry(category), [](const QString &entry) {
        return FlatpakSimpleEntry::parse(entry);
    });
}

std::optional<bool> FlatpakSimpleEntry::isEnabled(const QList<FlatpakSimpleEntry> &entries, const QString &name)
{
    for (const auto &entry : entries) {
        if (entry.name() == name) {
            return std::optional(entry.isEnabled());
        }
    }
    return std::nullopt;
}

QString FlatpakSimpleEntry::format() const
{
    if (m_enabled) {
        return m_name;
    } else {
        return PREFIX_DENY + m_name;
    }
}

bool FlatpakSimpleEntry::isEnabled() const
{
    return m_enabled;
}

void FlatpakSimpleEntry::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

const QString &FlatpakSimpleEntry::name() const
{
    return m_name;
}

bool FlatpakSimpleEntry::operator==(const FlatpakSimpleEntry &other) const
{
    return m_name == other.m_name && m_enabled == other.m_enabled;
}

bool FlatpakSimpleEntry::operator!=(const FlatpakSimpleEntry &other) const
{
    return !(*this == other);
}

FlatpakFilesystemsEntry::FlatpakFilesystemsEntry(FilesystemPrefix prefix, AccessMode mode, const QString &path)
    : m_prefix(prefix)
    , m_mode(mode)
    , m_path(path)
{
}

std::optional<FlatpakFilesystemsEntry> FlatpakFilesystemsEntry::parse(QStringView entry)
{
    std::optional<AccessMode> accessMode = std::nullopt;

    if (entry.endsWith(SUFFIX_RO)) {
        entry.chop(SUFFIX_RO.size());
        accessMode = std::optional(AccessMode::ReadOnly);
    } else if (entry.endsWith(SUFFIX_RW)) {
        entry.chop(SUFFIX_RW.size());
        accessMode = std::optional(AccessMode::ReadWrite);
    } else if (entry.endsWith(SUFFIX_CREATE)) {
        entry.chop(SUFFIX_CREATE.size());
        accessMode = std::optional(AccessMode::Create);
    }

    if (entry.startsWith(PREFIX_DENY)) {
        // ensure there is no access mode suffix
        if (accessMode.has_value()) {
            return std::nullopt;
        }
        entry = entry.mid(1);
        accessMode = std::optional(AccessMode::Deny);
    }

    AccessMode effectiveAccessMode = accessMode.value_or(AccessMode::ReadWrite);

    return parse(entry, effectiveAccessMode);
}

std::optional<FlatpakFilesystemsEntry> FlatpakFilesystemsEntry::parse(QStringView name, AccessMode accessMode)
{
    for (const TableEntry &filesystem : s_filesystems) {
        // Deliberately not using switch here, because of the overlapping Optional case.
        if (filesystem.mode == PathMode::Optional || filesystem.mode == PathMode::Required) {
            if (name.startsWith(filesystem.prefixString) || filesystem.prefix == FilesystemPrefix::Unknown) {
                if (filesystem.prefix != FilesystemPrefix::Unknown) {
                    name = name.mid(filesystem.prefixString.size());
                }
                QString path;
                if (!name.isEmpty()) {
                    path = QUrl(name.toString()).toDisplayString(QUrl::RemoveScheme | QUrl::StripTrailingSlash);
                } else if (filesystem.mode == PathMode::Required) {
                    return std::nullopt;
                }
                return std::optional(FlatpakFilesystemsEntry(filesystem.prefix, accessMode, path));
            }
        }
        if (filesystem.mode == PathMode::NoPath || filesystem.mode == PathMode::Optional) {
            if (name == filesystem.fixedString) {
                return std::optional(FlatpakFilesystemsEntry(filesystem.prefix, accessMode, QString()));
            }
        }
    }

    return std::nullopt;
}

QString FlatpakFilesystemsEntry::name() const
{
    const auto it = std::find_if(s_filesystems.begin(), s_filesystems.end(), [this](const TableEntry &filesystem) {
        if (filesystem.prefix != m_prefix) {
            return false;
        }
        // home/path should be serialized as ~/path, and fixed string "~" as "home".
        // So either the path should be empty, XOR current table entry is the "~" variant.
        if (filesystem.prefix == FilesystemPrefix::Home) {
            return m_path.isEmpty() != (filesystem.fixedString == QLatin1String("~"));
        }
        return true;
    });
    if (it == s_filesystems.end()) {
        // all prefixes are covered in the table, so this should never happen.
        Q_UNREACHABLE();
        return {};
    }

    if ((m_path.isEmpty() && it->mode == PathMode::Required) || (!m_path.isEmpty() && it->mode == PathMode::NoPath)) {
        return {};
    }

    return m_path.isEmpty() ? QString(it->fixedString) : it->prefixString + m_path;
}

QString FlatpakFilesystemsEntry::format() const
{
    const QString path = name();
    if (path.isEmpty()) {
        return {};
    }

    switch (m_mode) {
    case AccessMode::ReadOnly:
        return path + SUFFIX_RO;
    case AccessMode::ReadWrite:
        // Omit default value
        return path;
    case AccessMode::Create:
        return path + SUFFIX_CREATE;
    case AccessMode::Deny:
        return PREFIX_DENY + path;
    }
    return {};
}

FlatpakFilesystemsEntry::FilesystemPrefix FlatpakFilesystemsEntry::prefix() const
{
    return m_prefix;
}

QString FlatpakFilesystemsEntry::path() const
{
    return m_path;
}

FlatpakFilesystemsEntry::AccessMode FlatpakFilesystemsEntry::mode() const
{
    return m_mode;
}

bool FlatpakFilesystemsEntry::operator==(const FlatpakFilesystemsEntry &other) const
{
    return other.m_prefix == m_prefix && other.m_mode == m_mode && other.m_path == m_path;
}

bool FlatpakFilesystemsEntry::operator!=(const FlatpakFilesystemsEntry &other) const
{
    return !(*this == other);
}

PolicyChoicesModel::PolicyChoicesModel(QList<Entry> &&policies, QObject *parent)
    : QAbstractListModel(parent)
    , m_policies(policies)
{
}

QHash<int, QByteArray> PolicyChoicesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Roles::ValueRole, "value"},
    };
}

int PolicyChoicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_policies.count();
}

QVariant PolicyChoicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_policies.count()) {
        return {};
    }

    const auto policy = m_policies.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return policy.display;
    case Roles::ValueRole:
        return policy.value;
    }

    return {};
}

FilesystemChoicesModel::FilesystemChoicesModel(QObject *parent)
    : PolicyChoicesModel(
        QList<Entry>{
            {static_cast<int>(FlatpakFilesystemsEntry::AccessMode::ReadOnly), i18n("read-only")},
            {static_cast<int>(FlatpakFilesystemsEntry::AccessMode::ReadWrite), i18n("read/write")},
            {static_cast<int>(FlatpakFilesystemsEntry::AccessMode::Create), i18n("create")},
            {static_cast<int>(FlatpakFilesystemsEntry::AccessMode::Deny), i18n("OFF")},
        },
        parent)
{
}

DBusPolicyChoicesModel::DBusPolicyChoicesModel(QObject *parent)
    : PolicyChoicesModel(
        QList<Entry>{
            {FlatpakPolicy::FLATPAK_POLICY_NONE, i18n("None")},
            {FlatpakPolicy::FLATPAK_POLICY_SEE, i18n("see")},
            {FlatpakPolicy::FLATPAK_POLICY_TALK, i18n("talk")},
            {FlatpakPolicy::FLATPAK_POLICY_OWN, i18n("own")},
        },
        parent)
{
}

Q_GLOBAL_STATIC(FilesystemChoicesModel, s_FilesystemPolicies);
Q_GLOBAL_STATIC(DBusPolicyChoicesModel, s_DBusPolicies);

FlatpakPermission::ValueType FlatpakPermission::valueTypeFromSectionType(FlatpakPermissionsSectionType::Type section)
{
    switch (section) {
    case FlatpakPermissionsSectionType::Filesystems:
        return FlatpakPermission::ValueType::Filesystems;
    case FlatpakPermissionsSectionType::SessionBus:
    case FlatpakPermissionsSectionType::SystemBus:
        return FlatpakPermission::ValueType::Bus;
    case FlatpakPermissionsSectionType::Environment:
        return FlatpakPermission::ValueType::Environment;
    case FlatpakPermissionsSectionType::Basic:
    case FlatpakPermissionsSectionType::Advanced:
    case FlatpakPermissionsSectionType::SubsystemsShared:
    case FlatpakPermissionsSectionType::Sockets:
    case FlatpakPermissionsSectionType::Devices:
    case FlatpakPermissionsSectionType::Features:
        break;
    }
    return FlatpakPermission::ValueType::Simple;
}

FlatpakPermission::FlatpakPermission(FlatpakPermissionsSectionType::Type section)
    : FlatpakPermission(section, QString(), QString(), QString(), false)
{
    m_originType = OriginType::Dummy;
}

FlatpakPermission::FlatpakPermission(FlatpakPermissionsSectionType::Type section,
                                     const QString &name,
                                     const QString &category,
                                     const QString &description,
                                     bool isDefaultEnabled,
                                     const Variant &defaultValue)
    : m_section(section)
    , m_name(name)
    , m_category(category)
    , m_description(description)
    //
    , m_originType(OriginType::BuiltIn)
    //
    , m_defaultEnable(isDefaultEnabled)
    , m_overrideEnable(isDefaultEnabled)
    , m_effectiveEnable(isDefaultEnabled)
    //
    , m_defaultValue(defaultValue)
    , m_overrideValue(defaultValue)
    , m_effectiveValue(defaultValue)
{
}

FlatpakPermissionsSectionType::Type FlatpakPermission::section() const
{
    return m_section;
}

const QString &FlatpakPermission::name() const
{
    return m_name;
}

const QString &FlatpakPermission::category() const
{
    return m_category;
}

const QString &FlatpakPermission::description() const
{
    return m_description;
}

FlatpakPermission::ValueType FlatpakPermission::valueType() const
{
    return valueTypeFromSectionType(m_section);
}

FlatpakPermission::OriginType FlatpakPermission::originType() const
{
    return m_originType;
}

void FlatpakPermission::setOriginType(OriginType type)
{
    m_originType = type;
}

bool FlatpakPermission::isDefaultEnabled() const
{
    return m_defaultEnable;
}

void FlatpakPermission::setOverrideEnabled(bool enabled)
{
    m_overrideEnable = enabled;
}

bool FlatpakPermission::canBeDisabled() const
{
    return valueType() == ValueType::Simple || !m_defaultEnable;
}

bool FlatpakPermission::isEffectiveEnabled() const
{
    return m_effectiveEnable;
}

void FlatpakPermission::setEffectiveEnabled(bool enabled)
{
    if (canBeDisabled()) {
        m_effectiveEnable = enabled;
    }
}

const FlatpakPermission::Variant FlatpakPermission::defaultValue() const
{
    return m_defaultValue;
}

void FlatpakPermission::setDefaultValue(const Variant &value)
{
    m_defaultValue = value;
}

void FlatpakPermission::setOverrideValue(const Variant &value)
{
    m_overrideValue = value;
}

const FlatpakPermission::Variant FlatpakPermission::effectiveValue() const
{
    return m_effectiveValue;
}

void FlatpakPermission::setEffectiveValue(const Variant &value)
{
    m_effectiveValue = value;
}

bool FlatpakPermission::isSaveNeeded() const
{
    if (m_originType == FlatpakPermission::OriginType::Dummy) {
        return false;
    }

    const bool enableDiffers = m_effectiveEnable != m_overrideEnable;
    if (valueType() != FlatpakPermission::ValueType::Simple) {
        const bool valueDiffers = m_effectiveValue != m_overrideValue;
        return enableDiffers || valueDiffers;
    }
    return enableDiffers;
}

bool FlatpakPermission::isDefaults() const
{
    if (m_originType == FlatpakPermission::OriginType::Dummy) {
        return true;
    }

    const auto enableIsTheSame = m_effectiveEnable == m_defaultEnable;
    if (valueType() != FlatpakPermission::ValueType::Simple) {
        const auto customEntryIsMarkedForRemoval = !m_defaultEnable && !m_effectiveEnable;
        const auto valueIsTheSame = m_effectiveValue == m_defaultValue;
        // For disabled custom entries (i.e. marked for removal) value does not matter.
        return customEntryIsMarkedForRemoval || (enableIsTheSame && valueIsTheSame);
    }
    return enableIsTheSame;
}

static QString mapDBusFlatpakPolicyEnumValueToConfigString(FlatpakPolicy value)
{
    switch (value) {
    case FlatpakPolicy::FLATPAK_POLICY_SEE:
        return QStringLiteral("see");
    case FlatpakPolicy::FLATPAK_POLICY_TALK:
        return QStringLiteral("talk");
    case FlatpakPolicy::FLATPAK_POLICY_OWN:
        return QStringLiteral("own");
    case FlatpakPolicy::FLATPAK_POLICY_NONE:
        break;
    }
    return QStringLiteral("none");
}

static FlatpakPolicy mapDBusFlatpakPolicyConfigStringToEnumValue(const QString &value)
{
    if (value == QStringLiteral("see")) {
        return FlatpakPolicy::FLATPAK_POLICY_SEE;
    }
    if (value == QStringLiteral("talk")) {
        return FlatpakPolicy::FLATPAK_POLICY_TALK;
    }
    if (value == QStringLiteral("own")) {
        return FlatpakPolicy::FLATPAK_POLICY_OWN;
    }
    if (value != QStringLiteral("none")) {
        qWarning() << "Unsupported Flatpak D-Bus policy:" << value;
    }
    return FlatpakPolicy::FLATPAK_POLICY_NONE;
}

namespace FlatpakOverrides
{

KConfigPtr loadAndMerge(const QStringList &filenames)
{
    auto config = std::make_unique<KConfig>(QString(), KConfig::SimpleConfig);
    for (const auto &filename : filenames) {
        merge(*config, filename);
    }
    return config;
}

void merge(KConfig &target, const QString &filename)
{
    if (!QFileInfo::exists(filename)) {
        return;
    }
    const KConfig config(filename, KConfig::SimpleConfig);
    merge(target, config);
}

void merge(KConfig &target, const KConfig &source)
{
    /***/ auto targetContextGroup = target.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
    const auto sourceContextGroup = source.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));

    const std::array simpleCategories = {
        QLatin1String(FLATPAK_METADATA_KEY_SHARED),
        QLatin1String(FLATPAK_METADATA_KEY_SOCKETS),
        QLatin1String(FLATPAK_METADATA_KEY_DEVICES),
        QLatin1String(FLATPAK_METADATA_KEY_FEATURES),
    };

    for (const auto &category : simpleCategories) {
        const auto targetEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(targetContextGroup, category);
        const auto sourceEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(sourceContextGroup, category);

        QMap<QString, bool> entriesMap;

        for (const auto &entries : {targetEntries, sourceEntries}) {
            for (const auto &entry : entries) {
                entriesMap.insert(entry.name(), entry.isEnabled());
            }
        }

        QStringList entriesList;

        for (auto it = entriesMap.constKeyValueBegin(); it != entriesMap.constKeyValueEnd(); it++) {
            const auto [name, enabled] = *it;
            entriesList.append(FlatpakSimpleEntry(name, enabled).format());
        }

        targetContextGroup.writeXdgListEntry(category, entriesList, KConfig::WriteConfigFlags{});
    }

    /* Filesystems */
    {
        const auto category = QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS);

        const auto targetRawFilesystems = targetContextGroup.readXdgListEntry(category);
        const auto sourceRawFilesystems = sourceContextGroup.readXdgListEntry(category);

        QMap<QString, FlatpakFilesystemsEntry::AccessMode> entriesMap;

        for (const auto &rawFilesystems : {targetRawFilesystems, sourceRawFilesystems}) {
            const auto entries = filter_map(rawFilesystems, [](const QString &entry) {
                return FlatpakFilesystemsEntry::parse(entry);
            });
            for (const auto &entry : entries) {
                entriesMap.insert(entry.name(), entry.mode());
            }
        }

        QStringList entriesList;

        for (auto it = entriesMap.constKeyValueBegin(); it != entriesMap.constKeyValueEnd(); it++) {
            const auto [name, accessMode] = *it;
            if (const auto entry = FlatpakFilesystemsEntry::parse(name, accessMode); entry.has_value()) {
                entriesList.append(entry.value().format());
            }
        }

        targetContextGroup.writeXdgListEntry(category, entriesList, KConfig::WriteConfigFlags());
    }

    const std::array categories = {
        QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
        QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
        QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
    };
    for (const auto &category : categories) {
        /***/ auto targetGroup = target.group(category);
        const auto sourceGroup = source.group(category);

        QMap<QString, QString> entriesMap;

        entriesMap.insert(targetGroup.entryMap());
        entriesMap.insert(sourceGroup.entryMap());

        for (auto it = entriesMap.constKeyValueBegin(); it != entriesMap.constKeyValueEnd(); it++) {
            const auto &[key, value] = *it;
            targetGroup.writeEntry(key, value, KConfig::WriteConfigFlags());
        }
    }
}

};

FlatpakPermissionModel::FlatpakPermissionModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_showAdvanced(false)
{
}

int FlatpakPermissionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return rowCount(m_showAdvanced);
}

QVariant FlatpakPermissionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const auto permission = m_permissions.at(index.row());

    switch (role) {
    case Roles::Section:
        return permission.section();
    case Roles::Name:
        return permission.name();
    case Roles::Description:
        return permission.description();
    //
    case Roles::IsNotDummy:
        return permission.originType() != FlatpakPermission::OriginType::Dummy;
    //
    case Roles::CanBeDisabled:
        return permission.canBeDisabled();
    case Roles::IsDefaultEnabled:
        return permission.isDefaultEnabled();
    case Roles::IsEffectiveEnabled:
        return permission.isEffectiveEnabled();
    case Roles::DefaultValue:
        return QVariant::fromStdVariant(permission.defaultValue());
    case Roles::EffectiveValue:
        return QVariant::fromStdVariant(permission.effectiveValue());
    //
    case Roles::ValuesModel:
        return QVariant::fromValue(FlatpakPermissionModel::valuesModelForSectionType(permission.section()));
    }

    return QVariant();
}

QHash<int, QByteArray> FlatpakPermissionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    //
    roles[Roles::Section] = "section";
    roles[Roles::Name] = "name";
    roles[Roles::Description] = "description";
    //
    roles[Roles::IsNotDummy] = "isNotDummy";
    //
    roles[Roles::CanBeDisabled] = "canBeDisabled";
    roles[Roles::IsDefaultEnabled] = "isDefaultEnabled";
    roles[Roles::IsEffectiveEnabled] = "isEffectiveEnabled";
    roles[Roles::DefaultValue] = "defaultValue";
    roles[Roles::EffectiveValue] = "effectiveValue";
    //
    roles[Roles::ValuesModel] = "valuesModel";
    return roles;
}

void FlatpakPermissionModel::loadDefaultValues()
{
    const auto defaults = m_reference->defaultsFiles();
    auto parser = FlatpakOverrides::loadAndMerge(defaults);

    const auto contextGroup = parser->group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));

    QString category;
    QList<FlatpakSimpleEntry> simpleEntries;
    int basicIndex = 0;

    const auto insertSimpleEntry = [&](FlatpakPermissionsSectionType::Type section, const QString &name, const QString &description) {
        const auto isEnabledByDefault = FlatpakSimpleEntry::isEnabled(simpleEntries, name).value_or(false);
        const auto permission = FlatpakPermission(section, name, category, description, isEnabledByDefault);
        if (section == FlatpakPermissionsSectionType::Basic) {
            m_permissions.insert(basicIndex, permission);
            basicIndex += 1;
        } else {
            m_permissions.append(permission);
        }
    };

    /* SHARED category */
    category = QLatin1String(FLATPAK_METADATA_KEY_SHARED);
    simpleEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(contextGroup, category);
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("network"), i18n("Internet Connection"));
    insertSimpleEntry(FlatpakPermissionsSectionType::SubsystemsShared, QStringLiteral("ipc"), i18n("Inter-process Communication"));
    /* SHARED category */

    /* SOCKETS category */
    category = QLatin1String(FLATPAK_METADATA_KEY_SOCKETS);
    simpleEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(contextGroup, category);
    insertSimpleEntry(FlatpakPermissionsSectionType::Sockets, QStringLiteral("x11"), i18n("X11 Windowing System"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Sockets, QStringLiteral("wayland"), i18n("Wayland Windowing System"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Sockets, QStringLiteral("fallback-x11"), i18n("Fallback to X11 Windowing System"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("pulseaudio"), i18n("Pulseaudio Sound Server"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Sockets, QStringLiteral("session-bus"), i18n("Session Bus Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Sockets, QStringLiteral("system-bus"), i18n("System Bus Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("ssh-auth"), i18n("Remote Login Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("pcsc"), i18n("Smart Card Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("cups"), i18n("Print System Access"));
    /* SOCKETS category */

    /* DEVICES category */
    category = QLatin1String(FLATPAK_METADATA_KEY_DEVICES);
    simpleEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(contextGroup, category);
    insertSimpleEntry(FlatpakPermissionsSectionType::Devices, QStringLiteral("kvm"), i18n("Kernel-based Virtual Machine Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Devices, QStringLiteral("dri"), i18n("Direct Graphic Rendering"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Devices, QStringLiteral("shm"), i18n("Host dev/shm"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("all"), i18n("Device Access"));
    /* DEVICES category */

    /* FEATURES category */
    category = QLatin1String(FLATPAK_METADATA_KEY_FEATURES);
    simpleEntries = FlatpakSimpleEntry::getCategorySkippingInvalidEntries(contextGroup, category);
    insertSimpleEntry(FlatpakPermissionsSectionType::Features, QStringLiteral("devel"), i18n("System Calls by Development Tools"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Features, QStringLiteral("multiarch"), i18n("Run Multiarch/Multilib Binaries"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Basic, QStringLiteral("bluetooth"), i18n("Bluetooth"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Features, QStringLiteral("canbus"), i18n("Canbus Socket Access"));
    insertSimpleEntry(FlatpakPermissionsSectionType::Features,
                      QStringLiteral("per-app-dev-shm"),
                      i18n("Share dev/shm across all instances of an app per user ID"));
    /* FEATURES category */

    /* FILESYSTEM category */
    category = QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS);
    const auto rawFilesystems = contextGroup.readXdgListEntry(category);
    const auto filesystems = filter_map(rawFilesystems, [](const QString &entry) {
        return FlatpakFilesystemsEntry::parse(entry);
    });

    std::optional<FlatpakFilesystemsEntry::AccessMode> homeVal = std::nullopt;
    std::optional<FlatpakFilesystemsEntry::AccessMode> hostVal = std::nullopt;
    std::optional<FlatpakFilesystemsEntry::AccessMode> hostOsVal = std::nullopt;
    std::optional<FlatpakFilesystemsEntry::AccessMode> hostEtcVal = std::nullopt;

    QList<FlatpakFilesystemsEntry> nonStandardFilesystems;

    static const auto ignoredFilesystems = QList<FlatpakFilesystemsEntry>{
        FlatpakFilesystemsEntry(FlatpakFilesystemsEntry::FilesystemPrefix::XdgConfig,
                                FlatpakFilesystemsEntry::AccessMode::ReadOnly,
                                QLatin1String("kdeglobals")),
    };

    for (const auto &filesystem : filesystems) {
        if (ignoredFilesystems.contains(filesystem)) {
            continue;
        }

        if (filesystem.path().isEmpty()) {
            switch (filesystem.prefix()) {
            case FlatpakFilesystemsEntry::FilesystemPrefix::Home:
                homeVal = filesystem.mode();
                continue;
            case FlatpakFilesystemsEntry::FilesystemPrefix::Host:
                hostVal = filesystem.mode();
                continue;
            case FlatpakFilesystemsEntry::FilesystemPrefix::HostOs:
                hostOsVal = filesystem.mode();
                continue;
            case FlatpakFilesystemsEntry::FilesystemPrefix::HostEtc:
                hostEtcVal = filesystem.mode();
                continue;
            default:
                break;
            }
        }
        nonStandardFilesystems.append(filesystem);
    }

    const auto insertStandardFilesystemsEntry =
        [&](const QString &name, const QString &description, std::optional<FlatpakFilesystemsEntry::AccessMode> accessMode) {
            const auto enabled = accessMode.has_value();
            const auto effectiveAccessMode = accessMode.value_or(FlatpakFilesystemsEntry::AccessMode::ReadOnly);
            m_permissions.insert(basicIndex,
                                 FlatpakPermission(FlatpakPermissionsSectionType::Filesystems, name, category, description, enabled, effectiveAccessMode));
            basicIndex += 1;
        };

    insertStandardFilesystemsEntry(QStringLiteral("home"), i18n("All User Files"), homeVal);
    insertStandardFilesystemsEntry(QStringLiteral("host"), i18n("All System Files"), hostVal);
    insertStandardFilesystemsEntry(QStringLiteral("host-os"), i18n("All System Libraries, Executables and Binaries"), hostOsVal);
    insertStandardFilesystemsEntry(QStringLiteral("host-etc"), i18n("All System Configurations"), hostEtcVal);

    for (const auto &filesystem : std::as_const(nonStandardFilesystems)) {
        const auto name = filesystem.name();
        const auto accessMode = filesystem.mode();
        m_permissions.insert(basicIndex, FlatpakPermission(FlatpakPermissionsSectionType::Filesystems, name, category, name, true, accessMode));
        basicIndex += 1;
    }
    /* FILESYSTEM category */

    /* DUMMY ADVANCED category */
    m_permissions.insert(basicIndex, FlatpakPermission(FlatpakPermissionsSectionType::Advanced));
    basicIndex += 1;
    /* DUMMY ADVANCED category */

    /* SESSION BUS category */
    {
        category = QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY);
        const auto group = parser->group(category);
        if (const auto keys = group.keyList(); !keys.isEmpty()) {
            for (const auto &name : keys) {
                const auto policyString = group.readEntry(name);
                const auto policyValue = mapDBusFlatpakPolicyConfigStringToEnumValue(policyString);
                m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::SessionBus, name, category, name, true, policyValue));
            }
        } else {
            m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::SessionBus));
        }
    }
    /* SESSION BUS category */

    /* SYSTEM BUS category */
    {
        category = QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY);
        const auto group = parser->group(category);
        if (const auto keys = group.keyList(); !keys.isEmpty()) {
            for (const auto &name : keys) {
                const auto policyString = group.readEntry(name);
                const auto policyValue = mapDBusFlatpakPolicyConfigStringToEnumValue(policyString);
                m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::SystemBus, name, category, name, true, policyValue));
            }
        } else {
            m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::SystemBus));
        }
    }
    /* SYSTEM BUS category */

    /* ENVIRONMENT category */
    {
        category = QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT);
        const auto group = parser->group(category);
        if (const auto keys = group.keyList(); !keys.isEmpty()) {
            for (const auto &name : keys) {
                const auto value = group.readEntry(name);
                m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::Environment, name, category, name, true, value));
            }
        } else {
            m_permissions.append(FlatpakPermission(FlatpakPermissionsSectionType::Environment));
        }
    }
    /* ENVIRONMENT category */
}

void FlatpakPermissionModel::loadCurrentValues()
{
    const auto &userAppOverrides = m_reference->userLevelPerAppOverrideFile();

    /* all permissions are at default, so nothing to load */
    if (!QFileInfo::exists(userAppOverrides)) {
        return;
    }

    const KConfig parser(userAppOverrides, KConfig::SimpleConfig);
    const auto contextGroup = parser.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));

    // Mapping to valid entries. Invalid ones go into m_unparsableEntriesByCategory.
    QHash<QString, QList<FlatpakSimpleEntry>> entriesByCategory;

    const std::array simpleCategories = {
        QLatin1String(FLATPAK_METADATA_KEY_SHARED),
        QLatin1String(FLATPAK_METADATA_KEY_SOCKETS),
        QLatin1String(FLATPAK_METADATA_KEY_DEVICES),
        QLatin1String(FLATPAK_METADATA_KEY_FEATURES),
    };

    for (const auto &category : simpleCategories) {
        const auto [unparsable, entries] = FlatpakSimpleEntry::getCategory(contextGroup, category);
        if (!unparsable.isEmpty()) {
            m_unparsableEntriesByCategory.insert(category, unparsable);
        }
        if (!entries.isEmpty()) {
            entriesByCategory.insert(category, entries);
        }
    }

    const auto rawFilesystems = contextGroup.readXdgListEntry(QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS));
    const auto [unparsableFilesystems, filesystems] = try_filter_map(rawFilesystems, [](const QString &entry) {
        return FlatpakFilesystemsEntry::parse(entry);
    });
    if (!unparsableFilesystems.isEmpty()) {
        m_unparsableEntriesByCategory.insert(QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), unparsableFilesystems);
    }

    int fsIndex = -1;

    for (int i = 0; i < m_permissions.length(); ++i) {
        FlatpakPermission &permission = m_permissions[i];

        switch (permission.valueType()) {
        case FlatpakPermission::ValueType::Simple: {
            if (!entriesByCategory.contains(permission.category())) {
                continue;
            }
            const auto &entries = entriesByCategory[permission.category()];
            if (const auto entry = FlatpakSimpleEntry::isEnabled(entries, permission.name()); entry.has_value()) {
                const auto isEnabled = entry.value();
                permission.setEffectiveEnabled(isEnabled);
                permission.setOverrideEnabled(isEnabled);
            }
            break;
        }
        case FlatpakPermission::ValueType::Filesystems: {
            const auto it = std::find_if(filesystems.constBegin(), filesystems.constEnd(), [=](const FlatpakFilesystemsEntry &filesystem) {
                return filesystem.name() == permission.name();
            });
            if (it != filesystems.constEnd()) {
                const auto &filesystem = *it;

                permission.setOverrideEnabled(true);
                permission.setEffectiveEnabled(true);

                permission.setOverrideValue(filesystem.mode());
                permission.setEffectiveValue(filesystem.mode());
            }
            fsIndex = i + 1;
            break;
        }
        case FlatpakPermission::ValueType::Bus:
        case FlatpakPermission::ValueType::Environment: {
            const auto group = parser.group(permission.category());
            if (group.hasKey(permission.name())) {
                const auto value = group.readEntry(permission.name());

                if (permission.valueType() == FlatpakPermission::ValueType::Bus) {
                    const auto policyValue = mapDBusFlatpakPolicyConfigStringToEnumValue(value);
                    permission.setOverrideValue(policyValue);
                    permission.setEffectiveValue(policyValue);
                } else {
                    permission.setOverrideValue(value);
                    permission.setEffectiveValue(value);
                }

                permission.setOverrideEnabled(true);
                permission.setEffectiveEnabled(true);
            }
            break;
        }
        } // end of switch
    }

    if (!filesystems.isEmpty()) {
        const auto section = FlatpakPermissionsSectionType::Filesystems;
        const auto category = QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS);
        for (const auto &filesystem : filesystems) {
            const auto name = filesystem.name();
            const auto accessMode = filesystem.mode();
            if (!permissionExists(section, name)) {
                m_permissions.insert(fsIndex, FlatpakPermission(section, name, category, name, false, accessMode));
                m_permissions[fsIndex].setOverrideEnabled(true);
                m_permissions[fsIndex].setEffectiveEnabled(true);
                fsIndex++;
            }
        }
    }

    const std::array categories = {
        std::make_tuple(FlatpakPermissionsSectionType::SessionBus, QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY)),
        std::make_tuple(FlatpakPermissionsSectionType::SystemBus, QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY)),
        std::make_tuple(FlatpakPermissionsSectionType::Environment, QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT)),
    };
    for (const auto &[section, category] : categories) {
        const auto group = parser.group(category);
        if (!group.exists()) {
            continue;
        }
        const auto valueType = FlatpakPermission::valueTypeFromSectionType(section);
        // Model signals are not needed during load/reset.
        int insertIndex = findIndexToInsertRowAndRemoveDummyRowIfNeeded(section, false);

        const auto keys = group.keyList();
        for (const auto &name : keys) {
            if (!permissionExists(section, name)) {
                const auto value = group.readEntry(name);

                if (valueType == FlatpakPermission::ValueType::Bus) {
                    const auto policyValue = mapDBusFlatpakPolicyConfigStringToEnumValue(value);
                    m_permissions.insert(insertIndex, FlatpakPermission(section, name, category, name, false, policyValue));
                } else {
                    m_permissions.insert(insertIndex, FlatpakPermission(section, name, category, name, false, value));
                }

                m_permissions[insertIndex].setOverrideEnabled(true);
                m_permissions[insertIndex].setEffectiveEnabled(true);
                insertIndex++;
            }
        }
    }
}

FlatpakReference *FlatpakPermissionModel::reference() const
{
    return m_reference;
}

void FlatpakPermissionModel::setReference(FlatpakReference *reference)
{
    if (reference != m_reference) {
        beginResetModel();
        if (m_reference) {
            m_reference->setPermissionsModel(nullptr);
        }
        m_reference = reference;
        if (m_reference) {
            m_reference->setPermissionsModel(this);
        }
        endResetModel();
        Q_EMIT referenceChanged();
    }
}

bool FlatpakPermissionModel::showAdvanced() const
{
    return m_showAdvanced;
}

void FlatpakPermissionModel::setShowAdvanced(bool show)
{
    if (m_showAdvanced != show) {
        const int whenHidden = rowCount(false);
        const int whenShown = rowCount(true);

        if (show) {
            beginInsertRows(QModelIndex(), whenHidden, whenShown - 1);
        } else {
            beginRemoveRows(QModelIndex(), whenHidden, whenShown - 1);
        }

        m_showAdvanced = show;

        if (show) {
            endInsertRows();
        } else {
            endRemoveRows();
        }

        Q_EMIT showAdvancedChanged();
    }
}

int FlatpakPermissionModel::rowCount(bool showAdvanced) const
{
    if (showAdvanced) {
        return m_permissions.count();
    } else {
        int count = 0;
        for (const auto &permission : m_permissions) {
            if (permission.section() == FlatpakPermissionsSectionType::Basic || permission.section() == FlatpakPermissionsSectionType::Filesystems
                || permission.section() == FlatpakPermissionsSectionType::Advanced) {
                count += 1;
            } else {
                break;
            }
        }
        return count;
    }
}

void FlatpakPermissionModel::load()
{
    beginResetModel();
    {
        m_permissions.clear();
        m_unparsableEntriesByCategory.clear();
        loadDefaultValues();
        loadCurrentValues();
    }
    endResetModel();
}

void FlatpakPermissionModel::save()
{
    for (auto &permission : m_permissions) {
        permission.setOverrideEnabled(permission.isEffectiveEnabled());
        if (permission.valueType() != FlatpakPermission::ValueType::Simple) {
            permission.setOverrideValue(permission.effectiveValue());
            if (!permission.isDefaultEnabled()) {
                permission.setDefaultValue(permission.effectiveValue());
            }
        }
    }
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {Roles::DefaultValue});
    writeToFile();
}

void FlatpakPermissionModel::defaults()
{
    for (auto &permission : m_permissions) {
        permission.setEffectiveEnabled(permission.isDefaultEnabled());
        if (permission.valueType() != FlatpakPermission::ValueType::Simple) {
            permission.setEffectiveValue(permission.defaultValue());
        }
    }
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {Roles::IsEffectiveEnabled, Roles::EffectiveValue});
}

bool FlatpakPermissionModel::isDefaults() const
{
    return std::all_of(m_permissions.constBegin(), m_permissions.constEnd(), [](const FlatpakPermission &permission) {
        return permission.isDefaults();
    });
}

bool FlatpakPermissionModel::isSaveNeeded() const
{
    return std::any_of(m_permissions.constBegin(), m_permissions.constEnd(), [](const FlatpakPermission &permission) {
        return permission.isSaveNeeded();
    });
}

PolicyChoicesModel *FlatpakPermissionModel::valuesModelForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection)
{
    if (!QMetaEnum::fromType<FlatpakPermissionsSectionType::Type>().valueToKey(rawSection)) {
        return {};
    }
    // SAFETY: QMetaEnum above ensures that coercion is valid.
    const auto section = static_cast<FlatpakPermissionsSectionType::Type>(rawSection);

    switch (section) {
    case FlatpakPermissionsSectionType::Filesystems:
        return valuesModelForFilesystemsSection();
    case FlatpakPermissionsSectionType::SessionBus:
    case FlatpakPermissionsSectionType::SystemBus:
        return valuesModelForBusSections();
    case FlatpakPermissionsSectionType::Basic:
    case FlatpakPermissionsSectionType::Advanced:
    case FlatpakPermissionsSectionType::SubsystemsShared:
    case FlatpakPermissionsSectionType::Sockets:
    case FlatpakPermissionsSectionType::Devices:
    case FlatpakPermissionsSectionType::Features:
    case FlatpakPermissionsSectionType::Environment:
        break;
    }

    return {};
}

PolicyChoicesModel *FlatpakPermissionModel::valuesModelForFilesystemsSection()
{
    QQmlEngine::setObjectOwnership(s_FilesystemPolicies, QQmlEngine::CppOwnership);
    return s_FilesystemPolicies;
}

PolicyChoicesModel *FlatpakPermissionModel::valuesModelForBusSections()
{
    QQmlEngine::setObjectOwnership(s_DBusPolicies, QQmlEngine::CppOwnership);
    return s_DBusPolicies;
}

Q_INVOKABLE QString FlatpakPermissionModel::sectionHeaderForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection)
{
    if (!QMetaEnum::fromType<FlatpakPermissionsSectionType::Type>().valueToKey(rawSection)) {
        return {};
    }
    // SAFETY: QMetaEnum above ensures that coercion is valid.
    const auto section = static_cast<FlatpakPermissionsSectionType::Type>(rawSection);

    switch (section) {
    case FlatpakPermissionsSectionType::Basic:
        return i18n("Basic Permissions");
    case FlatpakPermissionsSectionType::Filesystems:
        return i18n("Filesystem Access");
    case FlatpakPermissionsSectionType::Advanced:
        return i18n("Advanced Permissions");
    case FlatpakPermissionsSectionType::SubsystemsShared:
        return i18n("Subsystems Shared");
    case FlatpakPermissionsSectionType::Sockets:
        return i18n("Sockets");
    case FlatpakPermissionsSectionType::Devices:
        return i18n("Device Access");
    case FlatpakPermissionsSectionType::Features:
        return i18n("Features Allowed");
    case FlatpakPermissionsSectionType::SessionBus:
        return i18n("Session Bus Policy");
    case FlatpakPermissionsSectionType::SystemBus:
        return i18n("System Bus Policy");
    case FlatpakPermissionsSectionType::Environment:
        return i18n("Environment");
    }

    Q_UNREACHABLE();
    return {};
}

Q_INVOKABLE QString FlatpakPermissionModel::sectionAddButtonToolTipTextForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection)
{
    if (!QMetaEnum::fromType<FlatpakPermissionsSectionType::Type>().valueToKey(rawSection)) {
        return {};
    }
    // SAFETY: QMetaEnum above ensures that coercion is valid.
    const auto section = static_cast<FlatpakPermissionsSectionType::Type>(rawSection);

    switch (section) {
    case FlatpakPermissionsSectionType::Filesystems:
        return i18n("Add a new filesystem path");
    case FlatpakPermissionsSectionType::SessionBus:
        return i18n("Add a new session bus");
    case FlatpakPermissionsSectionType::SystemBus:
        return i18n("Add a new system bus");
    case FlatpakPermissionsSectionType::Environment:
        return i18n("Add a new environment variable");
    case FlatpakPermissionsSectionType::Basic:
    case FlatpakPermissionsSectionType::Advanced:
    case FlatpakPermissionsSectionType::SubsystemsShared:
    case FlatpakPermissionsSectionType::Sockets:
    case FlatpakPermissionsSectionType::Devices:
    case FlatpakPermissionsSectionType::Features:
        break;
    }

    return {};
}

bool FlatpakPermissionModel::permissionExists(int /*FlatpakPermissionsSectionType::Type*/ rawSection, const QString &name) const
{
    if (!QMetaEnum::fromType<FlatpakPermissionsSectionType::Type>().valueToKey(rawSection)) {
        return false;
    }
    // SAFETY: QMetaEnum above ensures that coercion is valid.
    const auto section = static_cast<FlatpakPermissionsSectionType::Type>(rawSection);

    return permissionExists(section, name);
}

bool FlatpakPermissionModel::permissionExists(FlatpakPermissionsSectionType::Type section, const QString &name) const
{
    return findPermissionRow(section, name).has_value();
}

std::optional<int> FlatpakPermissionModel::findPermissionRow(FlatpakPermissionsSectionType::Type section, const QString &name) const
{
    for (int i = 0; i < m_permissions.length(); i++) {
        const auto &permission = m_permissions[i];
        if (permission.section() == section && permission.name() == name) {
            return std::optional(i);
        }
    }
    return std::nullopt;
}

QModelIndex FlatpakPermissionModel::findPermissionIndex(FlatpakPermissionsSectionType::Type section, const QString &name) const
{
    auto row = findPermissionRow(section, name);
    if (row.has_value()) {
        return index(row.value());
    }
    return QModelIndex();
}

bool FlatpakPermissionModel::isFilesystemNameValid(const QString &name)
{
    // Force parsing the name part only, pass dummy access mode.
    return FlatpakFilesystemsEntry::parse(name, FlatpakFilesystemsEntry::AccessMode::ReadWrite).has_value();
}

bool FlatpakPermissionModel::isDBusServiceNameValid(const QString &name)
{
    return FlatpakHelper::verifyDBusName(name);
}

bool FlatpakPermissionModel::isEnvironmentVariableNameValid(const QString &name)
{
    return !name.isEmpty() && !name.contains(QLatin1Char('='));
}

void FlatpakPermissionModel::togglePermissionAtRow(int row)
{
    if (row < 0 || row >= m_permissions.length()) {
        return;
    }

    FlatpakPermission &permission = m_permissions[row];

    const auto wasEnabled = permission.isEffectiveEnabled();
    const auto shouldBecomeEnabled = !wasEnabled;

    if (!shouldBecomeEnabled && !permission.canBeDisabled()) {
        qWarning() << "Illegal operation: Permission provided by upstream can not be toggled:" << permission.category() << permission.name();
        return;
    }

    permission.setEffectiveEnabled(shouldBecomeEnabled);

    Q_EMIT dataChanged(index(row), index(row), {Roles::IsEffectiveEnabled});
}

void FlatpakPermissionModel::setPermissionValueAtRow(int row, const QVariant &value)
{
    if (row < 0 || row >= m_permissions.length()) {
        return;
    }

    FlatpakPermission &permission = m_permissions[row];

    switch (permission.section()) {
    case FlatpakPermissionsSectionType::Filesystems:
        if (!value.canConvert<FlatpakFilesystemsEntry::AccessMode>()) {
            qWarning() << "Wrong data type assigned to Filesystem entry:" << value;
            return;
        }
        permission.setEffectiveValue(value.value<FlatpakFilesystemsEntry::AccessMode>());
        break;
    case FlatpakPermissionsSectionType::SessionBus:
    case FlatpakPermissionsSectionType::SystemBus:
        if (!value.canConvert<FlatpakPolicy>()) {
            qWarning() << "Wrong data type assigned to D-Bus entry:" << value;
            return;
        }
        permission.setEffectiveValue(value.value<FlatpakPolicy>());
        break;
    case FlatpakPermissionsSectionType::Environment:
        if (!value.canConvert<QString>()) {
            qWarning() << "Wrong data type assigned to Environment entry:" << value;
            return;
        }
        permission.setEffectiveValue(value.toString());
        break;
    case FlatpakPermissionsSectionType::Basic:
    case FlatpakPermissionsSectionType::Advanced:
    case FlatpakPermissionsSectionType::SubsystemsShared:
    case FlatpakPermissionsSectionType::Sockets:
    case FlatpakPermissionsSectionType::Devices:
    case FlatpakPermissionsSectionType::Features:
        return;
    }
    permission.setEffectiveEnabled(true);

    Q_EMIT dataChanged(index(row), index(row), {Roles::IsEffectiveEnabled, Roles::EffectiveValue});
}

void FlatpakPermissionModel::addUserEnteredPermission(int /*FlatpakPermissionsSectionType::Type*/ rawSection, const QString &name, const QVariant &value)
{
    if (!QMetaEnum::fromType<FlatpakPermissionsSectionType::Type>().valueToKey(rawSection)) {
        return;
    }
    // SAFETY: QMetaEnum above ensures that coercion is valid.
    const auto section = static_cast<FlatpakPermissionsSectionType::Type>(rawSection);
    QString category;
    FlatpakPermission::Variant variant;

    if (permissionExists(section, name)) {
        qWarning() << "Tried to add duplicate entry" << section << name;
        return;
    }

    switch (section) {
    case FlatpakPermissionsSectionType::Filesystems:
        if (!isFilesystemNameValid(name)) {
            qWarning() << "Tried to add Filesystem entry with invalid name:" << name;
            return;
        }
        if (!value.canConvert<FlatpakFilesystemsEntry::AccessMode>()) {
            qWarning() << "Tried to add Filesystem entry with wrong data type:" << value;
            return;
        }
        category = QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS);
        variant = value.value<FlatpakFilesystemsEntry::AccessMode>();
        break;
    case FlatpakPermissionsSectionType::SessionBus:
    case FlatpakPermissionsSectionType::SystemBus:
        if (!isDBusServiceNameValid(name)) {
            qWarning() << "Tried to add D-Bus entry with invalid service name or prefix:" << name;
            return;
        }
        if (!value.canConvert<FlatpakPolicy>()) {
            qWarning() << "Wrong data type assigned to D-Bus entry:" << value;
            return;
        }
        category = (section == FlatpakPermissionsSectionType::SessionBus) //
            ? QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY)
            : QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY);
        variant = value.value<FlatpakPolicy>();
        break;
    case FlatpakPermissionsSectionType::Environment:
        if (!isEnvironmentVariableNameValid(name)) {
            qWarning() << "Tried to add Environment entry with invalid name:" << name;
            return;
        }
        if (!value.canConvert<QString>()) {
            qWarning() << "Wrong data type assigned to Environment entry:" << value;
            return;
        }
        category = QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT);
        variant = value.toString();
        break;
    case FlatpakPermissionsSectionType::Basic:
    case FlatpakPermissionsSectionType::Advanced:
    case FlatpakPermissionsSectionType::SubsystemsShared:
    case FlatpakPermissionsSectionType::Sockets:
    case FlatpakPermissionsSectionType::Devices:
    case FlatpakPermissionsSectionType::Features:
        return;
    }

    FlatpakPermission permission(section, name, category, name, false, variant);
    permission.setOriginType(FlatpakPermission::OriginType::UserDefined);
    permission.setEffectiveEnabled(true);

    int index = findIndexToInsertRowAndRemoveDummyRowIfNeeded(section, true);
    beginInsertRows(QModelIndex(), index, index);
    {
        m_permissions.insert(index, permission);
    }
    endInsertRows();
}

int FlatpakPermissionModel::findIndexToInsertRowAndRemoveDummyRowIfNeeded(FlatpakPermissionsSectionType::Type section, bool emitModelSignals)
{
    int i = 0;
    while (i < m_permissions.length()) {
        const auto *permission = &m_permissions.at(i);
        if (permission->section() == section) {
            if (permission->originType() == FlatpakPermission::OriginType::Dummy) {
                if (emitModelSignals) {
                    beginRemoveRows(QModelIndex(), i, i);
                }
                permission = nullptr;
                m_permissions.remove(i, 1);
                if (emitModelSignals) {
                    endRemoveRows();
                }
            }
            break;
        }
        i++;
    }
    while (i < m_permissions.length()) {
        const auto &permission = m_permissions.at(i);
        if (permission.section() != section) {
            break;
        }
        i++;
    }
    return i;
}

void FlatpakPermissionModel::writeToFile() const
{
    const auto &userAppOverrides = m_reference->userLevelPerAppOverrideFile();
    if (isDefaults()) {
        QFile::remove(userAppOverrides);
    } else {
        // Ensure directory exists before creating a config in it.
        const auto dir = QFileInfo(userAppOverrides).dir();
        QDir().mkpath(dir.path());

        KConfig config(userAppOverrides, KConfig::SimpleConfig);
        if (!config.isConfigWritable(true)) {
            return;
        }
        // Clear the config first
        if (const auto groups = config.groupList(); !groups.isEmpty()) {
            for (const auto &group : groups) {
                config.deleteGroup(group);
            }
        }
        writeToKConfig(config);
    }
}

void FlatpakPermissionModel::writeToKConfig(KConfig &config) const
{
    QHash<QString, QStringList> entriesByCategory = m_unparsableEntriesByCategory;

    for (const auto &permission : m_permissions) {
        if (permission.isDefaults()) {
            continue;
        }
        switch (permission.valueType()) {
        case FlatpakPermission::ValueType::Simple: {
            auto &entries = entriesByCategory[permission.category()];
            const auto entry = FlatpakSimpleEntry(permission.name(), permission.isEffectiveEnabled());
            entries.append(entry.format());
            break;
        }
        case FlatpakPermission::ValueType::Filesystems: {
            auto &entries = entriesByCategory[permission.category()];
            const auto accessMode = std::get<FlatpakFilesystemsEntry::AccessMode>(permission.effectiveValue());
            FlatpakFilesystemsEntry::parse(permission.name(), accessMode);
            if (const auto entry = FlatpakFilesystemsEntry::parse(permission.name(), accessMode); entry.has_value()) {
                entries.append(entry.value().format());
            }
            break;
        }
        case FlatpakPermission::ValueType::Bus: {
            const auto policyValue = std::get<FlatpakPolicy>(permission.effectiveValue());
            const auto policyString = mapDBusFlatpakPolicyEnumValueToConfigString(policyValue);
            auto group = config.group(permission.category());
            group.writeEntry(permission.name(), policyString);
            break;
        }
        case FlatpakPermission::ValueType::Environment: {
            const auto value = std::get<QString>(permission.effectiveValue());
            auto group = config.group(permission.category());
            group.writeEntry(permission.name(), value);
            break;
        }
        } // switch
    }

    auto contextGroup = config.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
    for (auto it = entriesByCategory.constKeyValueBegin(); it != entriesByCategory.constKeyValueEnd(); it++) {
        const auto [category, entries] = *it;
        contextGroup.writeXdgListEntry(category, entries);
    }
}

#include "moc_flatpakpermission.cpp"
