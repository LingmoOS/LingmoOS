// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package users

import (
	"testing"

	libdate "github.com/rickb777/date"
	"github.com/stretchr/testify/assert"

	dutils "github.com/linuxdeepin/go-lib/utils"
)

func Test_GetUserInfos(t *testing.T) {
	var names = []string{"test1", "test2", "vbox"}
	infos, err := getUserInfosFromFile("testdata/passwd")
	assert.NoError(t, err)
	assert.Equal(t, len(infos), 3)
	for i, info := range infos {
		assert.Equal(t, info.Name, names[i])
	}
}

func Test_UserInfoValid(t *testing.T) {
	var infos = []struct {
		name  UserInfo
		valid bool
	}{
		{
			UserInfo{Name: "root", Uid: "0", Gid: "0"},
			systemType() == "Server",
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/bash", Uid: "1000", Gid: "1000"},
			true,
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/false", Uid: "1000", Gid: "1000"},
			false,
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/bash", Uid: "60000", Gid: "60000"},
			true,
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/bash", Uid: "999", Gid: "999"},
			false,
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/bash", Uid: "60001", Gid: "60001"},
			false,
		},
		{
			UserInfo{Name: "test1", Shell: "/bin/nologin", Uid: "1000", Gid: "1000"},
			false,
		},
		{
			UserInfo{Name: "test3", Shell: "/bin/bash", Uid: "1000", Gid: "1000"},
			true,
		},
		{
			UserInfo{Name: "test4", Shell: "/bin/bash", Uid: "1000", Gid: "1000"},
			true,
		},
	}

	for _, v := range infos {
		assert.Equal(t, v.name.isHumanUser("testdata/login.defs"), v.valid)
	}
}

func Test_FoundUserInfo(t *testing.T) {
	info, err := getUserInfo(UserInfo{Name: "test1"}, "testdata/passwd")
	assert.NoError(t, err)
	assert.Equal(t, info.Name, "test1")

	info, err = getUserInfo(UserInfo{Uid: "1001"}, "testdata/passwd")
	assert.NoError(t, err)
	assert.Equal(t, info.Name, "test1")

	info, err = getUserInfo(UserInfo{Name: "1006"}, "testdata/passwd")
	assert.Error(t, err)

	info, err = getUserInfo(UserInfo{Uid: "1006"}, "testdata/passwd")
	assert.Error(t, err)

	info, err = getUserInfo(UserInfo{Uid: "1006"}, "testdata/xxxxx")
	assert.Error(t, err)
}

func Test_AdminUser(t *testing.T) {
	var datas = []struct {
		name  string
		admin bool
	}{
		{
			name:  "wen",
			admin: true,
		},
		{
			name:  "test1",
			admin: true,
		},
		{
			name:  "test2",
			admin: false,
		},
	}

	list, err := getAdminUserList("testdata/group", "testdata/sudoers_deepin")
	assert.NoError(t, err)

	for _, data := range datas {
		assert.Equal(t, isStrInArray(data.name, list), data.admin)
	}
}

