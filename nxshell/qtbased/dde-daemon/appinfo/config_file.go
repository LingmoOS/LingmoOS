// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package appinfo

// #cgo pkg-config: glib-2.0
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
// #include <glib.h>
import "C"
import (
	"os"
	"path"

	"github.com/linuxdeepin/go-gir/glib-2.0"
	"github.com/linuxdeepin/go-lib/utils"
)

const (
	_DirDefaultPerm os.FileMode = 0755
)

// ConfigFilePath returns path in user's config dir.
func ConfigFilePath(name string) string {
	return path.Join(glib.GetUserConfigDir(), name)
}

// ConfigFile open the given keyfile, this file will be created if not existed.
func ConfigFile(name string) (*glib.KeyFile, error) {
	file := glib.NewKeyFile()
	conf := ConfigFilePath(name)
	if !utils.IsFileExist(conf) {
		_ = os.MkdirAll(path.Dir(conf), _DirDefaultPerm)
		f, err := os.Create(conf)
		if err != nil {
			return nil, err
		}
		defer func() {
			_ = f.Close()
		}()
	}

	if ok, err := file.LoadFromFile(conf, glib.KeyFileFlagsNone); !ok {
		file.Free()
		return nil, err
	}
	return file, nil
}
