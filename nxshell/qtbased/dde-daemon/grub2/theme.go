// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"sync"

	"github.com/linuxdeepin/go-lib/dbusutil"
)

// Theme is a dbus object which provide properties and methods to
// setup deepin grub2 theme.
type Theme struct {
	g       *Grub2
	service *dbusutil.Service

	PropsMu sync.RWMutex

	//nolint
	signals *struct {
		BackgroundChanged struct{}
	}
}

// NewTheme create Theme object.
func NewTheme(g *Grub2) *Theme {
	theme := &Theme{}
	theme.g = g
	theme.service = g.service
	return theme
}
