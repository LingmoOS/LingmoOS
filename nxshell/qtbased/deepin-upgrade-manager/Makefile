PRJ=deepin-upgrade-manager
PROG_UPGRADER=${PRJ}
PROG_UPGRADER_TOOL=deepin-upgrade-manager-tool
PROG_BOOTKIT=deepin-boot-kit
PROG_DBUS=org.deepin.AtomicUpgrade1
LANGUAGES_BOOT_KIT = $(basename $(notdir $(wildcard misc/deepin-boot-kit/po/*.po)))
LANGUAGES_UPGRADE_MANAGER = $(basename $(notdir $(wildcard misc/deepin-upgrade-manager/po/*.po)))
PREFIX=/usr
VAR=/var/lib
PWD=$(shell pwd)
GOCODE=/usr/share/gocode
GOPATH_DIR=gopath
CURRENT_DIR=$(notdir $(shell pwd))
ARCH=$(shell arch)
export GO111MODULE=off


all: build

prepare:
	@if [ ! -d ${GOPATH_DIR}/src/${PRJ} ]; then \
		mkdir -p ${GOPATH_DIR}/src/${PRJ}; \
		ln -sf ${PWD}/pkg ${GOPATH_DIR}/src/${PRJ}; \
		ln -sf ${PWD}/cmd ${GOPATH_DIR}/src/${PRJ}/; \
		ln -sf ${PWD}/go.mod ${GOPATH_DIR}/src/${PRJ}; \
		ln -sf ${PWD}/go.sum ${GOPATH_DIR}/src/${PRJ}; \
	fi

$(info, $(GOPATH))
$(warning, $(GOPATH))
$(error, $(GOPATH))


build: prepare
	mkdir -p ${PWD}/out
	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} ls -al ${PWD}/${GOPATH_DIR}/src/deepin-upgrade-manager/*
	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} ls -al ${PWD}/${GOPATH_DIR}/src/deepin-upgrade-manager/cmd/deepin-upgrade-manager/*
	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} ls -al ${PWD}/${GOPATH_DIR}/src/deepin-upgrade-manager/cmd/deepin-boot-kit/*

	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH}  \
	CGO_CPPFLAGS="-D_FORTIFY_SOURCE=2"  CGO_LDFLAGS="-Wl,-z,relro,-z,now" \
	go build -o ${PWD}/${PROG_UPGRADER} $(GO_BUILD_FLAGS) ${PRJ}/cmd/${PROG_UPGRADER}

	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} \
	CGO_CPPFLAGS="-D_FORTIFY_SOURCE=2"  CGO_LDFLAGS="-Wl,-z,relro,-z,now" \
	go build -o ${PWD}/${PROG_BOOTKIT} $(GO_BUILD_FLAGS) ${PRJ}/cmd/${PROG_BOOTKIT}

	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} \
	CGO_CPPFLAGS="-D_FORTIFY_SOURCE=2"  CGO_LDFLAGS="-Wl,-z,relro,-z,now" \
	go build -o ${PWD}/${PROG_UPGRADER_TOOL} $(GO_BUILD_FLAGS) ${PRJ}/cmd/${PROG_UPGRADER_TOOL}

install-upgrader:
#ifeq ($(ARCH),sw_64)
#	mkdir -p ${DESTDIR}/etc/grub.d/sw64
#	cp -f ${PWD}/cmd/grub.d/sw/15_deepin-upgrade-manager ${DESTDIR}/etc/grub.d/sw64
#endif
	@mkdir -p ${DESTDIR}/etc/${PROG_UPGRADER}/
	@cp -f ${PWD}/configs/upgrader/config.simple.json  ${DESTDIR}/etc/${PROG_UPGRADER}/config.json

	@mkdir -p ${DESTDIR}/etc/${PROG_UPGRADER}/ready/
	@cp -f ${PWD}/configs/upgrader/data.simple.yaml  ${DESTDIR}/etc/${PROG_UPGRADER}/ready/data.yaml

	@mkdir -p ${DESTDIR}${VAR}/${PROG_BOOTKIT}/config
	@cp -f ${PWD}/configs/upgrader/tool/atomic.json  ${DESTDIR}${VAR}/${PROG_BOOTKIT}/config/atomic.json

	@mkdir -p ${DESTDIR}${VAR}/${PROG_UPGRADER}/scripts
	@cp -f ${PWD}/configs/upgrader/${PROG_UPGRADER} ${DESTDIR}${VAR}/${PROG_UPGRADER}/scripts/

	@mkdir -p ${DESTDIR}${PREFIX}/share/dbus-1/system.d/
	@cp -f ${PWD}/configs/dbus/${PROG_DBUS}.conf  ${DESTDIR}${PREFIX}/share/dbus-1/system.d/

	@mkdir -p ${DESTDIR}${PREFIX}/share/dbus-1/system-services/
	@cp -f ${PWD}/configs/dbus/${PROG_DBUS}.service  ${DESTDIR}${PREFIX}/share/dbus-1/system-services/

	@mkdir -p ${DESTDIR}${PREFIX}/sbin
	@cp -f ${PWD}/${PROG_UPGRADER} ${DESTDIR}${PREFIX}/sbin

	@mkdir -p ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks
	@cp -f ${PWD}/cmd/initramfs-hook/${PROG_UPGRADER} ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/
	@cp -f ${PWD}/cmd/initramfs-hook/ostree ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/

	@mkdir -p ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/init-bottom
	@cp -f ${PWD}/cmd/initramfs-scripts/${PROG_UPGRADER} ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/init-bottom/

	@mkdir -p ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/local-bottom
	@cp -f ${PWD}/cmd/initramfs-scripts/${PROG_UPGRADER}-partition-mount ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/local-bottom/

install-upgrader-tool:
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${PWD}/${PROG_UPGRADER_TOOL} ${DESTDIR}${PREFIX}/bin

	@mkdir -p ${DESTDIR}${PREFIX}/share/${PROG_UPGRADER}
	@cp -f ${PWD}/configs/upgrader/deepin-upgrade-manager-tool.desktop ${DESTDIR}${PREFIX}/share/${PROG_UPGRADER}/deepin-upgrade-manager-tool.desktop

install-bootkit:
	@mkdir -p ${DESTDIR}/usr/share/${PROG_BOOTKIT}/
	@cp -f ${PWD}/configs/bootkit/config.simple.json  ${DESTDIR}/usr/share/${PROG_BOOTKIT}/config.json

	@mkdir -p ${DESTDIR}${PREFIX}/sbin
	@cp -f ${PWD}/${PROG_BOOTKIT} ${DESTDIR}${PREFIX}/sbin/

	@mkdir -p ${DESTDIR}/etc/grub.d ${DESTDIR}/etc/default/grub.d
	@cp -f ${PWD}/cmd/grub.d/99_deepin-boot-kit ${DESTDIR}/etc/grub.d

	@mkdir -p ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks
	@cp -f ${PWD}/cmd/initramfs-hook/${PROG_BOOTKIT} ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/

	@mkdir -p ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/init-bottom
	@cp -f ${PWD}/cmd/initramfs-scripts/${PROG_BOOTKIT} ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/init-bottom/

	@mkdir -p ${DESTDIR}${VAR}/${PROG_BOOTKIT}

install-plymouth:
	install -d ${DESTDIR}${PREFIX}/share/plymouth/themes
	@cp -rvf ${PWD}/misc/plymouth/themes/deepin-recovery ${DESTDIR}${PREFIX}/share/plymouth/themes/

install-translate:
	install -d ${DESTDIR}${PREFIX}/share/locale
	@cp -rfv ${PWD}/out/locale/* ${DESTDIR}${PREFIX}/share/locale

install: translate-bootkit translate-upgrade install-translate install-bootkit install-upgrader-tool install-upgrader install-plymouth

out/locale/%/LC_MESSAGES/deepin-boot-kit.mo: misc/deepin-boot-kit/po/%.po
	mkdir -p ${PWD}/$(@D)
	msgfmt -o ${PWD}/$@ ${PWD}/$<

out/locale/%/LC_MESSAGES/deepin-upgrade-manager.mo: misc/deepin-upgrade-manager/po/%.po
	mkdir -p ${PWD}/$(@D)
	msgfmt -o ${PWD}/$@ ${PWD}/$<

translate-bootkit: $(addsuffix /LC_MESSAGES/deepin-boot-kit.mo, $(addprefix out/locale/, ${LANGUAGES_BOOT_KIT}))

translate-upgrade: $(addsuffix /LC_MESSAGES/deepin-upgrade-manager.mo, $(addprefix out/locale/, ${LANGUAGES_UPGRADE_MANAGER}))

check_code_quality:
	go vet .

pot:
	xgettext -kTr --language=C -o misc/deepin-boot-kit/po/deepin-boot-kit.pot pkg/bootkit/bootkit.go
	xgettext -kTr --language=C -o misc/deepin-upgrade-manager/po/deepin-upgrade-manager.pot pkg/upgrader/upgrader.go

uninstall-upgrader:
#ifeq ($(ARCH),sw_64)
#	rm -f ${DESTDIR}/etc/grub.d/sw64/15_deepin-upgrade-manager
#endif
	@rm -f ${DESTDIR}${PREFIX}/sbin/${PROG_UPGRADER}
	@rm -f ${DESTDIR}${PREFIX}/share/dbus-1/system.d/${PROG_DBUS}.conf
	@rm -f ${DESTDIR}${PREFIX}/share/dbus-1/system-services/${PROG_DBUS}.service
	@rm -f ${DESTDIR}/etc/${PROG_UPGRADER}/config.json
	@rm -f ${DESTDIR}/etc/${PROG_UPGRADER}/ready/data.yaml
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/ostree
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/${PROG_UPGRADER}
	@rm -f ${DESTDIR}${VAR}/${PROG_UPGRADER}/scripts/${PROG_UPGRADER}
	@rm -f ${DESTDIR}${VAR}/${PROG_BOOTKIT}/config/atomic.json
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/local-bottom/${PROG_UPGRADER}
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/local-bottom/${PROG_UPGRADER}-partition-mount

uninstall-upgrader-tool:
	@rm -f ${DESTDIR}${PREFIX}/bin/${PROG_UPGRADER_TOOL}
	@rm -f ${DESTDIR}${PREFIX}/share/${PROG_UPGRADER}/deepin-upgrade-manager-tool.desktop

uninstall-bootkit:
	@rm -f ${DESTDIR}${PREFIX}/sbin/${PROG_BOOTKIT}
	@rm -f ${DESTDIR}/usr/share/${PROG_BOOTKIT}/config.json
	@rm -f ${DESTDIR}/etc/grub.d/99_deepin-boot-kit
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/hooks/${PROG_BOOTKIT}
	@rm -f ${DESTDIR}${PREFIX}/share/initramfs-tools/scripts/init-bottom/${PROG_BOOTKIT}
	@rm -f ${DESTDIR}${VAR}/${PROG_BOOTKIT}

uninstall-plymouth:
	@rm -rf ${DESTDIR}${PREFIX}/share/plymouth/themes/deepin-recovery

uninstall: uninstall-upgrader uninstall-upgrader-tool uninstall-bootkit uninstall-plymouth

clean:
	@rm -rf ${GOPATH_DIR}
	@rm -rf ${PWD}/${PROG_UPGRADER}
	@rm -rf ${PWD}/${PROG_UPGRADER_TOOL}
	@rm -rf ${PWD}/${PROG_BOOTKIT}
	@rm -rf ${PWD}/out

rebuild: clean build