func TestGetAutoLoginUser(t *testing.T) {
	// lightdm
	name, err := getIniKeys("testdata/autologin/lightdm_autologin.conf",
		kfGroupLightdmSeat,
		[]string{kfKeyLightdmAutoLoginUser}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = getIniKeys("testdata/autologin/lightdm.conf",
		kfGroupLightdmSeat,
		[]string{kfKeyLightdmAutoLoginUser}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "")
	_, err = getIniKeys("testdata/autologin/xxxxx.conf", "", nil, nil)
	assert.Error(t, err)

	// gdm
	name, err = getIniKeys("testdata/autologin/custom_autologin.conf",
		kfGroupGDM3Daemon, []string{kfKeyGDM3AutomaticEnable,
			kfKeyGDM3AutomaticLogin}, []string{"True", ""})
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = getIniKeys("testdata/autologin/custom.conf",
		kfGroupGDM3Daemon, []string{kfKeyGDM3AutomaticEnable,
			kfKeyGDM3AutomaticLogin}, []string{"True", ""})
	assert.NoError(t, err)
	assert.Equal(t, name, "")

	// kdm
	name, err = getIniKeys("testdata/autologin/kdmrc_autologin",
		kfGroupKDMXCore, []string{kfKeyKDMAutoLoginEnable,
			kfKeyKDMAutoLoginUser}, []string{"true", ""})
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = getIniKeys("testdata/autologin/kdmrc",
		kfGroupKDMXCore, []string{kfKeyKDMAutoLoginEnable,
			kfKeyKDMAutoLoginUser}, []string{"true", ""})
	assert.NoError(t, err)
	assert.Equal(t, name, "")

	// sddm
	name, err = getIniKeys("testdata/autologin/sddm_autologin.conf",
		kfGroupSDDMAutologin,
		[]string{kfKeySDDMUser}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = getIniKeys("testdata/autologin/sddm.conf",
		kfGroupSDDMAutologin,
		[]string{kfKeySDDMUser}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "")

	// lxdm
	name, err = getIniKeys("testdata/autologin/lxdm_autologin.conf",
		kfGroupLXDMBase,
		[]string{kfKeyLXDMAutologin}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = getIniKeys("testdata/autologin/lxdm.conf",
		kfGroupLXDMBase,
		[]string{kfKeyLXDMAutologin}, []string{""})
	assert.NoError(t, err)
	assert.Equal(t, name, "")

	// slim
	name, err = parseSlimConfig("testdata/autologin/slim_autologin.conf",
		"", false)
	assert.NoError(t, err)
	assert.Equal(t, name, "wen")
	name, err = parseSlimConfig("testdata/autologin/slim.conf", "", false)
	assert.NoError(t, err)
	assert.Equal(t, name, "")
	// cp 'testdata/autologin/slim.conf' to '/tmp/slim_tmp.conf'
	// _, err = parseSlimConfig("/tmp/slim_tmp.conf", "wen", true)
	// c.Check(err, C.Equals, nil)
	// name, err = parseSlimConfig("/tmp/slim_tmp.conf", "", false)
	// c.Check(err, C.Equals, nil)
	// c.Check(name, C.Equals, "wen")

	m, err := getDefaultDM("testdata/autologin/default-display-manager")
	assert.NoError(t, err)
	assert.Equal(t, m, "lightdm")
	_, err = getDefaultDM("testdata/autologin/xxxxx")
	assert.Error(t, err)
}

func Test_XSession(t *testing.T) {
	session, _ := getIniKeys("testdata/autologin/lightdm.conf", kfGroupLightdmSeat,
		[]string{"user-session"}, []string{""})
	assert.Equal(t, session, "deepin")
	session, _ = getIniKeys("testdata/autologin/sddm.conf", kfGroupSDDMAutologin,
		[]string{kfKeySDDMSession}, []string{""})
	assert.Equal(t, session, "kde-plasma.desktop")
}

func Test_WriteStrvData(t *testing.T) {
	var (
		datas = []string{"123", "abc", "xyz"}
		file  = "/tmp/write_strv"
	)
	err := writeStrvToFile(datas, file, 0644)
	assert.NoError(t, err)

	md5, _ := dutils.SumFileMd5(file)
	assert.Equal(t, md5, "0b188e42e5f8d5bc5a6560ce68d5fbc6")
}

func Test_GetDefaultShell(t *testing.T) {
	shell, err := getDefaultShell("testdata/adduser.conf")
	assert.NoError(t, err)
	assert.Equal(t, shell, "/bin/zsh")

	shell, err = getDefaultShell("testdata/adduser1.conf")
	assert.NoError(t, err)
	assert.Equal(t, shell, "")

	_, err = getDefaultShell("testdata/xxxxx.conf")
	assert.Error(t, err)
}

func Test_StrInArray(t *testing.T) {
	var array = []string{"abc", "123", "xyz"}

	var datas = []struct {
		value string
		ret   bool
	}{
		{
			value: "abc",
			ret:   true,
		},
		{
			value: "xyz",
			ret:   true,
		},
		{
			value: "abcd",
			ret:   false,
		},
	}

	for _, data := range datas {
		assert.Equal(t, isStrInArray(data.value, array), data.ret)
	}
}

func Test_GetAdmGroup(t *testing.T) {
	groups, users, err := getAdmGroupAndUser("testdata/sudoers_deepin")
	assert.NoError(t, err)
	assert.Equal(t, isStrInArray("sudo", groups), true)
	assert.Equal(t, isStrInArray("root", users), true)

	groups, users, err = getAdmGroupAndUser("testdata/sudoers_arch")
	assert.NoError(t, err)
	assert.Equal(t, isStrInArray("sudo", groups), true)
	assert.Equal(t, isStrInArray("wheel", groups), true)
	assert.Equal(t, isStrInArray("root", users), true)
}

func Test_DMFromService(t *testing.T) {
	dm, _ := getDMFromSystemService("testdata/autologin/display-manager.service")
	assert.Equal(t, dm, "lightdm")
}

func Test_IsPasswordExpired(t *testing.T) {
	for _, testCase := range []struct {
		shadowInfo *ShadowInfo
		today      libdate.Date
		result     bool
	}{
		{
			shadowInfo: &ShadowInfo{
				MaxDays:    -1,
				LastChange: 0, // must change
			},
			today:  libdate.New(2019, 12, 6),
			result: true,
		},

		{
			shadowInfo: &ShadowInfo{
				LastChange: 1, // 1970-01-02
				MaxDays:    -1,
			},
			today:  libdate.New(2019, 12, 6),
			result: false,
		},

		{
			shadowInfo: &ShadowInfo{
				MaxDays:    1,
				LastChange: 1, // 1970-01-02
			},
			today:  libdate.New(1970, 1, 3),
			result: false,
		},

		{
			shadowInfo: &ShadowInfo{
				MaxDays:    1,
				LastChange: 1, // 1970-01-02
			},
			today:  libdate.New(1970, 1, 4),
			result: true,
		},
	} {
		assert.Equal(t, isPasswordExpired(testCase.shadowInfo, testCase.today), testCase.result)
	}
}
