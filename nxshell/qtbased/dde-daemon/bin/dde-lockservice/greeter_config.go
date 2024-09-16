// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"os"
	"path"

	"github.com/linuxdeepin/go-lib/keyfile"
	"github.com/linuxdeepin/go-lib/utils"
)

const (
	greeterUserConfig   = "/var/lib/lightdm/lightdm-deepin-greeter/state_user"
	greeterGroupGeneral = "General"
	greeterKeyLastUser  = "last-user"
)

func setGreeterUser(file, username string) error {
	if !utils.IsFileExist(file) {
		err := os.MkdirAll(path.Dir(file), 0755)
		if err != nil {
			return err
		}
		err = utils.CreateFile(file)
		if err != nil {
			return err
		}
	}
	kf, err := loadKeyFile(file)
	if err != nil {
		kf = nil
		return err
	}

	v, _ := kf.GetString(greeterGroupGeneral, greeterKeyLastUser)
	if v == username {
		return nil
	}

	kf.SetString(greeterGroupGeneral, greeterKeyLastUser, username)
	return kf.SaveToFile(file)
}

func getGreeterUser(file string) (string, error) {
	kf, err := loadKeyFile(file)
	if err != nil {
		kf = nil
		return "", err
	}

	return kf.GetString(greeterGroupGeneral, greeterKeyLastUser)
}

var _kf *keyfile.KeyFile

func loadKeyFile(file string) (*keyfile.KeyFile, error) {
	if _kf != nil {
		return _kf, nil
	}

	var kf = keyfile.NewKeyFile()
	return kf, kf.LoadFromFile(file)
}
