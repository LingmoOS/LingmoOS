/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "flatpakcommon.h"
#include "flatpakreference.h"

#include <KConfigGroup>
#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QPointer>
#include <QString>

#include <optional>
#include <variant>

class FlatpakReference;

/** For exporting enum to QML */
class FlatpakPermissionsSectionType : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Basic,
        Filesystems,
        Advanced,
        SubsystemsShared,
        Sockets,
        Devices,
        Features,
        SessionBus,
        SystemBus,
        Environment,
    };
    Q_ENUM(Type)
};

/**
 * Boolean entry of one of the `shared=`, `sockets=`, `devices=` or `features=`
 * lists of the `[Context]` group.
 *
 * Such entries are either ON or OFF. They are turned OFF if and only if their
 * string representation is prefixed with a `!` bang character.
 */
class FlatpakSimpleEntry
{
    Q_GADGET
public:
    // Default constructor is required for meta-type registration.
    /** Default constructor. Creates an invalid entry. */
    FlatpakSimpleEntry() = default;

    /**
     * Construct new entry directly from raw unvalidated data.
     */
    explicit FlatpakSimpleEntry(const QString &name, bool enabled = true);

    /**
     * Parse list entry into data structure. Might fail for various reasons,
     * such as ill-formed access mode, so returns an optional value.
     */
    static std::optional<FlatpakSimpleEntry> parse(QStringView entry);

    /**
     * Parse list of entries. Returns list of successfully parsed valid entries,
     * as well as a list of the rest unparsable strings.
     */
    static std::pair<QStringList, QList<FlatpakSimpleEntry>> getCategory(const KConfigGroup &group, const QString &category);

    /**
     * Parse list of entries. Returns only list of successfully parsed valid entries.
     */
    static QList<FlatpakSimpleEntry> getCategorySkippingInvalidEntries(const KConfigGroup &group, const QString &category);

    static std::optional<bool> isEnabled(const QList<FlatpakSimpleEntry> &entries, const QString &name);

    /**
     * Format this entry into string. Opposite of parse().
     */
    QString format() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    const QString &name() const;

    // TODO C++20: use `= default` implementation.
    bool operator==(const FlatpakSimpleEntry &other) const;
    bool operator!=(const FlatpakSimpleEntry &other) const;

private:
    QString m_name;
    bool m_enabled = true;
};

/**
 * Entry of the `filesystems=` list.
 *
 * "Deny" access mode represents an entry prefixes with a `!` bang character.
 * Other modes correspond to `:ro`, `:rw` and `:create` suffixes.
 *
 * Path should be without trailing `/` slash character.
 * See flatpak-metadata(5) for possible values.
 */
class FlatpakFilesystemsEntry
{
    Q_GADGET
public:
    enum class AccessMode {
        /** Make the given directory available read-only. */
        ReadOnly,
        /** Make the given directory available read/write. This is the default. */
        ReadWrite,
        /** Make the given directory available read/write, and create it if it does not already exist. */
        Create,
        /** Don't expose filesystem to app. */
        Deny,
    };
    Q_ENUM(AccessMode)

    enum class PathMode {
        Required,
        Optional,
        NoPath,
    };

    enum class FilesystemPrefix {
        /** Path is required. */
        Absolute,
        /** Path is optional. */
        Home,
        /** Path is invalid. */
        Host,
        HostOs,
        HostEtc,
        /** Path is optional. */
        XdgDesktop,
        XdgDocuments,
        XdgDownload,
        XdgMusic,
        XdgPictures,
        XdgPublicShare,
        XdgVideos,
        XdgTemplates,
        /** Path is optional. */
        XdgCache,
        XdgConfig,
        XdgData,
        /** Path is required. */
        XdgRun,
        // Future-proof fallback variant. Prefix is empty, so path mode is
        // required, which also ensures that is will be non-empty.
        Unknown,
    };

    // For Required path mode, fixed string is empty because it is meaningless.
    // For Invalid path mode, second, prefix string is empty.
    struct TableEntry {
        FilesystemPrefix prefix;
        PathMode mode;
        // Fixed magic filesystem name
        QLatin1String fixedString;
        // Same as fixed, but with '/' appended.
        QLatin1String prefixString;
    };

