TEMPLATE = subdirs

SUBDIRS += \
    liblingmo-app-widget \
    lingmo-appwidget-test \
    lingmo-appwidget-manager

lingmo-appwidget-test.depends += liblingmo-app-widget
lingmo-appwidget-manager.depends += liblingmo-app-widget
