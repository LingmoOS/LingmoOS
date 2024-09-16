// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package users

/*
#cgo CFLAGS: -W -Wall -g  -fstack-protector-all -fPIC
#cgo LDFLAGS: -lcrypt

#include <stdlib.h>
#include <shadow.h>
#include "passwd.h"
#include  <pwd.h>
#include  <grp.h>
*/
import "C"

import (
	"errors"
	"fmt"
	"math"
	"os"
	"sort"
	"strconv"
	"strings"
	"sync"
	"unsafe"
)

var (
	wLocker sync.Mutex
)

func EncodePasswd(words string) string {
	cwords := C.CString(words)
	defer C.free(unsafe.Pointer(cwords))

	return C.GoString(C.mkpasswd(cwords))
}

func ExistPwUid(uid uint32) int {
	return int(C.exist_pw_uid(C.uint(uid)))
}

func GetPwName(uid uint32) string {
	return C.GoString(C.get_pw_name(C.uint(uid)))
}

func GetPwGecos(uid uint32) string {
	var pwGecos string
	pwGecos = C.GoString(C.get_pw_gecos(C.uint(uid)))

	return strings.Split(pwGecos, ",")[0]
}

func GetPwUid(uid uint32) string {
	return strconv.FormatUint(uint64(C.get_pw_uid(C.uint(uid))), 10)
}

func GetPwGid(uid uint32) string {
	return strconv.FormatUint(uint64(C.get_pw_gid(C.uint(uid))), 10)
}

func GetPwDir(uid uint32) string {
	return C.GoString(C.get_pw_dir(C.uint(uid)))
}

func GetPwShell(uid uint32) string {
	return C.GoString(C.get_pw_shell(C.uint(uid)))
}

// 通过当前用户的uid获取当前账号的Group
func GetADUserGroupsByUID(uid uint32) ([]string, error) {
	var result []string

	pwent := C.getpwuid(C.uint(uid))
	if pwent == nil {
		return nil, fmt.Errorf("Invalid uuid: %d", uid)
	}

	gid := C.uint(pwent.pw_gid)
	groupPtr := C.getgrgid(gid)
	if groupPtr == nil {
		return nil, fmt.Errorf("Invalid gid: %d", gid)
	}

	result = append(result, C.GoString(groupPtr.gr_name))
	sort.Strings(result)

	return result, nil
}

// 判断用户是否是LDAP网络账户，LDAP域账户信息只能由服务端设置，本地没有保存
func IsLDAPDomainUserID(uid string) bool {
	id, _ := strconv.Atoi(uid)

	domainUserGroups, err := GetADUserGroupsByUID(uint32(id))
	if err != nil {
		return false
	}

	for _, v := range domainUserGroups {
		if strings.Contains(v, "domain") {
			return true
		}
	}

	return false
}

type originShadowInfo struct {
	Name       string
	LastChange int
	MaxDays    int
	ShadowPwdp string // password status
}

func getSpwd(username string) (*originShadowInfo, error) {
	cname := C.CString(username)
	defer C.free(unsafe.Pointer(cname))
	spwd := C.getspnam(cname)
	if spwd == nil {
		return &originShadowInfo{}, errors.New("no such user name")
	}
	sInfo := originShadowInfo{
		Name:       C.GoString(spwd.sp_namp),
		LastChange: int(spwd.sp_lstchg),
		MaxDays:    int(spwd.sp_max),
		ShadowPwdp: C.GoString(spwd.sp_pwdp),
	}
	return &sInfo, nil
}

func writeStrvToFile(datas []string, file string, mode os.FileMode) error {
	var content string
	for i, v := range datas {
		if i != 0 {
			content += "\n"
		}

		content += v
	}

	f, err := os.Create(file + ".bak~")
	if err != nil {
		return err
	}
	defer func() {
		_ = f.Close()
	}()

	wLocker.Lock()
	defer wLocker.Unlock()
	_, err = f.WriteString(content)
	if err != nil {
		return err
	}

	err = f.Sync()
	if err != nil {
		return err
	}

	info, err := os.Stat(file)
	if err == nil && mode == math.MaxUint32 {
		mode = info.Mode()
	}

	os.Rename(file+".bak~", file)
	os.Chmod(file, mode)

	return nil
}