    // Default constructor is required for meta-type registration.
    /** Default constructor. Creates an invalid entry. */
    FlatpakFilesystemsEntry() = default;

    /**
     * Construct new entry directly from raw unvalidated data.
     */
    explicit FlatpakFilesystemsEntry(FilesystemPrefix prefix, AccessMode mode, const QString &path = QString());

    /**
     * Parse list entry into data structure. Might fail for various reasons,
     * such as ill-formed access mode, so returns an optional value.
     */
    static std::optional<FlatpakFilesystemsEntry> parse(QStringView entry);

    /**
     * Parse name of a list entry into data structure. Access mode is passed
     * separately and not expected to be a part of the name. Might fail for
     * various reasons, so returns an optional value.
     *
     * See also: name()
     */
    static std::optional<FlatpakFilesystemsEntry> parse(QStringView name, AccessMode accessMode);

    /**
     * Formatted prefix and path only, without access mode prefixes or suffixes. Suitable for comparison.
     *
     * See also: parse(QStringView, AccessMode)
     */
    QString name() const;

    /**
     * Format this entry into string. Opposite of parse(). Omits default `:rw` suffix.
     */
    QString format() const;

    FilesystemPrefix prefix() const;
    QString path() const;
    AccessMode mode() const;

    // TODO C++20: use `= default` implementation.
    bool operator==(const FlatpakFilesystemsEntry &other) const;
    bool operator!=(const FlatpakFilesystemsEntry &other) const;

private:
    FilesystemPrefix m_prefix = FilesystemPrefix::Absolute;
    AccessMode m_mode = AccessMode::ReadWrite;
    // Depending on prefix type, path can be optional, required or illegal.
    QString m_path;
};

class PolicyChoicesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        // Uses standard Qt::DisplayRole for text
        ValueRole = Qt::UserRole + 1,
    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

protected:
    struct Entry {
        int value;
        QString display;
    };

    explicit PolicyChoicesModel(QList<Entry> &&policies, QObject *parent = nullptr);

private:
    const QList<Entry> m_policies;
};

class FilesystemChoicesModel : public PolicyChoicesModel
{
    Q_OBJECT
public:
    explicit FilesystemChoicesModel(QObject *parent = nullptr);
};

class DBusPolicyChoicesModel : public PolicyChoicesModel
{
    Q_OBJECT
public:
    explicit DBusPolicyChoicesModel(QObject *parent = nullptr);
};

/**
 * @class FlatpakPermission describes a single configurable entry in the list model of permissions.
 *
 * The content of instances of this class can be interpretted in different ways depending on their
 * value type(), permission pType() and section sType().
 *
 * See flatpak-metadata(5) for more.
 */
class FlatpakPermission
{
public:
    enum class ValueType {
        /**
         * This type is for permission entries representing simple boolean
         * toggles.
         *
         * Name of such entry is one of the predefined resource names, e.g.:
         * "bluetooth" from "features" category, "kvm" from "devices" category
         * etc, "pulseaudio" from "sockets" category etc.
         */
        Simple,
        /**
         * Filesystem permissions fall into the "Basic" section type, i.e.
         * always shown.
         *
         * Name of such entry is an actual filesystem path, and the value is
         * one of the suffixes: ":ro", ":rw" (default), ":create".
         */
        Filesystems,
        /**
         * Name of such permission entry is a D-Bus bus name or prefix thereof,
         * for example org.gnome.SessionManager or org.freedesktop.portal.*
         *
         * The possible values for entry are: "none", "see", "talk" or "own".
         */
        Bus,
        /**
         * Name and value of such permission entry are name and value of an
         * environment variable.
         */
        Environment
    };

    static ValueType valueTypeFromSectionType(FlatpakPermissionsSectionType::Type section);

    enum class OriginType {
        /**
         * Built-in type is for all pre-defined system resources (permissions)
         * as found in flatpak-metadata(5) man page, and any other additional
         * resources declared in app metadata.
         *
         * They shall not be removed from the list of permissions when
         * unchecked.
         *
         * Predefined resources come with translated description.
         */
        // TODO: Instead of unchecking there should be more obvious UI. For Bus
        // type, there's a "none" policy. For environment we should implement
        // "unset-environment" category.
        BuiltIn,
        /**
         * User-defined permissions are resources that user has manually added
         * in their overrides. In other words, they are not present in app
         * metadata manifest, and can be removed completely when unchecked.
         */
        // TODO: Same as in BuiltIn, consider "Remove" button instead of unchecking.
        UserDefined,
        /**
         * Empty permissions, just for showing section headers for categories
         * that don't have permissions.
         */
        Dummy
    };

