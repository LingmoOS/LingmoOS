PREFIX = /usr
GOPATH_DIR = gopath
GOPKG_PREFIX = github.com/linuxdeepin/dde-daemon
GOBUILD = go build $(GO_BUILD_FLAGS)
export GOPATH=$(shell go env GOPATH)

ifneq (${shell uname -m}, mips64el)
    GOBUILD_OPTIONS = -ldflags '-linkmode=external -extldflags "-pie"'
endif

TEST = \
    ${GOPKG_PREFIX}/accounts1 \
    ${GOPKG_PREFIX}/accounts1/checkers \
    ${GOPKG_PREFIX}/accounts1/logined \
    ${GOPKG_PREFIX}/accounts1/users \
    ${GOPKG_PREFIX}/appinfo \
    ${GOPKG_PREFIX}/apps1 \
    ${GOPKG_PREFIX}/audio1 \
    ${GOPKG_PREFIX}/bin/backlight_helper \
    ${GOPKG_PREFIX}/bin/backlight_helper/ddcci \
    ${GOPKG_PREFIX}/bin/dde-authority \
    ${GOPKG_PREFIX}/bin/dde-greeter-setter \
    ${GOPKG_PREFIX}/bin/dde-lockservice \
    ${GOPKG_PREFIX}/bin/dde-session-daemon \
    ${GOPKG_PREFIX}/bin/dde-system-daemon \
    ${GOPKG_PREFIX}/bin/default-terminal \
    ${GOPKG_PREFIX}/bin/grub2 \
    ${GOPKG_PREFIX}/bin/langselector \
    ${GOPKG_PREFIX}/bin/search \
    ${GOPKG_PREFIX}/bin/soundeffect \
    ${GOPKG_PREFIX}/bin/user-config \
    ${GOPKG_PREFIX}/bluetooth1 \
    ${GOPKG_PREFIX}/calltrace \
    ${GOPKG_PREFIX}/clipboard1 \
    ${GOPKG_PREFIX}/clipboard1/mocks \
    ${GOPKG_PREFIX}/common/bluetooth \
    ${GOPKG_PREFIX}/common/dsync \
    ${GOPKG_PREFIX}/common/sessionmsg \
    ${GOPKG_PREFIX}/dbus \
    ${GOPKG_PREFIX}/debug \
    ${GOPKG_PREFIX}/fprintd1 \
    ${GOPKG_PREFIX}/fprintd1/common \
    ${GOPKG_PREFIX}/gesture1 \
    ${GOPKG_PREFIX}/graph \
    ${GOPKG_PREFIX}/grub2 \
    ${GOPKG_PREFIX}/grub_common \
    ${GOPKG_PREFIX}/grub_gfx \
    ${GOPKG_PREFIX}/housekeeping \
    ${GOPKG_PREFIX}/image_effect1 \
    ${GOPKG_PREFIX}/inputdevices1 \
    ${GOPKG_PREFIX}/inputdevices1/iso639 \
    ${GOPKG_PREFIX}/iw \
    ${GOPKG_PREFIX}/keybinding1 \
    ${GOPKG_PREFIX}/keybinding1/shortcuts \
    ${GOPKG_PREFIX}/keybinding1/util \
    ${GOPKG_PREFIX}/langselector1 \
    ${GOPKG_PREFIX}/lastore1 \
    ${GOPKG_PREFIX}/loader \
    ${GOPKG_PREFIX}/network1 \
    ${GOPKG_PREFIX}/network1/nm \
    ${GOPKG_PREFIX}/network1/nm_generator \
    ${GOPKG_PREFIX}/network1/proxychains \
    ${GOPKG_PREFIX}/screenedge1 \
    ${GOPKG_PREFIX}/screensaver1 \
    ${GOPKG_PREFIX}/service_trigger \
    ${GOPKG_PREFIX}/session/common \
    ${GOPKG_PREFIX}/session/eventlog \
    ${GOPKG_PREFIX}/session/power1 \
    ${GOPKG_PREFIX}/session/uadpagent1 \
    ${GOPKG_PREFIX}/sessionwatcher1 \
    ${GOPKG_PREFIX}/soundeffect1 \
    ${GOPKG_PREFIX}/system/airplane_mode1 \
    ${GOPKG_PREFIX}/system/bluetooth1 \
    ${GOPKG_PREFIX}/system/display1 \
    ${GOPKG_PREFIX}/system/gesture1 \
    ${GOPKG_PREFIX}/system/hostname \
    ${GOPKG_PREFIX}/system/inputdevices1 \
    ${GOPKG_PREFIX}/system/keyevent1 \
    ${GOPKG_PREFIX}/system/lang \
    ${GOPKG_PREFIX}/system/network1 \
    ${GOPKG_PREFIX}/system/power1 \
    ${GOPKG_PREFIX}/system/power_manager1 \
    ${GOPKG_PREFIX}/system/resource_ctl \
    ${GOPKG_PREFIX}/system/scheduler \
    ${GOPKG_PREFIX}/system/swapsched1 \
    ${GOPKG_PREFIX}/system/systeminfo1 \
    ${GOPKG_PREFIX}/system/timedate1 \
    ${GOPKG_PREFIX}/system/uadp1 \
    ${GOPKG_PREFIX}/systeminfo1 \
    ${GOPKG_PREFIX}/timedate1 \
    ${GOPKG_PREFIX}/trayicon1 \
    ${GOPKG_PREFIX}/x_event_monitor1 \
    ${GOPKG_PREFIX}/bin/default-file-manager
    #${GOPKG_PREFIX}/timedate1/zoneinfo \

