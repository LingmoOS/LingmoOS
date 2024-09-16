// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"encoding/gob"
	"os"
	"path/filepath"
	"sync"

	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

var (
	cacheLocker sync.Mutex
	cacheFile   = filepath.Join(basedir.GetUserCacheDir(),
		"deepin/dde-daemon/systeminfo.cache")
)

func doReadCache(file string) (*SystemInfo, error) {
	cacheLocker.Lock()
	defer cacheLocker.Unlock()
	fp, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer fp.Close()

	var info SystemInfo
	decoder := gob.NewDecoder(fp)
	err = decoder.Decode(&info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func doSaveCache(info *SystemInfo, file string) error {
	cacheLocker.Lock()
	defer cacheLocker.Unlock()
	err := os.MkdirAll(filepath.Dir(file), 0755)
	if err != nil {
		return err
	}

	fp, err := os.Create(file)
	if err != nil {
		return err
	}
	defer fp.Close()

	encoder := gob.NewEncoder(fp)
	err = encoder.Encode(info)
	if err != nil {
		return err
	}
	return fp.Sync()
}