    using Variant = std::variant<QString, FlatpakPolicy, FlatpakFilesystemsEntry::AccessMode>;

    // Default constructor is required for meta-type registration.
    /** Default constructor. Creates an invalid entry. */
    FlatpakPermission() = default;

    /**
     * Create a Dummy entry for the Advanced and user-editable sections, just so
     * that ListView shows a section header even if there are no permission row
     * entries in it.
     */
    explicit FlatpakPermission(FlatpakPermissionsSectionType::Type section);

    explicit FlatpakPermission(FlatpakPermissionsSectionType::Type section,
                               const QString &name,
                               const QString &category,
                               const QString &description,
                               bool isDefaultEnabled,
                               const Variant &defaultValue = QString());

    /** Section type for QtQuick/ListView. */
    FlatpakPermissionsSectionType::Type section() const;

    /**
     * Technical untranslated name of the resource managed by this permission entry.
     *
     * See ValueType enum for more.
     */
    const QString &name() const;

    /**
     * Technical untranslated category name of the resource managed by this permission entry.
     *
     * See ValueType enum for more.
     */
    const QString &category() const;

    /**
     * Untranslate section heading back into category identifier. It's a hack
     * until the model is refactored to only operate on identifiers, and all
     * i18n stuff is moved elsewhere.
     */
    static QString categoryHeadingToRawCategory(const QString &section);

    /**
     * User-facing translated description of the resource managed by this permission entry.
     *
     * See ValueType enum for more.
     */
    const QString &description() const;

    /**
     * Return type of value of this entry, inferred from its SectionType.
     */
    ValueType valueType() const;

    /**
     * Type of permission this entry represents.
     */
    OriginType originType() const;

    /**
     * Set which type of permissions this entry represents.
     */
    // TODO: This method should be replaced with constructor argument.
    void setOriginType(OriginType type);

    /**
     * System default "enabled" status of this permission. It can not be modified.
     */
    bool isDefaultEnabled() const;

    /** Set user override */
    void setOverrideEnabled(bool enabled);

    /**
     * This property reports the current effective "enabled" status of this
     * permission in KCM.
     *
     * For ValueType::Simple permissions, if current enabled status matches
     * system defaults, it will be removed from user overrides.
     *
     * For user-defined permissions of other ValueType types, disabling them
     * would mark them for removal but only if they are disabled (not present)
     * in defaults.
     */
    bool canBeDisabled() const;
    bool isEffectiveEnabled() const;
    void setEffectiveEnabled(bool enabled);

    /**
     * System default value for this permission. It can only be modified for
     * entries which are disabled (not present) by default.
     *
     * Applicable for any permissions other than ValueType::Simple.
     */
    const Variant defaultValue() const;

    void setDefaultValue(const Variant &value);

    /** Set user override. Affects isSaveNeeded() state. */
    void setOverrideValue(const Variant &value);

    /**
     * This property holds the current effective value of this permission in
     * KCM.
     *
     * See ValueType enum for more.
     */
    const Variant effectiveValue() const;
    void setEffectiveValue(const Variant &value);

    /** Integration with KCM. */
    bool isSaveNeeded() const;
    bool isDefaults() const;

private:
    /** Section type for QtQuick/ListView. */
    FlatpakPermissionsSectionType::Type m_section;

    /**
     * Untranslatable identifier of permission.
     *
     * For ValueType::Simple permissions, it's the name of the entry in the list of togglable options in that category.
     * For ValueType::Filesystems it's either one of the pre-defined symbolic names or the absolute filepath.
     * For ValueType::Bus permissions it's the the name or glob pattern of D-Bus service(s).
     * For ValueType::Environment permissions it's the name of environment variable.
     */
    QString m_name;
    /** Untranslatable name of [Category] as seen in metadata and override ini-style files. */
    QString m_category;
    /** Human-readable description of the permission, or whatever to be displayed in UI. */
    QString m_description;

    /* Attempts to classify permissions into various types and groups. */

    OriginType m_originType;

