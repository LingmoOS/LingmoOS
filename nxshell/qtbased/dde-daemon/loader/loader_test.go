// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package loader

import (
	"math/rand"
	"strings"
	"testing"
	"time"

	"github.com/linuxdeepin/go-lib/log"
	"github.com/stretchr/testify/assert"
)

type Test_Module struct {
	*ModuleBase
	dependencies string
	test         *testing.T
}

type testItem struct {
	input  Modules
	output error
}

func NewTestModule(name, dependencies string, t *testing.T) *Test_Module {
	daemon := new(Test_Module)
	logger := log.NewLogger(name)
	daemon.ModuleBase = NewModuleBase(name, daemon, logger)
	daemon.test = t
	daemon.dependencies = dependencies
	return daemon
}

func (d *Test_Module) GetDependencies() []string {
	if d.dependencies == "" {
		return nil
	}
	return strings.Split(d.dependencies, " ")
}

func (d *Test_Module) Start() error {
	time.Sleep(time.Duration(int32(time.Second) * rand.Int31n(3)))
	return nil
}

func (d *Test_Module) Stop() error {
	return nil
}

func Test_Loader(t *testing.T) {
	testItems := []testItem{
		{
			Modules{
				"1": NewTestModule("1", "", t),
				"2": NewTestModule("2", "", t),
				"3": NewTestModule("3", "", t),
				"4": NewTestModule("4", "", t),
				"5": NewTestModule("5", "", t),
				"6": NewTestModule("6", "", t),
			},
			nil,
		},
		{
			Modules{
				"1": NewTestModule("1", "2", t),
				"2": NewTestModule("2", "3", t),
				"3": NewTestModule("3", "4", t),
				"4": NewTestModule("4", "5", t),
				"5": NewTestModule("5", "6", t),
				"6": NewTestModule("6", "", t),
			},
			nil,
		},
		{
			Modules{
				"1": NewTestModule("1", "2", t),
				"2": NewTestModule("2", "3", t),
				"3": NewTestModule("3", "4", t),
				"4": NewTestModule("4", "5", t),
				"5": NewTestModule("5", "6", t),
				"6": NewTestModule("6", "1", t),
			},
			&EnableError{Code: ErrorCircleDependencies},
		},
	}
	for _, data := range testItems {
		_loader = &Loader{
			modules: Modules{},
			log:     log.NewLogger("daemon/loader"),
		}
		allModules := []string{}
		for name, module := range data.input {
			Register(module)
			allModules = append(allModules, name)
		}
		err := EnableModules(allModules, nil, EnableFlagNone)
		assert.Equal(t, err, data.output)
	}
}
