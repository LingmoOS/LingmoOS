#!/bin/bash

.PHONY:all kelvinu-dm install preinstall
GOPATH := /usr/share/gocode
GOPATH_DIR = gopath
GOBUILD = go build -v
export GO111MODULE = off

all:kelvinu-dm


kelvinu-dm: dde-setHidden

dde-setHidden:
	env GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}"  ${GOBUILD} -o Xdeepin ${GOBUILD_OPTIONS} setHidden.go

install::
	mkdir -p ${DESTDIR}/usr/share/wayland-sessions/
	install Wayland.desktop ${DESTDIR}/usr/share/wayland-sessions/
	install DeepinOnXwayland.desktop ${DESTDIR}/usr/share/wayland-sessions/
	mkdir -p ${DESTDIR}/usr/bin/
	install dde-session-wayland ${DESTDIR}/usr/bin/
	install runkwin.sh ${DESTDIR}/usr/bin/
	install kwin_wayland_helper ${DESTDIR}/usr/bin/
	install dde_update_dbus_env ${DESTDIR}/usr/bin/
	mkdir -p ${DESTDIR}/var/lib/lastore/
	install dde-session-x11 ${DESTDIR}/usr/bin/
	install runkwin-x11.sh ${DESTDIR}/usr/bin/
	install kwin_wayland-x11_helper ${DESTDIR}/usr/bin/
	chown root Xdeepin
	chgrp root Xdeepin
	install -Dm4755 Xdeepin ${DESTDIR}/usr/bin

uninstall:
	rm -rf ${DESTDIR}/usr/share/wayland-sessions/
	rm -f  ${DESTDIR}/etc/udcp-client.conf
	rm -f  ${DESTDIR}/usr/bin/dde-session-wayland
	rm -f  ${DESTDIR}/usr/bin/runkwin.sh
	rm -f  ${DESTDIR}/usr/bin/dde-session-x11
	rm -f  ${DESTDIR}/usr/bin/runkwin-x11.sh
	rm -f  ${DESTDIR}/usr/bin/Xdeepin
	rm -f  ${DESTDIR}/usr/bin/kwin_wayland-x11_helper
	rm -f  ${DESTDIR}/usr/bin/dde_update_dbus_env
	rm -f  ${DESTDIR}/usr/bin/kwin_wayland_helper