    /* Applicable for all ValueType permissions. */

    /** System defaults */
    bool m_defaultEnable;
    /** User overrides */
    bool m_overrideEnable;
    /** Current value in KCM */
    bool m_effectiveEnable;

    /* Applicable for any permissions other than ValueType::Simple. */

    /** System defaults */
    Variant m_defaultValue;
    /** User overrides */
    Variant m_overrideValue;
    /** Current value in KCM */
    Variant m_effectiveValue;
};

namespace FlatpakOverrides
{

using KConfigPtr = std::unique_ptr<KConfig>;

KConfigPtr loadAndMerge(const QStringList &filenames);
void merge(KConfig &target, const QString &filename);
void merge(KConfig &target, const KConfig &source);
};

class FlatpakPermissionModel : public QAbstractListModel
{
    friend class FlatpakPermissionModelTest;
    Q_OBJECT
    Q_PROPERTY(FlatpakReference *reference READ reference WRITE setReference NOTIFY referenceChanged)
    Q_PROPERTY(bool showAdvanced READ showAdvanced WRITE setShowAdvanced NOTIFY showAdvancedChanged)
public:
    FlatpakPermissionModel(QObject *parent = nullptr);

    enum Roles {
        Section = Qt::UserRole + 1,
        Name,
        Description,
        //
        IsNotDummy,
        //
        CanBeDisabled,
        IsDefaultEnabled,
        IsEffectiveEnabled,
        DefaultValue,
        EffectiveValue,
        //
        ValuesModel,
    };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadDefaultValues();
    void loadCurrentValues();

    FlatpakReference *reference() const;
    void setReference(FlatpakReference *ref);

    bool showAdvanced() const;
    void setShowAdvanced(bool);
    /** Helper function to count actual rows regardless of current "showAdvanced" setting. */
    int rowCount(bool showAdvanced) const;

    void load();
    void save();
    void defaults();
    bool isDefaults() const;
    bool isSaveNeeded() const;

    /**
     * Static list of user-facing translated policy names for this kind of
     * permission.
     *
     * Applicable for ValueType::Filesystems and ValueType::Bus only.
     */
    Q_INVOKABLE static PolicyChoicesModel *valuesModelForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection);
    Q_INVOKABLE static PolicyChoicesModel *valuesModelForFilesystemsSection();
    Q_INVOKABLE static PolicyChoicesModel *valuesModelForBusSections();

    Q_INVOKABLE static QString sectionHeaderForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection);
    Q_INVOKABLE static QString sectionAddButtonToolTipTextForSectionType(int /*FlatpakPermissionsSectionType::Type*/ rawSection);

    Q_INVOKABLE bool permissionExists(int /*FlatpakPermissionsSectionType::Type*/ rawSection, const QString &name) const;
    bool permissionExists(FlatpakPermissionsSectionType::Type section, const QString &name) const;

    std::optional<int> findPermissionRow(FlatpakPermissionsSectionType::Type section, const QString &name) const;
    QModelIndex findPermissionIndex(FlatpakPermissionsSectionType::Type section, const QString &name) const;

    /**
     * Validators to check that names comply with section-specific rules.
     */
    Q_INVOKABLE static bool isFilesystemNameValid(const QString &name);
    Q_INVOKABLE static bool isDBusServiceNameValid(const QString &name);
    Q_INVOKABLE static bool isEnvironmentVariableNameValid(const QString &name);

public Q_SLOTS:
    void togglePermissionAtRow(int row);
    void setPermissionValueAtRow(int row, const QVariant &newValue);
    void addUserEnteredPermission(int /*FlatpakPermissionsSectionType::Type*/ rawSection, const QString &name, const QVariant &value);

Q_SIGNALS:
    void referenceChanged();
    void showAdvancedChanged();

private:
    int findIndexToInsertRowAndRemoveDummyRowIfNeeded(FlatpakPermissionsSectionType::Type section, bool emitModelSignals);

    void writeToFile() const;
    void writeToKConfig(KConfig &config) const;

    QList<FlatpakPermission> m_permissions;
    QHash<QString, QStringList> m_unparsableEntriesByCategory;
    QPointer<FlatpakReference> m_reference;
    bool m_showAdvanced;

    friend class FlatpakPermissionModelTest;
};
