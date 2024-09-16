// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package repo

import (
	"deepin-upgrade-manager/pkg/module/repo/branch"
	"deepin-upgrade-manager/pkg/module/repo/ostree"
	"fmt"
)

type Repository interface {
	Init() error
	Exist(branchName string) bool
	Last() (string, error)
	First() (string, error)
	List() (branch.BranchList, error)
	ListByName(branchName string, offset, limit int) (branch.BranchList, int, error)
	Snapshot(branchName, dstDir string) error
	Commit(branchName, subject, dataDir string) error
	Diff(baseBranch, targetBranch, dstFile string) error
	Cat(branchName, filepath, dstFile string) error
	Previous(targetName string) (string, error)
	Delete(version string) error
	Subject(branchName string) (string, error)
	CommitTime(branchName string) (string, error)
}

const (
	REPO_TY_OSTREE = iota + 1
)

func NewRepo(ty int, dir string) (Repository, error) {
	var _repo Repository
	switch ty {
	case REPO_TY_OSTREE:
		_repo, _ = ostree.NewRepo(dir)
	default:
		return nil, fmt.Errorf("unknown repo type: %d", ty)
	}

	return _repo, nil
}
