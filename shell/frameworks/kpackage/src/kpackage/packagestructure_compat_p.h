// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#include <KPackage/Package>
#include <QMap>

class KConfigGroup;
class KDesktopFile;
namespace KPackagePrivate
{
template<typename DesktopFile = KDesktopFile, typename ConfigGroup = KConfigGroup>
/**
 * @param package KPackage which will have the desktop file metadata set to (if present)
 * @param customValueTypes Additional keys and their types that should be read from the desktop file
 */
void convertCompatMetaDataDesktopFile(KPackage::Package *package, const QMap<QString, QMetaType::Type> &customValueTypes = {})
{
    if (const QString legacyPath = package->filePath("metadata"); !legacyPath.isEmpty() && legacyPath.endsWith(QLatin1String(".desktop"))) {
        DesktopFile file(legacyPath);
        const ConfigGroup grp = file.desktopGroup();
        QJsonObject kplugin{
            {QLatin1String("Name"), grp.readEntry("Name")},
            {QLatin1String("Description"), grp.readEntry("Comment")},
            {QLatin1String("Version"), grp.readEntry("X-KDE-PluginInfo-Version")},
            {QLatin1String("Id"), grp.readEntry("X-KDE-PluginInfo-Name")},
        };
        QJsonObject obj{
            {QLatin1String("KPlugin"), kplugin},
            {QLatin1String("KPackageStructure"), grp.readEntry("X-KDE-ServiceTypes")},
        };
        for (auto it = customValueTypes.begin(), end = customValueTypes.end(); it != end; ++it) {
            if (const QString value = grp.readEntry(it.key()); !value.isEmpty()) {
                if (QVariant variant(value); variant.convert(QMetaType(it.value()))) { // Make sure the type in resulting json is what the API caller needs
                    obj.insert(it.key(), QJsonValue::fromVariant(variant));
                }
            }
        }
        package->setMetadata(KPluginMetaData(obj, legacyPath));
    }
}
};
