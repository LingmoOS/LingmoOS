// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QAbstractItemModel>
#include <QDebug>
#include <QMetaEnum>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <array>

#include "flatpakcommon.h"
#include "flatpakpermission.h"

class FlatpakPermissionModelTest : public QObject
{
    Q_OBJECT
private:
    /** KConfig comparison with a grain of Flatpak specifics. */
    bool operatorFlatpakConfigEquals(const KConfig &actual, const KConfig &expected)
    {
        auto actualGroups = actual.groupList();
        actualGroups.sort();
        auto expectedGroups = expected.groupList();
        expectedGroups.sort();

        if (actualGroups != expectedGroups) {
            return false;
        }

        for (const auto &group : std::as_const(actualGroups)) {
            const auto actualGroup = actual.group(group);
            const auto expectedGroup = expected.group(group);

            auto actualKeys = actualGroup.keyList();
            actualKeys.sort();
            auto expectedKeys = expectedGroup.keyList();
            expectedKeys.sort();

            if (actualKeys != expectedKeys) {
                return false;
            }

            for (const auto &key : std::as_const(actualKeys)) {
                if (group == QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT)) {
                    // Treat everything in [Context] as XDG lists
                    auto actualValues = actualGroup.readXdgListEntry(key);
                    actualValues.sort();
                    auto expectedValues = expectedGroup.readXdgListEntry(key);
                    expectedValues.sort();

                    if (actualValues != expectedValues) {
                        return false;
                    }
                } else {
                    auto actualValue = actualGroup.readEntry(key);
                    auto expectedValue = expectedGroup.readEntry(key);

                    if (actualValue != expectedValue) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool modelContains(const QAbstractItemModel *model, const QVariant &value, int role = Qt::DisplayRole)
    {
        for (int i = 0; i < model->rowCount(); i++) {
            const auto &data = model->data(model->index(i, 0), role);
            if (data == value) {
                return true;
            }
        }
        return false;
    }

    QStringList mockMetadataAndOverridesFiles(const QString &flatpakName) const
    {
        const auto metadata = QFINDTESTDATA(QStringLiteral("fixtures/metadata/%1").arg(flatpakName));
        // Metadata should exist, but overrides file might be created on the fly.
        static const auto overridesDirectory = QFINDTESTDATA(QStringLiteral("fixtures/overrides"));
        const auto override = QStringLiteral("%1/%2").arg(overridesDirectory, flatpakName);
        return {metadata, override};
    }

    // These are little helpers to avoid writing out default-constructed flags
    // every time. These flags are for pure in-memory configs to avoid any
    // possible surprizes from KConfig infrastructure.

    static void write(KConfigGroup &group, const QString &key, const QString &value)
    {
        group.writeEntry(key, value, KConfig::WriteConfigFlags());
    };

    static void write(KConfigGroup &group, const QString &key, const QStringList &values)
    {
        group.writeXdgListEntry(key, values, KConfig::WriteConfigFlags());
    };

private Q_SLOTS:
    void init()
    {
        QDir().rmdir(QFINDTESTDATA("fixtures/overrides/"));
    }

    void testCompareFlatpakConfigsEqual()
    {
        KConfig configA(QString(), KConfig::SimpleConfig);
        KConfig configB(QString(), KConfig::SimpleConfig);

        const std::array configs = {&configA, &configB};
        for (const auto &config : configs) {
            auto group1 = config->group(QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT));
            auto group2 = config->group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));

            write(group1, QStringLiteral("key1"), QStringLiteral("value1"));
            write(group1, QStringLiteral("key2"), QStringLiteral("value2"));

            auto values = QStringList{QStringLiteral("itemA"), QStringLiteral("itemB")};
            write(group2, QStringLiteral("key3"), values);
            // Reorder list entries
            if (config == &configB) {
                values = QStringList{QStringLiteral("itemC"), QStringLiteral("itemD")};
            } else {
                values = QStringList{QStringLiteral("itemD"), QStringLiteral("itemC")};
            }
            write(group2, QStringLiteral("key4"), values);
        }

        QVERIFY(operatorFlatpakConfigEquals(configA, configB));
    }

