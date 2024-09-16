// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package daemon

//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/dde-session-daemon
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/dde-system-daemon
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/grub2
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/search
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/backlight_helper
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/langselector
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/soundeffect
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/dde-lockservice
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/dde-authority
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/default-terminal
//go:generate go build -o target/ github.com/linuxdeepin/dde-daemon/bin/dde-greeter-setter
