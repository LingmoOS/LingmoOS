// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package application

import "testing"

func TestQueryPkgExistInRepo(t *testing.T) {
	status := queryPkgExistInRepo("bluez")
	t.Error(status)
}

func TestComparePkgVersion(t *testing.T) {
	s := comparePkgVersion("1:2018.4.16-1", "20070829-4ubuntu2", ">")
	t.Error(s)
}

func TestGetOrigSysCommitVersion(t *testing.T) {
	version := getOrigSysCommitVersion()
	t.Error(version)
}

func TestQueryPkgDepends(t *testing.T) {
	depends, err := queryPkgDepends("/tmp/deepin-system-upgrade/dde_2022.02.16-1_amd64.deb", "dde")
	if err != nil {
		t.Error(err)
		return
	}
	for _, depend := range depends {
		exist := queryPkgExistInRepo(depend)
		t.Error(depend)
		t.Error(exist)
	}
}
