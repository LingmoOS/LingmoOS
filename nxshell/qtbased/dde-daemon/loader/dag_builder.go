// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package loader

import (
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/dde-daemon/graph"
)

type DAGBuilder struct {
	modules         Modules
	enablingModules []string
	disableModules  map[string]struct{}
	flag            EnableFlag

	log *log.Logger

	dag *graph.Data
}

func NewDAGBuilder(loader *Loader, enablingModules []string, disableModules []string, flag EnableFlag) *DAGBuilder {
	disableModulesMap := map[string]struct{}{}
	for _, name := range disableModules {
		if _, ok := loader.modules[name]; ok {
			loader.log.Warningf("disabled module(%s) is no existed", name)
			continue
		}
		disableModulesMap[name] = struct{}{}
	}

	return &DAGBuilder{
		modules:         loader.modules,
		enablingModules: enablingModules,
		disableModules:  disableModulesMap,
		flag:            flag,
		log:             loader.log,
		dag:             graph.New(),
	}
}

func (builder *DAGBuilder) buildDAG() error {
	logLevel := builder.log.GetLogLevel()
	queue := make([]*graph.Node, 0, len(builder.enablingModules))
	for _, name := range builder.enablingModules {
		node := graph.NewNode(name)
		if builder.dag.AddNode(node) {
			queue = append(queue, node)
		}
	}
	for len(queue) != 0 {
		node := queue[0]
		queue = queue[1:]
		name := node.ID
		module, ok := builder.modules[name]
		if !ok {
			if builder.flag.HasFlag(EnableFlagIgnoreMissingModule) {
				if logLevel == log.LevelDebug {
					builder.log.Info("no such a module named", name)
					continue
				}
			} else {
				return &EnableError{ModuleName: name, Code: ErrorMissingModule}
			}
		}
		if _, ok := builder.disableModules[name]; ok {
			if !builder.flag.HasFlag(EnableFlagForceStart) {
				return &EnableError{ModuleName: name, Code: ErrorConflict}
			}

			// TODO: add a flag: skip module whose dependencies is not disabled.
		}
		dependencies := module.GetDependencies()
		for _, dependency := range dependencies {
			depNode := graph.NewNode(dependency)
			if builder.dag.AddNode(depNode) {
				queue = append(queue, depNode)
			}
			builder.dag.UpdateEdgeWeight(builder.dag.GetNodeByID(dependency), node, 0)
		}
	}
	return nil
}

func (builder *DAGBuilder) Execute() (*graph.Data, error) {
	err := builder.buildDAG()
	if err != nil {
		return nil, err
	}

	return builder.dag, nil
}
