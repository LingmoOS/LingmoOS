// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package checkers

import (
	"errors"
	"io/ioutil"
	"os/user"
	"regexp"
	"strconv"
	"strings"
)

func Tr(text string) string {
	return text
}

const (
	userNameMaxLength = 32
	userNameMinLength = 3

	passwdFile = "/etc/passwd"
	groupFile  = "/etc/group"
)

type ErrorCode int32

type ErrorInfo struct {
	Code  ErrorCode
	Error error
}

const (
	ErrCodeEmpty ErrorCode = iota + 1
	ErrCodeInvalidChar
	ErrCodeFirstCharInvalid
	ErrCodeExist
	ErrCodeNameInGroup
	ErrCodeSystemUsed
	ErrCodeLen
)

func (code ErrorCode) Error() *ErrorInfo {
	var err error
	switch code {
	case ErrCodeEmpty:
		err = errors.New(Tr("Username cannot be empty"))
	case ErrCodeInvalidChar:
		// NOTE: 一般由界面上控制可以键入的字符，不需要做翻译了。
		err = errors.New("Username must only contain a~z, A-Z, 0~9, - or _")
	case ErrCodeFirstCharInvalid:
		err = errors.New(Tr("The first character must be a letter or number"))
	case ErrCodeExist, ErrCodeSystemUsed:
		//提示校验项目与全名、用户名是否相同
		err = errors.New(Tr("The username already exists"))
	case ErrCodeNameInGroup:
		//提示校验项目与用户组名是否相同
		err = errors.New(Tr("The group name already exists"))
	case ErrCodeLen:
		err = errors.New(Tr("Username must be between 3 and 32 characters"))
	default:
		return nil
	}

	return &ErrorInfo{
		Code:  code,
		Error: err,
	}
}

func CheckUsernameValid(name string) *ErrorInfo {
	length := len(name)
	if length == 0 {
		return ErrCodeEmpty.Error()
	}

	if length > userNameMaxLength || length < userNameMinLength {
		return ErrCodeLen.Error()
	}

	if Username(name).isNameExist() {
		id, err := Username(name).getUid()
		if err != nil || id >= 1000 {
			return ErrCodeExist.Error()
		} else {
			return ErrCodeSystemUsed.Error()
		}
	}

	// in euler version, linux kernel is 4.19.90
	// do not allow create user already token by group
	if Username(name).isNameInGroup() {
		return ErrCodeNameInGroup.Error()
	}

	if !Username(name).isFirstCharValid() {
		return ErrCodeFirstCharInvalid.Error()
	}

	if !Username(name).isStringValid() {
		return ErrCodeInvalidChar.Error()
	}

	return nil
}

type Username string

type UsernameList []string

func (name Username) isNameExist() bool {
	names, err := getAllUsername(passwdFile)
	if err != nil {
		return false
	}

	if !isStrInArray(string(name), names) {
		return false
	}

	return true
}

func (name Username) isNameInGroup() bool {
	names, err := getAllUsername(groupFile)
	if err != nil {
		return false
	}
	if !isStrInArray(string(name), names) {
		return false
	}
	return true
}

func (name Username) isStringValid() bool {
	match := regexp.MustCompile(`^[a-zA-Z0-9_-]+$`)
	return match.MatchString(string(name))
}

func (name Username) isFirstCharValid() bool {
	match := regexp.MustCompile(`^[a-zA-Z0-9]`)
	return match.MatchString(string(name))
}

func (name Username) getUid() (int64, error) {
	u, err := user.Lookup(string(name))
	if err != nil {
		return -1, err
	}

	id, err := strconv.ParseInt(u.Uid, 10, 64)
	if err != nil {
		return -1, err
	}

	return id, nil
}

func getAllUsername(file string) (UsernameList, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var names UsernameList
	lines := strings.Split(string(content), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}

		items := strings.Split(line, ":")
		if len(items) < 3 {
			continue
		}

		names = append(names, items[0])
	}

	return names, nil
}
