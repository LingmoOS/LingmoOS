#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>

#include <QDebug>

int main(int, char**)
{
    KConfig globals("kdeglobals");
    KConfigGroup general(&globals, QStringLiteral("General"));
    if (general.readEntry("ColorScheme") != QLatin1String("Ocean")) {
        return 0;
    }
    QString oceanLightPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/OceanLight.colors"));
    if (oceanLightPath.isEmpty()) {
        return 0;
    }
    KConfig oceanLight(oceanLightPath, KConfig::SimpleConfig);
    for (const auto &group : oceanLight.groupList()) {
        auto destination = KConfigGroup(&globals, group);
        KConfigGroup(&oceanLight, group).copyTo(&destination, KConfig::Notify);
    }
    return 0;
}
