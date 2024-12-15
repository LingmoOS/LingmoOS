// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"crypto/md5"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
)

const uadpDataDir = uadpDir + "data/"
const uadpDataMap = uadpDir + "data.json"

type Data struct {
	AesKey []byte
	File   string
}

// data name => data file path
type ProcessData = map[string]Data

// process exe path => ProcessData
type ProcessMap = map[string]ProcessData

type DataManager struct {
	Data ProcessMap
}

func NewDataManager(dir string) *DataManager {
	_, err := os.Stat(dir)
	if os.IsNotExist(err) {
		err = os.MkdirAll(dir, 0700)
		if err != nil {
			logger.Warning(err)
		}
	}

	return &DataManager{
		Data: make(ProcessMap),
	}
}

func (dm *DataManager) Save(file string) error {
	data, err := json.MarshalIndent(dm, "", "    ")
	if err != nil {
		return err
	}

	_, err = os.Stat(filepath.Dir(file))
	if os.IsNotExist(err) {
		err = os.MkdirAll(filepath.Dir(file), 0700)
		if err != nil {
			return err
		}
	}

	err = os.WriteFile(file, data, 0600)
	if err != nil {
		return err
	}

	logger.Debugf("%s:\n%s", file, string(data))
	return nil
}

func (dm *DataManager) Load(file string) bool {
	data, err := os.ReadFile(file)
	if err != nil {
		logger.Debugf("%s not exist, create it", file)
		return false
	}

	err = json.Unmarshal(data, dm)
	if err != nil {
		logger.Debugf("%s unable to unmarshal, create it", file)
		return false
	}

	return true
}

func (dm *DataManager) ListName(process string) []string {
	nameList := []string{}
	for key, _ := range dm.Data[process] {
		nameList = append(nameList, key)
	}
	return nameList
}

func (dm *DataManager) SetData(dir string, process string, name string, aesKey []byte, data []byte) error {
	dm.makeProcessData(process)
	file := dm.Data[process][name].File
	if len(file) != 0 {
		os.Remove(file)
	}

	file = dir + md5str(data)
	err := os.WriteFile(file, data, 0600)
	if err != nil {
		return err
	}

	dm.Data[process][name] = Data{
		AesKey: aesKey,
		File:   file,
	}
	return nil
}

func (dm *DataManager) GetData(process string, name string) ([]byte, []byte, error) {
	dm.makeProcessData(process)
	aesKey := dm.Data[process][name].AesKey
	file := dm.Data[process][name].File
	if len(file) == 0 {
		return []byte{}, []byte{}, fmt.Errorf("'%s' not exist", name)
	}

	data, err := os.ReadFile(file)
	return aesKey, data, err
}

func (dm *DataManager) DeleteData(process string, name string) {
	dm.makeProcessData(process)
	file := dm.Data[process][name].File
	if len(file) != 0 {
		os.Remove(file)
	}

	delete(dm.Data[process], name)
}

func (dm *DataManager) DeleteProcess(process string) {
	dm.makeProcessData(process)
	for _, data := range dm.Data[process] {
		if len(data.File) != 0 {
			os.Remove(data.File)
		}
	}

	delete(dm.Data, process)
}

func (dm *DataManager) makeProcessData(process string) {
	if _, ok := dm.Data[process]; !ok {
		dm.Data[process] = make(ProcessData)
	}
}

func md5str(data []byte) string {
	return fmt.Sprintf("%x", md5.Sum(data))
}