BINARIES =  \
	    dde-session-daemon \
	    dde-system-daemon \
	    grub2 \
	    search \
	    backlight_helper \
	    langselector \
	    soundeffect \
	    dde-lockservice \
	    dde-authority \
	    default-terminal \
	    dde-greeter-setter \
	    default-file-manager

LANGUAGES = $(basename $(notdir $(wildcard misc/po/*.po)))

CFLAGS = -W -Wall -fPIC -fstack-protector-all -z relro -z noexecstack -z now -pie

all: build

prepare:
	@mkdir -p out/bin
	@mkdir -p ${GOPATH_DIR}/src/$(dir ${GOPKG_PREFIX});
	@ln -snf ../../../.. ${GOPATH_DIR}/src/${GOPKG_PREFIX};

out/bin/%: prepare
	env GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}" ${GOBUILD} -o $@ ${GOBUILD_OPTIONS} ${GOPKG_PREFIX}/bin/${@F}

out/bin/desktop-toggle: bin/desktop-toggle/main.c
	gcc $^ $(shell pkg-config --cflags --libs x11) $(CFLAGS) -o $@

out/locale/%/LC_MESSAGES/dde-daemon.mo: misc/po/%.po
	mkdir -p $(@D)
	msgfmt -o $@ $<

translate: $(addsuffix /LC_MESSAGES/dde-daemon.mo, $(addprefix out/locale/, ${LANGUAGES}))

pot:
	deepin-update-pot misc/po/locale_config.ini

POLICIES=accounts grub2 fprintd
ts:
	for i in $(POLICIES); do \
		deepin-policy-ts-convert policy2ts misc/polkit-action/org.deepin.dde.$$i.policy.in misc/ts/org.deepin.dde.$$i.policy; \
	done

ts_to_policy:
	for i in $(POLICIES); do \
	deepin-policy-ts-convert ts2policy misc/polkit-action/org.deepin.dde.$$i.policy.in misc/ts/org.deepin.dde.$$i.policy misc/polkit-action/org.deepin.dde.$$i.policy; \
	done

build: prepare out/bin/default-terminal out/bin/default-file-manager out/bin/desktop-toggle $(addprefix out/bin/, ${BINARIES}) ts_to_policy icons translate

test: prepare
	env GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}" go test -v ${TEST}
test-coverage: prepare
	env GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}" go test -cover -v ./... | awk '$$2 ~ "_${CURDIR}" {print $$2","$$5}' | sed "s:${CURDIR}::g" | sed 's/files\]/0\.0%/g' > coverage.csv

print_gopath: prepare
	GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}"

install: build install-dde-data install-icons
	mkdir -pv ${DESTDIR}${PREFIX}/lib/deepin-daemon
	cp -f out/bin/* ${DESTDIR}${PREFIX}/lib/deepin-daemon/

	mkdir -pv ${DESTDIR}${PREFIX}/share/locale
	cp -r out/locale/* ${DESTDIR}${PREFIX}/share/locale

	mkdir -pv ${DESTDIR}${PREFIX}/share/dbus-1/system.d
	cp misc/conf/*.conf ${DESTDIR}${PREFIX}/share/dbus-1/system.d/

	mkdir -pv ${DESTDIR}${PREFIX}/share/dbus-1
	cp -r misc/services ${DESTDIR}${PREFIX}/share/dbus-1/
	cp -r misc/system-services ${DESTDIR}${PREFIX}/share/dbus-1/

	mkdir -pv ${DESTDIR}${PREFIX}/share/polkit-1/actions
	cp misc/polkit-action/*.policy ${DESTDIR}${PREFIX}/share/polkit-1/actions/

	mkdir -pv ${DESTDIR}/var/lib/polkit-1/localauthority/10-vendor.d
	cp misc/polkit-localauthority/*.pkla ${DESTDIR}/var/lib/polkit-1/localauthority/10-vendor.d/

	mkdir -pv ${DESTDIR}${PREFIX}/share/dde-daemon
	cp -r misc/dde-daemon/*   ${DESTDIR}${PREFIX}/share/dde-daemon/
	cp -r misc/usr/share/deepin ${DESTDIR}${PREFIX}/share/

	mkdir -pv ${DESTDIR}/lib/systemd/system/
	cp -f misc/systemd/services/system/* ${DESTDIR}/lib/systemd/system/

	mkdir -p $(DESTDIR)$(PREFIX)/lib/systemd/user/dde-session-initialized.target.wants/
	ln -s $(PREFIX)/lib/systemd/user/org.dde.session.Daemon1.service $(DESTDIR)$(PREFIX)/lib/systemd/user/dde-session-initialized.target.wants/org.dde.session.Daemon1.service

	mkdir -pv ${DESTDIR}/etc/pam.d/
	cp -f misc/etc/pam.d/* ${DESTDIR}/etc/pam.d/

	mkdir -pv ${DESTDIR}/etc/default/grub.d
	cp -f misc/etc/default/grub.d/* ${DESTDIR}/etc/default/grub.d

	mkdir -pv ${DESTDIR}/etc/deepin
	cp -f misc/etc/deepin/* ${DESTDIR}/etc/deepin

	mkdir -pv ${DESTDIR}/etc/acpi/events
	cp -f misc/etc/acpi/events/* ${DESTDIR}/etc/acpi/events/

	mkdir -pv ${DESTDIR}/etc/acpi/actions
	cp -f misc/etc/acpi/actions/* ${DESTDIR}/etc/acpi/actions/

	mkdir -pv ${DESTDIR}/etc/pulse/daemon.conf.d
	cp -f misc/etc/pulse/daemon.conf.d/*.conf ${DESTDIR}/etc/pulse/daemon.conf.d/

	mkdir -pv ${DESTDIR}/lib/udev/rules.d
	cp -f misc/udev-rules/*.rules ${DESTDIR}/lib/udev/rules.d/

	mkdir -pv ${DESTDIR}${PREFIX}/lib/deepin-daemon/service-trigger
	cp -f misc/service-trigger/*.json ${DESTDIR}${PREFIX}/lib/deepin-daemon/service-trigger/

	mkdir -pv ${DESTDIR}/etc/NetworkManager/conf.d
	cp -f misc/etc/NetworkManager/conf.d/* ${DESTDIR}/etc/NetworkManager/conf.d/

	mkdir -pv ${DESTDIR}${PREFIX}/libexec/dde-daemon/
	cp -r misc/libexec/dde-daemon/* ${DESTDIR}${PREFIX}/libexec/dde-daemon/

	mkdir -pv ${DESTDIR}${PREFIX}/share/dsg/configs/org.deepin.dde.daemon/
	cp -r misc/dsg-configs/*.json ${DESTDIR}${PREFIX}/share/dsg/configs/org.deepin.dde.daemon/

	cp -f misc/scripts/dde-lock.sh ${DESTDIR}${PREFIX}/lib/deepin-daemon/
	cp -f misc/scripts/dde-shutdown.sh ${DESTDIR}${PREFIX}/lib/deepin-daemon/
install-dde-data:
	mkdir -pv ${DESTDIR}${PREFIX}/share/dde/
	cp -r misc/data ${DESTDIR}${PREFIX}/share/dde/

icons:
	python3 misc/icons/install_to_hicolor.py -d status -o out/icons misc/icons/status

install-icons: icons
	mkdir -pv ${DESTDIR}${PREFIX}/share/icons/
	cp -r out/icons/hicolor ${DESTDIR}${PREFIX}/share/icons/

clean:
	rm -rf ${GOPATH_DIR}
	rm -rf out

rebuild: clean build

check_code_quality: prepare
	env GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}" go vet ./...
