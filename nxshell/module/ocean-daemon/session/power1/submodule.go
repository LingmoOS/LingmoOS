// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"fmt"
)

var submoduleList []func(*Manager) (string, submodule, error)

type submodule interface {
	Start() error
	Destroy()
}

func (m *Manager) initSubmodules() {
	m.submodules = make(map[string]submodule, len(submoduleList))
	// new all submodule
	for _, newMethod := range submoduleList {
		name, submoduleInstance, err := newMethod(m)
		logger.Debug("New submodule:", name)
		if err != nil {
			logger.Warningf("New submodule %v failed: %v", name, err)
			continue
		}
		m.submodules[name] = submoduleInstance
	}
}

func (m *Manager) _startSubmodule(name string) error {
	submodule, ok := m.submodules[name]
	if !ok {
		return fmt.Errorf("%v not exist", name)
	}
	return submodule.Start()
}

func (m *Manager) startSubmodules() {
	startOrder := []string{"PowerSavePlan", "LidSwitchHandler"}
	for _, name := range startOrder {
		logger.Infof("submodule %v start", name)
		err := m._startSubmodule(name)
		if err != nil {
			logger.Warningf("submodule %v start failed: %v", name, err)
		}
	}
}

func (m *Manager) destroySubmodules() {
	if m.submodules != nil {
		for name, submodule := range m.submodules {
			logger.Debug("destroy submodule:", name)
			submodule.Destroy()
		}
		m.submodules = nil
	}
}
