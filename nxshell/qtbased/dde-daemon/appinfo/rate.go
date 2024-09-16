// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package appinfo

import (
	"io/ioutil"
	"os"

	"github.com/linuxdeepin/go-gir/glib-2.0"
)

const (
	_RateRecordFile = "launcher/rate.ini"
	_RateRecordKey  = "rate"
)

// GetFrequencyRecordFile returns the file which records items' use frequency.
func GetFrequencyRecordFile() (*glib.KeyFile, error) {
	return ConfigFile(_RateRecordFile)
}

func GetFrequency(id string, f *glib.KeyFile) uint64 {
	rate, _ := f.GetUint64(id, _RateRecordKey)
	return rate
}

func SetFrequency(id string, freq uint64, f *glib.KeyFile) {
	f.SetUint64(id, _RateRecordKey, freq)
	_ = saveKeyFile(f, ConfigFilePath(_RateRecordFile))
}

// saveKeyFile saves key file.
func saveKeyFile(file *glib.KeyFile, path string) error {
	_, content, err := file.ToData()
	if err != nil {
		return err
	}

	stat, err := os.Lstat(path)
	if err != nil {
		return err
	}

	err = ioutil.WriteFile(path, []byte(content), stat.Mode())
	if err != nil {
		return err
	}
	return nil
}
