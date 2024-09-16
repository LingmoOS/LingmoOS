// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package screenedge

import (
	"github.com/linuxdeepin/go-gir/gio-2.0"
)

type Settings struct {
	gsettings *gio.Settings
}

func NewSettings() *Settings {
	s := new(Settings)
	s.gsettings = gio.NewSettings("com.deepin.dde.zone")
	return s
}

func (s *Settings) GetDelay() int32 {
	return s.gsettings.GetInt("delay")
}

func (s *Settings) SetEdgeAction(name, value string) {
	s.gsettings.SetString(name, value)
}

func (s *Settings) GetEdgeAction(name string) string {
	return s.gsettings.GetString(name)
}

func (s *Settings) GetWhiteList() []string {
	return s.gsettings.GetStrv("white-list")
}

func (s *Settings) GetBlackList() []string {
	return s.gsettings.GetStrv("black-list")
}

func (s *Settings) Destroy() {
	s.gsettings.Unref()
}
