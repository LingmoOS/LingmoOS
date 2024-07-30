set(%{APPNAMELC}_ICONS
  16-apps-%{APPNAMELC}.png
  32-apps-%{APPNAMELC}.png
  48-apps-%{APPNAMELC}.png
)

ecm_install_icons(ICONS ${%{APPNAMELC}_ICONS} DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)
