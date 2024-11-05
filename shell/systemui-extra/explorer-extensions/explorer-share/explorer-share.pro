TEMPLATE = subdirs

SUBDIRS = $$PWD/service/service.pro $$PWD/app/app.pro $$PWD/test/test.pro

daemonConfig.files = $$PWD/data/org.lingmo.samba.share.config.conf
daemonConfig.path = /usr/share/dbus-1/system.d/

daemonPolicy.files = $$PWD/data/org.lingmo.samba.share.config.policy
daemonPolicy.path = /usr/share/polkit-1/actions

daemonService.files = $$PWD/data/org.lingmo.samba.share.config.service
daemonService.path = /usr/share/dbus-1/system-services

INSTALLS += daemonConfig daemonPolicy daemonService