    void testCompareFlatpakConfigsWhereXDGListsDiffer()
    {
        KConfig configA(QString(), KConfig::SimpleConfig);
        KConfig configB(QString(), KConfig::SimpleConfig);

        auto groupA = configA.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
        auto groupB = configB.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));

        QStringList values = QStringList{QStringLiteral("itemA"), QStringLiteral("itemB")};
        write(groupA, QStringLiteral("key4"), values);
        values.append(QStringLiteral("itemC"));
        write(groupB, QStringLiteral("key4"), values);

        QVERIFY(!operatorFlatpakConfigEquals(configA, configB));
    }

    void testCompareFlatpakConfigsWhereNumberOfGroupsDiffers()
    {
        KConfig configA(QString(), KConfig::SimpleConfig);
        KConfig configB(QString(), KConfig::SimpleConfig);

        const std::array configs = {&configA, &configB};
        for (const auto &config : configs) {
            const std::array groupNames = {QLatin1String("G1"), QLatin1String("G2")};
            for (const auto &groupName : groupNames) {
                auto group = config->group(groupName);
                write(group, QStringLiteral("key1"), QStringLiteral("value1"));
            }
        }
        QVERIFY(operatorFlatpakConfigEquals(configA, configB));
        const auto extraGroupName = QLatin1String("GExtra");
        {
            auto groupA3 = configA.group(extraGroupName);
            write(groupA3, QStringLiteral("key1"), QStringLiteral("value1"));
        }
        QVERIFY(!operatorFlatpakConfigEquals(configA, configB));
        configA.deleteGroup(extraGroupName);
        QVERIFY(operatorFlatpakConfigEquals(configA, configB));
        {
            // Test the other way around too
            auto groupB3 = configB.group(extraGroupName);
            write(groupB3, QStringLiteral("key1"), QStringLiteral("value1"));
        }
        QVERIFY(!operatorFlatpakConfigEquals(configA, configB));
        configB.deleteGroup(extraGroupName);
        QVERIFY(operatorFlatpakConfigEquals(configA, configB));
    }

    void testMergeConfigObjectsWithBareTarget()
    {
        KConfig target(QString(), KConfig::SimpleConfig);
        KConfig source(QString(), KConfig::SimpleConfig);
        KConfig expected(QString(), KConfig::SimpleConfig);

        for (const auto config : {&source, &expected}) {
            auto group = config->group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("network")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("x11"), QStringLiteral("wayland"), QStringLiteral("fallback-x11")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("dri"), QStringLiteral("kvm")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("bluetooth"), QStringLiteral("devel")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("xdg-download"), QStringLiteral("~/path")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = config->group(groupName);
                write(group, QStringLiteral("abc.def"), QStringLiteral("talk"));
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("see"));
            }
        }

        FlatpakOverrides::merge(target, source);
        QVERIFY(operatorFlatpakConfigEquals(target, expected));
    }

    void testMergeConfigObjectsNonOverlapping()
    {
        // In this test case entries are not overlapping, i.e. they are unique to source and target.

        KConfig target(QString(), KConfig::SimpleConfig);
        KConfig source(QString(), KConfig::SimpleConfig);
        KConfig expected(QString(), KConfig::SimpleConfig);

        // put some stuff in target, some in source, and both in the expected config.
        {
            auto group = target.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("network")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("fallback-x11")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("dri")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("devel")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("~/path")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = target.group(groupName);
                write(group, QStringLiteral("abc.def"), QStringLiteral("talk"));
            }
        }
        {
            auto group = source.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("ipc")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("x11"), QStringLiteral("wayland")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("kvm")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("bluetooth")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("xdg-download")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = source.group(groupName);
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("see"));
            }
        }
        {
            auto group = expected.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("network"), QStringLiteral("ipc")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("x11"), QStringLiteral("wayland"), QStringLiteral("fallback-x11")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("dri"), QStringLiteral("kvm")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("bluetooth"), QStringLiteral("devel")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("xdg-download"), QStringLiteral("~/path")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = expected.group(groupName);
                write(group, QStringLiteral("abc.def"), QStringLiteral("talk"));
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("see"));
            }
        }

        FlatpakOverrides::merge(target, source);
        QVERIFY(operatorFlatpakConfigEquals(target, expected));
    }

    void testMergeConfigObjectsOverlapping()
    {
        // In this test some entries in target are overridden by source.

        KConfig target(QString(), KConfig::SimpleConfig);
        KConfig source(QString(), KConfig::SimpleConfig);
        KConfig expected(QString(), KConfig::SimpleConfig);

        {
            auto group = target.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("!network"), QStringLiteral("ipc")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("x11"), QStringLiteral("wayland"), QStringLiteral("fallback-x11")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("dri"), QStringLiteral("kvm")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("bluetooth"), QStringLiteral("!devel")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("xdg-download"), QStringLiteral("~/path")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = target.group(groupName);
                write(group, QStringLiteral("abc.def"), QStringLiteral("talk"));
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("own"));
            }
        }
        {
            auto group = source.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("network"), QStringLiteral("!ipc")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), {QStringLiteral("!fallback-x11"), QStringLiteral("pulseaudio")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("!dri")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("devel")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS), {QStringLiteral("!xdg-download"), QStringLiteral("host-etc")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = source.group(groupName);
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("none"));
                write(group, QStringLiteral("org.kde.*"), QStringLiteral("see"));
            }
        }
        {
            auto group = expected.group(QLatin1String(FLATPAK_METADATA_GROUP_CONTEXT));
            write(group, QLatin1String(FLATPAK_METADATA_KEY_SHARED), {QStringLiteral("network"), QStringLiteral("!ipc")});
            write(group,
                  QLatin1String(FLATPAK_METADATA_KEY_SOCKETS),
                  {QStringLiteral("x11"), QStringLiteral("wayland"), QStringLiteral("!fallback-x11"), QStringLiteral("pulseaudio")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_DEVICES), {QStringLiteral("!dri"), QStringLiteral("kvm")});
            write(group, QLatin1String(FLATPAK_METADATA_KEY_FEATURES), {QStringLiteral("bluetooth"), QStringLiteral("devel")});
            write(group,
                  QLatin1String(FLATPAK_METADATA_KEY_FILESYSTEMS),
                  {QStringLiteral("!xdg-download"), QStringLiteral("~/path"), QStringLiteral("host-etc")});

            for (const auto &groupName : {
                     QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY),
                     QLatin1String(FLATPAK_METADATA_GROUP_ENVIRONMENT),
                 }) {
                auto group = expected.group(groupName);
                write(group, QStringLiteral("abc.def"), QStringLiteral("talk"));
                write(group, QStringLiteral("xyz.xyz"), QStringLiteral("none"));
                write(group, QStringLiteral("org.kde.*"), QStringLiteral("see"));
            }
        }

        FlatpakOverrides::merge(target, source);
        QVERIFY(operatorFlatpakConfigEquals(target, expected));
    }

    void testMergeConfigFiles()
    {
        const QStringList overridesFiles = {
            QFINDTESTDATA(QStringLiteral("fixtures/metadata/com.example.cascade.metadata")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.system-global")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.user-global")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.user-app")),
        };
        const auto expectedFilepath = QFINDTESTDATA(QStringLiteral("fixtures/overrides.out/com.example.cascade.final"));

        const auto actual = FlatpakOverrides::loadAndMerge(overridesFiles);
        const KConfig expected(expectedFilepath, KConfig::SimpleConfig);

        QVERIFY(operatorFlatpakConfigEquals(*actual, expected));
    }

    void testReadMergedMultiLevelOverrides()
    {
        const QStringList overrideFiles = {
            QFINDTESTDATA(QStringLiteral("fixtures/metadata/com.example.cascade.metadata")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.system-global")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.user-global")),
            QFINDTESTDATA(QStringLiteral("fixtures/overrides.in/com.example.cascade.user-app")),
        };
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference
            reference(&referencesModel, "com.example.cascade.metadata", "x86_64", "stable", "1.0.0", "com.example.cascade.metadata", QUrl(), overrideFiles);
        model.setReference(&reference);
        model.setShowAdvanced(true);
        model.load();

        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::Sockets, QLatin1String("x11"));
            QVERIFY(index.isValid());
            QVERIFY(!model.data(index, FlatpakPermissionModel::IsDefaultEnabled).toBool());
            QVERIFY(model.data(index, FlatpakPermissionModel::IsEffectiveEnabled).toBool());
        }
        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::Filesystems, QLatin1String("xdg-pictures"));
            QVERIFY(index.isValid());
            QVERIFY(model.data(index, FlatpakPermissionModel::IsDefaultEnabled).toBool());
            QVERIFY(model.data(index, FlatpakPermissionModel::IsEffectiveEnabled).toBool());
            QCOMPARE(model.data(index, FlatpakPermissionModel::DefaultValue).value<FlatpakFilesystemsEntry::AccessMode>(),
                     FlatpakFilesystemsEntry::AccessMode::ReadOnly);
            QCOMPARE(model.data(index, FlatpakPermissionModel::EffectiveValue).value<FlatpakFilesystemsEntry::AccessMode>(),
                     FlatpakFilesystemsEntry::AccessMode::Create);
        }
        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::SystemBus, QLatin1String("com.example.system1"));
            QVERIFY(index.isValid());
            QCOMPARE(model.data(index, FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_TALK);
            QCOMPARE(model.data(index, FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_SEE);
        }
        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::SystemBus, QLatin1String("com.example.system2"));
            QVERIFY(index.isValid());
            QVERIFY(!model.data(index, FlatpakPermissionModel::IsDefaultEnabled).toBool());
            QVERIFY(model.data(index, FlatpakPermissionModel::IsEffectiveEnabled).toBool());
            QCOMPARE(model.data(index, FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_OWN);
            QCOMPARE(model.data(index, FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_OWN);
        }
        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::SessionBus, QLatin1String("com.example.session1"));
            QVERIFY(index.isValid());
            QCOMPARE(model.data(index, FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_TALK);
            QCOMPARE(model.data(index, FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_NONE);
        }
        {
            const auto index = model.findPermissionIndex(FlatpakPermissionsSectionType::Environment, QLatin1String("EXAMPLE_NAME"));
            QVERIFY(index.isValid());
            QCOMPARE(model.data(index, FlatpakPermissionModel::DefaultValue).toString(), QLatin1String("user"));
            QCOMPARE(model.data(index, FlatpakPermissionModel::EffectiveValue).toString(), QLatin1String("user-app"));
        }
    }

    void testRead()
    {
        // The primary motivation behind this test is to make sure that translations aren't being pulled in for the raw names.
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel,
                                   "com.discordapp.Discord",
                                   "x86_64",
                                   "stable",
                                   "0.0.24",
                                   "Discord",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("com.discordapp.Discord"));
        model.setReference(&reference);
        model.load();
        bool containsNetwork = false;
        bool containsXdgDownload = false;
        for (auto i = 0; i < model.rowCount(); ++i) {
            const QString name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();
            if (name == "network") {
                containsNetwork = true;
            }

            if (name == "xdg-download") {
                containsXdgDownload = true;
                QVERIFY(!model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled), true);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakFilesystemsEntry::AccessMode>(),
                         FlatpakFilesystemsEntry::AccessMode::ReadWrite);
            }
        }

        QVERIFY(containsNetwork);
        QVERIFY(containsXdgDownload);
        QVERIFY(model.permissionExists(FlatpakPermissionsSectionType::Basic, QLatin1String("network")));
        QVERIFY(!model.permissionExists(FlatpakPermissionsSectionType::Basic, QLatin1String("yolo-foobar")));
    }

    void testLoadSameSimpleOverride()
    {
        // Test that values that are for some reason enabled and present in
        // both defaults and overrides are parsed correctly.
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel,
                                   "com.example.same.override",
                                   "x86_64",
                                   "stable",
                                   "0.0.24",
                                   "Test Same Simple Override",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("com.example.same.override"));
        model.setReference(&reference);
        model.setShowAdvanced(true);
        model.load();

        QStringList found;

        for (auto i = 0; i < model.rowCount(); ++i) {
            const auto name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();

            const auto isDefaultEnabled = model.data(model.index(i, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool();
            const auto isEffectiveEnabled = model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool();

            if (name == QLatin1String("ipc")) {
                QVERIFY(isDefaultEnabled);
                QVERIFY(isEffectiveEnabled);
            } else if (name == QLatin1String("pulseaudio")) {
                QVERIFY(isDefaultEnabled);
                QVERIFY(isEffectiveEnabled);
            } else if (name == QLatin1String("dri")) {
                QVERIFY(!isDefaultEnabled);
                QVERIFY(!isEffectiveEnabled);
            } else if (name == QLatin1String("bluetooth")) {
                QVERIFY(!isDefaultEnabled);
                QVERIFY(isEffectiveEnabled);
            } else if (name == QLatin1String("xdg-download")) {
                QVERIFY(isDefaultEnabled);
                QVERIFY(isEffectiveEnabled);
            } else {
                continue;
            }

            found.append(name);
        }

        QVERIFY(found.contains(QLatin1String("ipc")));
        QVERIFY(found.contains(QLatin1String("pulseaudio")));
        QVERIFY(found.contains(QLatin1String("dri")));
        QVERIFY(found.contains(QLatin1String("bluetooth")));
        QVERIFY(found.contains(QLatin1String("xdg-download")));
    }

    void testDefaultFilesystemsGoFirst()
    {
        // If there are no custom filesystems specified in defaults, then all custom ones should go below it.
        // The Discord test above can't be reused, because it has custom filesystems in base metadata, so
        // the well-known "host-etc" filesystem would not be the last one. But we want to test for the last
        // default index too.
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel, //
                                   "org.gnome.dfeet",
                                   "x86_64",
                                   "stable",
                                   "0.3.16",
                                   "D-Feet",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("org.gnome.dfeet"));
        model.setReference(&reference);
        model.load();
        QStringList filesystems;
        for (auto i = 0; i < model.rowCount(); ++i) {
            const QString name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();
            // collect all filesystems
            const auto section = model.data(model.index(i, 0), FlatpakPermissionModel::Section).value<FlatpakPermissionsSectionType::Type>();
            if (section == FlatpakPermissionsSectionType::Filesystems) {
                filesystems.append(name);
            }
        }

        // Note: update this name when more standard filesystems are added.
        const QString hostEtc = QLatin1String("host-etc");
        const auto indexOfHostEtc = filesystems.indexOf(hostEtc);
        QCOMPARE(indexOfHostEtc, 3);
        // But custom overrides should come after standard ones anyway.
        const auto custom = QLatin1String("/custom/path");
        const auto indexOfCustom = filesystems.indexOf(custom);
        QVERIFY(indexOfCustom != -1);
        QVERIFY(indexOfHostEtc < indexOfCustom);
    }

    void testDBusNonePolicy()
    {
        // This test verifies that "none" policy for D-Bus services works just
        // like other regular policies. It can be loaded, changed to, changed
        // from, and saved.
        const auto metadataAndOverridesFiles = mockMetadataAndOverridesFiles("org.gnome.dfeet");
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel, //
                                   "org.gnome.dfeet",
                                   "x86_64",
                                   "stable",
                                   "0.3.16",
                                   "D-Feet",
                                   QUrl(),
                                   metadataAndOverridesFiles);
        model.setReference(&reference);
        model.load();
        model.setShowAdvanced(true);

        // This service is set to "none" by default (in metadata).
        const auto service0 = QLatin1String("com.example.service0");
        int indexOfService0 = -1;
        // This service is set to "none" in override file.
        const auto service1 = QLatin1String("com.example.service1");
        int indexOfService1 = -1;
        // This service is set to "see" in override, but will be changed to "none".
        const auto service2 = QLatin1String("com.example.service2");
        int indexOfService2 = -1;

        for (auto i = 0; i < model.rowCount(); ++i) {
            const QString name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();
            const auto section = model.data(model.index(i, 0), FlatpakPermissionModel::Section).value<FlatpakPermissionsSectionType::Type>();
            if (section == FlatpakPermissionsSectionType::SessionBus) {
                if (name == service0) {
                    indexOfService0 = i;
                } else if (name == service1) {
                    indexOfService1 = i;
                } else if (name == service2) {
                    indexOfService2 = i;
                }
            }
        }
        QVERIFY(indexOfService0 != -1);
        QVERIFY(indexOfService1 != -1);
        QVERIFY(indexOfService2 != -1);
        const auto name0 = model.data(model.index(indexOfService0, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>();
        const auto name1 = model.data(model.index(indexOfService1, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>();
        const auto name2 = model.data(model.index(indexOfService2, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>();
        QCOMPARE(name0, FlatpakPolicy::FLATPAK_POLICY_NONE);
        QCOMPARE(name1, FlatpakPolicy::FLATPAK_POLICY_NONE);
        QCOMPARE(name2, FlatpakPolicy::FLATPAK_POLICY_SEE);
        QVERIFY(model.data(model.index(indexOfService0, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QVERIFY(!model.data(model.index(indexOfService1, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QVERIFY(!model.data(model.index(indexOfService2, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QVERIFY(!model.data(model.index(indexOfService0, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
        QVERIFY(model.data(model.index(indexOfService1, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
        QVERIFY(model.data(model.index(indexOfService2, 0), FlatpakPermissionModel::CanBeDisabled).toBool());

        const auto setAndCheckBus = [&](const QString &value, FlatpakPolicy newPolicy) {
            model.setPermissionValueAtRow(indexOfService2, newPolicy);
            const auto name2i18nValue = model.data(model.index(indexOfService2, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>();
            QCOMPARE(name2i18nValue, newPolicy);

            QVERIFY(model.isSaveNeeded());
            model.save();

            const KConfig expectedDesktopFile(metadataAndOverridesFiles.last());
            const auto group = expectedDesktopFile.group(QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY));
            const auto name2value = group.readEntry(service2);
            QCOMPARE(name2value, value);
        };

        setAndCheckBus(QLatin1String("none"), FlatpakPolicy::FLATPAK_POLICY_NONE);
        setAndCheckBus(QLatin1String("see"), FlatpakPolicy::FLATPAK_POLICY_SEE);
        setAndCheckBus(QLatin1String("talk"), FlatpakPolicy::FLATPAK_POLICY_TALK);
        setAndCheckBus(QLatin1String("own"), FlatpakPolicy::FLATPAK_POLICY_OWN);

        const auto checkPossibleValues = [&](int row) {
            const auto values = model.data(model.index(row, 0), FlatpakPermissionModel::ValuesModel).value<QAbstractListModel *>();
            QVERIFY(modelContains(values, i18n("None")));
            QVERIFY(modelContains(values, i18n("see")));
            QVERIFY(modelContains(values, i18n("talk")));
            QVERIFY(modelContains(values, i18n("own")));
        };
        checkPossibleValues(indexOfService0);
        checkPossibleValues(indexOfService1);
        checkPossibleValues(indexOfService2);

        const auto isEffectiveEnabled = [&](int row) -> bool {
            return model.data(model.index(row, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool();
        };
        QVERIFY(isEffectiveEnabled(indexOfService0));
        QVERIFY(isEffectiveEnabled(indexOfService1));
        QVERIFY(isEffectiveEnabled(indexOfService2));

        // Toggling non-default bus entry should disable it (i.e. mark for deletion)
        model.togglePermissionAtRow(indexOfService2);
        QVERIFY(!isEffectiveEnabled(indexOfService2));
        // Reloading data should re-enable it again.
        model.load();
        QVERIFY(isEffectiveEnabled(indexOfService2));
        // Disabling non-default bus entry and saving it should remove it from override file.
        model.togglePermissionAtRow(indexOfService2);
        QVERIFY(!isEffectiveEnabled(indexOfService2));
        model.save();
        {
            const KConfig expectedDesktopFile(metadataAndOverridesFiles.last());
            const auto group = expectedDesktopFile.group(QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY));
            QVERIFY(!group.hasKey(service2));
        }
        // Re-enabling non-default bus entry and saving it should add it back to override file.
        model.togglePermissionAtRow(indexOfService2);
        QVERIFY(isEffectiveEnabled(indexOfService2));
        model.save();
        {
            const KConfig expectedDesktopFile(metadataAndOverridesFiles.last());
            const auto group = expectedDesktopFile.group(QLatin1String(FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY));
            QVERIFY(group.hasKey(service2));
        }
    }

    void testDBusBrokenPolicies()
    {
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel, //
                                   "org.gnome.Boxes",
                                   "x86_64",
                                   "stable",
                                   "43.1",
                                   "Boxes",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("org.gnome.Boxes"));
        model.setReference(&reference);
        model.load();
        model.setShowAdvanced(true);

        // This service is set to "hello" by default (in metadata).
        const auto session0 = QLatin1String("org.freedesktop.secrets");
        int indexOfSession0 = -1;
        // This service is set to "talk" by default (in metadata) but overriden with an invalid policy string.
        const auto session1 = QLatin1String("ca.desrt.dconf");
        int indexOfSession1 = -1;
        // This service is set to an invalid policy string by default (in metadata) but overridden with "see".
        const auto system = QLatin1String("org.freedesktop.timedate1");
        int indexOfSystem = -1;

        for (auto i = 0; i < model.rowCount(); ++i) {
            const auto name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();
            const auto section = model.data(model.index(i, 0), FlatpakPermissionModel::Section).value<FlatpakPermissionsSectionType::Type>();
            if (section == FlatpakPermissionsSectionType::SessionBus) {
                if (name == session0) {
                    indexOfSession0 = i;
                } else if (name == session1) {
                    indexOfSession1 = i;
                }
            } else if (section == FlatpakPermissionsSectionType::SystemBus) {
                if (name == system) {
                    indexOfSystem = i;
                }
            }
        }
        QVERIFY(indexOfSession0 != -1);
        QVERIFY(indexOfSession1 != -1);
        QVERIFY(indexOfSystem != -1);

        QVERIFY(model.data(model.index(indexOfSession0, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QCOMPARE(model.data(model.index(indexOfSession0, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_NONE);
        QCOMPARE(model.data(model.index(indexOfSession0, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(),
                 FlatpakPolicy::FLATPAK_POLICY_NONE);

        QVERIFY(model.data(model.index(indexOfSession1, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QVERIFY(model.data(model.index(indexOfSession1, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
        QCOMPARE(model.data(model.index(indexOfSession1, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_TALK);
        QCOMPARE(model.data(model.index(indexOfSession1, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(),
                 FlatpakPolicy::FLATPAK_POLICY_NONE);

        QVERIFY(model.data(model.index(indexOfSystem, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
        QVERIFY(model.data(model.index(indexOfSystem, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
        QCOMPARE(model.data(model.index(indexOfSystem, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_NONE);
        QCOMPARE(model.data(model.index(indexOfSystem, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_SEE);
    }

    void testMutable()
    {
        // Ensure override files mutate properly
        const auto metadataAndOverridesFiles = mockMetadataAndOverridesFiles("com.discordapp.Discord");
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel, //
                                   "com.discordapp.Discord",
                                   "x86_64",
                                   "stable",
                                   "0.0.24",
                                   "Discord",
                                   QUrl(),
                                   metadataAndOverridesFiles);
        model.setReference(&reference);
        model.load();
        model.setShowAdvanced(true);

        const auto filesystem = QLatin1String("~/path");
        const auto session = QLatin1String("com.example.session");
        const auto system = QLatin1String("com.example.system");
        const auto envName = QLatin1String("SOME_ENV");
        const auto envValue = QLatin1String("abc123");

        model.addUserEnteredPermission(FlatpakPermissionsSectionType::Filesystems,
                                       filesystem,
                                       QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::Create));
        model.addUserEnteredPermission(FlatpakPermissionsSectionType::SessionBus, session, FlatpakPolicy::FLATPAK_POLICY_TALK);
        // Try int cast to make sure QML/JS works fine too.
        model.addUserEnteredPermission(FlatpakPermissionsSectionType::SystemBus, system, FlatpakPolicy::FLATPAK_POLICY_SEE);
        model.addUserEnteredPermission(FlatpakPermissionsSectionType::Environment, envName, envValue);

        for (auto i = 0; i < model.rowCount(); ++i) {
            const QString name = model.data(model.index(i, 0), FlatpakPermissionModel::Name).toString();
            if (name == "host") {
                const auto metaEnum = QMetaEnum::fromType<FlatpakPermissionModel::Roles>();
                for (auto j = 0; j < metaEnum.keyCount(); ++j) { // purely for debugging purposes
                    qDebug() << metaEnum.key(j) << model.data(model.index(i, 0), metaEnum.value(j));
                }

                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled), false);
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                model.togglePermissionAtRow(i);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled), true);
            }

            if (name == "org.kde.StatusNotifierWatcher") {
                model.setPermissionValueAtRow(i, FlatpakPolicy::FLATPAK_POLICY_OWN);
            }

            if (name == "host-os") {
                // Make sure the config manipulation works across multiple changes
                model.setPermissionValueAtRow(i, QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::ReadOnly));
                model.setPermissionValueAtRow(i, QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::ReadWrite));
                model.setPermissionValueAtRow(i, QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::Create));
                model.setPermissionValueAtRow(i, QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::ReadOnly));
            }

            if (name == filesystem) {
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                QVERIFY(!model.data(model.index(i, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakFilesystemsEntry::AccessMode>(),
                         FlatpakFilesystemsEntry::AccessMode::Create);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakFilesystemsEntry::AccessMode>(),
                         FlatpakFilesystemsEntry::AccessMode::Create);
            }

            if (name == session) {
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                QVERIFY(!model.data(model.index(i, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_TALK);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_TALK);
            }

            if (name == system) {
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                QVERIFY(!model.data(model.index(i, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::DefaultValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_SEE);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::EffectiveValue).value<FlatpakPolicy>(), FlatpakPolicy::FLATPAK_POLICY_SEE);
            }

            if (name == envName) {
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::CanBeDisabled).toBool());
                QVERIFY(!model.data(model.index(i, 0), FlatpakPermissionModel::IsDefaultEnabled).toBool());
                QVERIFY(model.data(model.index(i, 0), FlatpakPermissionModel::IsEffectiveEnabled).toBool());
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::DefaultValue).toString(), envValue);
                QCOMPARE(model.data(model.index(i, 0), FlatpakPermissionModel::EffectiveValue).toString(), envValue);
            }
        }
        model.save();
        const KConfig actual(metadataAndOverridesFiles.last());
        const KConfig expected(QFINDTESTDATA("fixtures/overrides.out/com.discordapp.Discord"));
        QVERIFY(operatorFlatpakConfigEquals(actual, expected));
    }

    void testValuesModelForSectionsWithoutModels()
    {
        const auto values = FlatpakPermissionModel::valuesModelForSectionType(FlatpakPermissionsSectionType::Features);
        QCOMPARE(values, nullptr);
    }

    void testValuesModelForFilesystemsSection()
    {
        const auto values = FlatpakPermissionModel::valuesModelForFilesystemsSection();
        QCOMPARE(values->rowCount(QModelIndex()), 4);
        QVERIFY(modelContains(values, i18n("OFF"))); // Filesystems logic is not ready to use this value yet.
        QVERIFY(modelContains(values, i18n("read/write")));
        QVERIFY(modelContains(values, i18n("read-only")));
        QVERIFY(modelContains(values, i18n("create")));
    }

    void testValuesModelForBusSections()
    {
        const auto values = FlatpakPermissionModel::valuesModelForBusSections();
        QCOMPARE(values->rowCount(QModelIndex()), 4);
        QVERIFY(modelContains(values, i18n("None")));
        QVERIFY(modelContains(values, i18n("talk")));
        QVERIFY(modelContains(values, i18n("own")));
        QVERIFY(modelContains(values, i18n("see")));
    }

    void testValuesModelMapping()
    {
        auto expected = FlatpakPermissionModel::valuesModelForFilesystemsSection();
        auto actual = FlatpakPermissionModel::valuesModelForSectionType(FlatpakPermissionsSectionType::Filesystems);
        QCOMPARE(actual, expected);

        expected = FlatpakPermissionModel::valuesModelForBusSections();
        actual = FlatpakPermissionModel::valuesModelForSectionType(FlatpakPermissionsSectionType::SessionBus);
        QCOMPARE(actual, expected);

        actual = FlatpakPermissionModel::valuesModelForSectionType(FlatpakPermissionsSectionType::SystemBus);
        QCOMPARE(actual, expected);
    }

    void testUnparsableFilesystems()
    {
        const auto metadataAndOverridesFiles = mockMetadataAndOverridesFiles("com.example.unparsable.filesystems");
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel,
                                   "com.example.unparsable.filesystems",
                                   "x86_64",
                                   "stable",
                                   "0.0.24",
                                   "Unparsable Filesystems",
                                   QUrl(),
                                   metadataAndOverridesFiles);
        model.setReference(&reference);
        model.load();
        model.setShowAdvanced(true);

        const auto filesystem = QLatin1String("xdg-data/path");
        model.addUserEnteredPermission(FlatpakPermissionsSectionType::Filesystems,
                                       filesystem,
                                       QVariant::fromValue(FlatpakFilesystemsEntry::AccessMode::ReadOnly));
        model.save();

        const KConfig actual(metadataAndOverridesFiles.last());
        const KConfig expected(QFINDTESTDATA("fixtures/overrides.out/com.example.unparsable.filesystems"));
        QVERIFY(operatorFlatpakConfigEquals(actual, expected));
    }

    void testDefaultHomeFilesystem()
    {
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel,
                                   "com.example.home.filesystems",
                                   "x86_64",
                                   "stable",
                                   "0.0.24",
                                   "Default Home Filesystems",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("com.example.home.filesystems"));
        FlatpakPermissionModel model;
        model.setReference(&reference);
        model.load();

        int dConfIndex = -1;
        int homeIndex = -1;

        for (int i = 0; i < model.rowCount(); i++) {
            const auto section = model.data(model.index(i), FlatpakPermissionModel::Section).value<FlatpakPermissionsSectionType::Type>();
            const auto name = model.data(model.index(i), FlatpakPermissionModel::Name).toString();

            if (section == FlatpakPermissionsSectionType::Filesystems) {
                const auto value = model.data(model.index(i), FlatpakPermissionModel::DefaultValue).value<FlatpakFilesystemsEntry::AccessMode>();

                if (name.contains(QLatin1String("dconf"))) {
                    dConfIndex = i;
                    QCOMPARE(value, FlatpakFilesystemsEntry::AccessMode::ReadWrite);
                } else if (name == QLatin1String("home")) {
                    homeIndex = i;
                    QCOMPARE(value, FlatpakFilesystemsEntry::AccessMode::ReadOnly);
                }
            }
        }

        QVERIFY(dConfIndex != -1);
        QVERIFY(homeIndex != -1);
    }

    void testValidNames()
    {
        FlatpakPermissionModel model;
        FlatpakReferencesModel referencesModel;
        FlatpakReference reference(&referencesModel,
                                   "com.example.valid.names",
                                   "x86_64",
                                   "stable",
                                   "1.0",
                                   "Valid Names",
                                   QUrl(),
                                   mockMetadataAndOverridesFiles("com.example.valid.names"));
        model.setReference(&reference);
        model.load();
        model.setShowAdvanced(true);

        QVERIFY(!model.isFilesystemNameValid(QString()));
        QVERIFY(model.permissionExists(FlatpakPermissionsSectionType::Filesystems, QLatin1String("~/path")));
        QVERIFY(model.isFilesystemNameValid(QLatin1String("~/valid/path")));
        QVERIFY(model.isFilesystemNameValid(QLatin1String("~/other")));

        QVERIFY(!model.isDBusServiceNameValid(QString()));
        QVERIFY(!model.isDBusServiceNameValid(QLatin1String("com")));
        QVERIFY(!model.isDBusServiceNameValid(QLatin1String("#$%")));
        QVERIFY(model.permissionExists(FlatpakPermissionsSectionType::SessionBus, QLatin1String("com.example.session")));
        QVERIFY(model.permissionExists(FlatpakPermissionsSectionType::SystemBus, QLatin1String("com.example.system")));
        QVERIFY(model.isDBusServiceNameValid(QLatin1String("com.example.session")));
        QVERIFY(model.isDBusServiceNameValid(QLatin1String("com.example.system")));
        QVERIFY(model.isDBusServiceNameValid(QLatin1String("com.example.session2")));
        QVERIFY(model.isDBusServiceNameValid(QLatin1String("com.example.*")));
        QVERIFY(!model.isDBusServiceNameValid(QLatin1String("com.example.")));

        QVERIFY(!model.isEnvironmentVariableNameValid(QString()));
        QVERIFY(!model.isEnvironmentVariableNameValid(QLatin1String("ABC=DEF")));
        QVERIFY(model.permissionExists(FlatpakPermissionsSectionType::Environment, QLatin1String("EXAMPLE_NAME")));
        QVERIFY(model.isEnvironmentVariableNameValid(QLatin1String("EXAMPLE_NAME")));
        QVERIFY(model.isEnvironmentVariableNameValid(QLatin1String("whatever")));
    }
};

QTEST_MAIN(FlatpakPermissionModelTest)

#include "flatpakpermissiontest.moc"
