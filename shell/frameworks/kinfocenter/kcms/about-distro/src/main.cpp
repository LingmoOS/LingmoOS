/*
    SPDX-FileCopyrightText: 2012-2022 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <iostream>
#include <utility>

#include <QAbstractListModel>
#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QLoggingCategory>
#include <QWindow>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KConfig>
#include <KConfigGroup>
#include <KCoreAddons>
#include <KLocalizedString>
#include <KOSRelease>
#include <KPluginFactory>
#include <KQuickConfigModule>
#include <KSharedConfig>

#include "CPUEntry.h"
#include "GPUEntry.h"
#include "GraphicsPlatformEntry.h"
#include "KernelEntry.h"
#include "MemoryEntry.h"
#include "OSVersionEntry.h"
#include "LingmoEntry.h"
#include "ServiceRunner.h"
#include "ThirdPartyEntry.h"
#include "Version.h"

#include <QHostInfo>
#include <QObject>
#include <QString>
#include <QSysInfo>

#include <sys/sysinfo.h>
#include <sys/types.h>


class EntryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        EntryRole = Qt::UserRole,
    };
    Q_ENUM(Role)

    using QAbstractListModel::QAbstractListModel;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent);
        return m_entries.size();
    }

    [[nodiscard]] QVariant data(const QModelIndex &index, int intRole) const override
    {
        if (!index.isValid()) {
            return {};
        }

        switch (static_cast<Role>(intRole)) {
        case Role::EntryRole:
            return QVariant::fromValue(m_entries.at(index.row()));
        }

        return {};
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override
    {
        static QHash<int, QByteArray> roles;
        if (!roles.isEmpty()) {
            return roles;
        }

        roles = QAbstractListModel::roleNames();
        roles.insert(Role::EntryRole, QByteArrayLiteral("entry"));
        return roles;
    }

    void append(Entry *entry)
    {
        beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
        m_entries.push_back(entry);
        endInsertRows();
    }

private:
    std::vector<Entry *> m_entries;
};
Q_DECLARE_METATYPE(EntryModel *)
Q_DECLARE_METATYPE(Entry *)

class KCMAboutSystem : public KQuickConfigModule
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(KCMAboutSystem) // rule of 5
public:
    explicit KCMAboutSystem(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
        : KQuickConfigModule(parent, data)
        , m_dumpToStdout(args.contains(QStringLiteral("dump")))
    {
        if (m_dumpToStdout) {
            const auto windows = qGuiApp->allWindows();
            for (const auto &window : windows) {
                // Don't steal focus when running in wayland or xcb platform mode. Bit of a hack but the best we can do
                // since we can't move the window off-screen on wayland.
                window->setVisibility(QWindow::Minimized);
            }
            QLoggingCategory::setFilterRules(QStringLiteral("*=false"));
        }

        qmlRegisterType<ServiceRunner>("org.kde.kinfocenter.about_distro.private", 1, 0, "ServiceRunner");

        // https://bugs.kde.org/show_bug.cgi?id=366158
        // When a KCM loads fast enough do a blocking load via the constructor.
        // Otherwise there is a noticeable rendering gap where dummy/no data is
        // shown. Makes it look bad.
        load();
    }

    ~KCMAboutSystem() override
    {
        qDeleteAll(m_entries);
    }

    void load() override
    {
        // load is called lazily, but also from the ctor -> prevent double init.
        if (!m_entries.empty()) {
            return;
        }

        loadOSData();
        loadEntries();

        if (m_dumpToStdout) {
            std::wcout << clipboardText(Entry::Language::English).toStdWString();
            // No more output thank you very much. This for example prevents warnings coming out of libwayland.
            fclose(stderr);
            fclose(stdout);
            QMetaObject::invokeMethod(qApp, &QCoreApplication::quit, Qt::QueuedConnection);
            return;
        }
    }

    void loadOSData()
    {
        // NOTE: do not include globals, otherwise kdeglobals could provide values
        //       even though we only explicitly want them from our own config.
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kcm-about-distrorc"), KConfig::NoGlobals);
        KConfigGroup cg = KConfigGroup(config, "General");

        KOSRelease os;

        QString logoPath = cg.readEntry("LogoPath", os.logo());
        if (logoPath.isEmpty()) {
            logoPath = QStringLiteral("start-here-kde");
        }
        m_distroLogo = logoPath;

        // Default to not show Build
        const bool showBuild = cg.readEntry("ShowBuild", false);

        // Check if distro want's us to show extra values
        const QString entryName = QStringLiteral("ExtraSoftwareData");
        const QStringList entries = cg.readXdgListEntry(entryName, QStringList());
        for (const auto &script : entries) {
            m_extraDataEntries.push_back(new ThirdPartyEntry(script));
        }

        // as a product brand is different from Kubuntu.
        const QString distroName = cg.readEntry("Name", os.name());
        const QString osrVersion = cg.readEntry("UseOSReleaseVersion", false) ? os.version() : os.versionId();
        const QString versionId = cg.readEntry("Version", osrVersion);

        auto versionEntry = new OSVersionEntry(distroName, versionId, showBuild ? os.buildId() : QString());
        // This creates a trailing space if versionId is empty, so trimming String
        // to remove possibly trailing spaces
        m_distroNameVersion = versionEntry->localizedValue().trimmed();
        m_entries.push_back(versionEntry);

        const QString variant = cg.readEntry("Variant", os.variant());
        m_distroVariant = variant;

        const QString url = cg.readEntry("Website", os.homeUrl());
        m_distroUrl = url;

        m_kernelVersion = QSysInfo::kernelVersion();

        Q_EMIT changed();
    }

    static KLocalizedString systemInfoKey(const QString &key)
    {
        if (key == QStringLiteral("system-manufacturer")) {
            return ki18nc("@label", "Manufacturer:");
        }
        if (key == QStringLiteral("system-product-name")) {
            return ki18nc("@label", "Product Name:");
        }
        if (key == QStringLiteral("system-version")) {
            return ki18nc("@label", "System Version:");
        }
        if (key == QStringLiteral("system-serial-number")) {
            return ki18nc("@label", "Serial Number:");
        }
        qFatal("unexpected systeminfo key %s\n", qUtf8Printable(key));
        Q_UNREACHABLE();
        return ki18nc("@label unknown entry in table", "Unknown:");
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    static KLocalizedString fdtNodeName(const QString &fdtNode)
    {
        if (fdtNode == QStringLiteral("model")) {
            return ki18nc("@label", "Product Name:");
        }
        if (fdtNode == QStringLiteral("serial-number")) {
            return ki18nc("@label", "Serial Number:");
        }
        if (fdtNode == QStringLiteral("chosen/u-boot,version")) {
            return ki18nc("@label uboot is the name of a bootloader for embedded devices", "U-Boot Version:");
        }
        qFatal("unexpected devicetree property %s\n", qUtf8Printable(fdtNode));
        Q_UNREACHABLE();
        return ki18nc("@label unknown entry in table", "Unknown:");
    }

    QString fdtGetValue(const QString &fdtNode)
    {
        const QFileInfo targetNode(QLatin1String("/proc/device-tree/") + fdtNode);
        if (targetNode.exists() && targetNode.isFile()) {
            QFile fdtNodeFile(targetNode.filePath());
            if (!fdtNodeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning("Devicetree: could not retrieve value from node %s\n", qUtf8Printable(fdtNode));
                return {};
            }
            return fdtNodeFile.readLine();
        }
        return {};
    }
#endif // Q_OS_LINUX || Q_OS_ANDROID

    void loadEntries()
    {
        auto addEntriesToGrid = [this](EntryModel *model, const std::vector<Entry *> &entries) {
            for (auto entry : entries) {
                if (!entry->isValid()) {
                    delete entry; // since we do not keep it around
                    continue;
                }
                model->append(entry);
                m_entries.push_back(entry);
            }
        };

        // software
        addEntriesToGrid(m_softwareEntries,
                         {new LingmoEntry(),
                          new Entry(ki18n("KDE Frameworks Version:"), KCoreAddons::versionString()),
                          new Entry(ki18n("Qt Version:"), QString::fromLatin1(qVersion())),
                          new KernelEntry(),
                          new GraphicsPlatformEntry()});

        // Add any extraData entries
        if (!m_extraDataEntries.empty()) {
            addEntriesToGrid(m_softwareEntries, m_extraDataEntries);
        }

        // hardware
        addEntriesToGrid(m_hardwareEntries, {new CPUEntry, new MemoryEntry, new GPUEntry});

        KAuth::Action action(QStringLiteral("org.kde.kinfocenter.dmidecode.systeminformation"));
        action.setHelperId(QStringLiteral("org.kde.kinfocenter.dmidecode"));
        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [this, job, addEntriesToGrid] {
            QVariantMap data = job->data();
            static const QString systemSerialNumberKey = QStringLiteral("system-serial-number");
            const QString systemSerialNumber = data.take(systemSerialNumberKey).toString();
            for (auto it = data.cbegin(); it != data.cend(); ++it) {
                addEntriesToGrid(m_hardwareEntries, {new Entry(systemInfoKey(it.key()), it.value().toString())});
            }
            // Insert hidden entries at the end so it doesn't look weird visually to have a button mid-layout.
            if (!systemSerialNumber.isEmpty()) {
                addEntriesToGrid(m_hardwareEntries, {new Entry(systemInfoKey(systemSerialNumberKey), systemSerialNumber, Entry::Hidden::Yes)});
            }

            Q_EMIT changed();
        });
        job->start();

#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
        // FDT nodes should be relative to fdtBase/. Note the lack of leading slashes.
        // Keep nodes which contain personal information at the bottom of the list to avoid mid-layout buttons.
        static const QStringList fdtSupportedInfo = {QStringLiteral("model"), QStringLiteral("chosen/u-boot,version"), QStringLiteral("serial-number")};

        for (const auto &fdtNode : fdtSupportedInfo) {
            const QString fdtValue = fdtGetValue(fdtNode);
            if (!fdtValue.isEmpty()) {
                if (fdtNode == "serial-number") {
                    addEntriesToGrid(m_hardwareEntries, {new Entry(fdtNodeName(fdtNode), fdtValue, Entry::Hidden::Yes)});
                } else {
                    addEntriesToGrid(m_hardwareEntries, {new Entry(fdtNodeName(fdtNode), fdtValue)});
                }
            }
            Q_EMIT changed();
        }
#endif // Q_OS_LINUX || Q_OS_ANDROID

        Q_EMIT changed();
    }

    QString clipboardText(Entry::Language language) const
    {
        QString text;
        for (auto entry : std::as_const(m_entries)) {
            if (entry->isHidden()) {
                continue;
            }
            text += entry->diagnosticLine(language);
        }
        return text;
    }

    Q_INVOKABLE void copyToClipboard() const
    {
        storeInClipboard(clipboardText(Entry::Language::System));
    }

    Q_INVOKABLE void copyToClipboardInEnglish() const
    {
        storeInClipboard(clipboardText(Entry::Language::English));
    }

    Q_SCRIPTABLE static void storeInClipboard(const QString &text)
    {
        QGuiApplication::clipboard()->setText(text.trimmed());
    }

private:
    const bool m_dumpToStdout;
    std::vector<const Entry *> m_entries;
    // Extra data distro wants to show in key/value pairs
    std::vector<Entry *> m_extraDataEntries;

    Q_SIGNAL void changed();

    Q_PROPERTY(EntryModel *softwareEntries MEMBER m_softwareEntries CONSTANT)
    EntryModel *m_softwareEntries = new EntryModel(this);
    Q_PROPERTY(EntryModel *hardwareEntries MEMBER m_hardwareEntries CONSTANT)
    EntryModel *m_hardwareEntries = new EntryModel(this);

    Q_PROPERTY(QString distroLogo MEMBER m_distroLogo NOTIFY changed)
    QString m_distroLogo;
    Q_PROPERTY(QString distroNameVersion MEMBER m_distroNameVersion NOTIFY changed)
    QString m_distroNameVersion;
    Q_PROPERTY(QString distroVariant MEMBER m_distroVariant NOTIFY changed)
    QString m_distroVariant;
    Q_PROPERTY(QString kernelVersion MEMBER m_kernelVersion NOTIFY changed)
    QString m_kernelVersion;
    Q_PROPERTY(QString distroUrl MEMBER m_distroUrl NOTIFY changed)
    QString m_distroUrl;

    Q_PROPERTY(bool isEnglish MEMBER m_isEnglish CONSTANT)
    const bool m_isEnglish = QLocale::system().language() == QLocale::English || QLocale::system().language() == QLocale::C;
    Q_PROPERTY(bool isThisKInfoCenter MEMBER m_isThisKInfoCenter CONSTANT)
    const bool m_isThisKInfoCenter = qGuiApp->desktopFileName() == QLatin1String("org.kde.kinfocenter");
};

K_PLUGIN_CLASS_WITH_JSON(KCMAboutSystem, "kcm_about-distro.json")

#include "main.moc"