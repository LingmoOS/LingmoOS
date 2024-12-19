import apport.packaging

def add_info(report, ui):
    report["InstalledPlugins"] = apport.hookutils.package_versions(
        'lingmo-appstore-plugin-flatpak',
        'lingmo-appstore-plugin-snap')
