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
type Fstart struct {
	g       *Grub2
	service *dbusutil.Service

	PropsMu sync.RWMutex
	// props:
	IsSkipGrub   bool
}

// NewTheme create Theme object.
func NewFstart(g *Grub2) *Fstart {
	fstart := &Fstart{}
	fstart.g = g
	fstart.service = g.service
	isSkipGrub, lineNum := getFstartState()
	if lineNum == -1 {
		logger.Warning(deepinFstartFile + "is illegal!")
	}
	fstart.IsSkipGrub = isSkipGrub
	return fstart
}
