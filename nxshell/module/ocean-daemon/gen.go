// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package daemon

//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/ocean-session-daemon
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/ocean-system-daemon
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/grub2
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/search
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/backlight_helper
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/langselector
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/soundeffect
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/ocean-lockservice
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/ocean-authority
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/default-terminal
//go:generate go build -o target/ github.com/lingmoos/ocean-daemon/bin/ocean-greeter-setter
