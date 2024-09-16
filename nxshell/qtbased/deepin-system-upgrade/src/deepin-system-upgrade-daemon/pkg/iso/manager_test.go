// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package iso

import (
	"os"
	"strconv"
	"strings"
	"testing"
)

func TestIsFileExist(t *testing.T) {
	err := isFileExist("/etc/resolv.conf")
	t.Error(err)
}

func TestSetPlymouthTheme(t *testing.T) {
	f, _ := os.Open("/")
	chrootIsoDir("/home/lxxx/Downloads/isotest/iso/extract/live/squashfs-root")
	setPlymouthTheme()
	exitChroot(f)
}

func TestPrepareRootDir(t *testing.T) {
	root := "/home/lxxx/Downloads/isotest/iso/extract/live/squashfs-root"
	err := prepareRootDir(root)
	if err != nil {
		t.Error(err)
	}
}

func TestMigrateUserInfoForShadow(t *testing.T) {
	userList := []string{"lxxx"}
	root := "/home/lxxx/Downloads/isotest/iso/extract/live/squashfs-root"
	err := migrateUserInfoForShadow(userList, root)
	if err != nil {
		t.Error(err)
	}
}

func TestAppendUserToGroupFile(t *testing.T) {
	userList := []string{"lxxx"}
	root := "/home/lxxx/Downloads/isotest/iso/extract/live/squashfs-root"
	err := appendUserToGroupFile(userList, root)
	if err != nil {
		t.Error(err)
	}
}

func TestGetCurrentGroups(t *testing.T) {
	userName := "lxxx"
	groups, err := getCurrentGroups(userName)
	if err != nil {
		t.Error(err)
	}
	if groups == nil {
		t.Error("get empty groups")
	}
}

func TestParseUserFromPasswdFile(t *testing.T) {
	var cfgPath = "/etc/passwd"
	userList, err := parsePwdUserFromConfig(cfgPath)
	if err != nil {
		t.Error(err)
	}
	if len(userList) <= 0 {
		t.Error("parse passwd file get empty list")
	}
	for _, user := range userList {
		uid, err := strconv.Atoi(strings.Split(user, ":")[2])
		if err != nil {
			t.Error(err)
		}
		if uid < 1000 {
			t.Errorf("parse passwd file get error uid: %d", uid)
		}
	}
}
