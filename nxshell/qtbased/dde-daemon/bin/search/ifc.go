// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"path"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/pinyin"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) NewSearchWithStrList(list []string) (md5sum string, ok bool, busErr *dbus.Error) {
	m.service.DelayAutoQuit()
	var datas []dataInfo
	strs := ""

	for _, v := range list {
		strs += v + "+"
		pyList := pinyin.HansToPinyin(v)
		if len(pyList) == 1 && pyList[0] == v {
			info := dataInfo{v, v}
			datas = append(datas, info)
			continue
		}
		for _, k := range pyList {
			info := dataInfo{k, v}
			datas = append(datas, info)
		}
		info := dataInfo{v, v}
		datas = append(datas, info)
	}

	md5Str, ok1 := dutils.SumStrMd5(strs)
	if !ok1 {
		logger.Warning("Sum MD5 Failed")
		return "", false, nil
	}

	cachePath, ok1 := getCachePath()
	if !ok1 {
		logger.Warning("Get Cache Path Failed")
		return "", false, nil
	}

	filename := path.Join(cachePath, md5Str)
	m.writeStart = true
	m.writeEnd = make(chan bool)
	go func() {
		writeDatasToFile(&datas, filename)
		m.writeEnd <- true
		m.writeStart = false
	}()

	return md5Str, true, nil
}

func (m *Manager) NewSearchWithStrDict(dict map[string]string) (md5sum string, ok bool, busErr *dbus.Error) {
	m.service.DelayAutoQuit()

	var datas []dataInfo
	strs := ""

	for k, v := range dict {
		strs += k + "+"
		pyList := pinyin.HansToPinyin(v)
		if len(pyList) == 1 && pyList[0] == v {
			info := dataInfo{v, k}
			datas = append(datas, info)
			continue
		}

		for _, l := range pyList {
			info := dataInfo{l, k}
			datas = append(datas, info)
		}
		info := dataInfo{v, k}
		datas = append(datas, info)
	}

	md5Str, ok1 := dutils.SumStrMd5(strs)
	if !ok1 {
		logger.Warning("Sum MD5 Failed")
		return "", false, nil
	}

	cachePath, ok1 := getCachePath()
	if !ok1 {
		logger.Warning("Get Cache Path Failed")
		return "", false, nil
	}

	filename := path.Join(cachePath, md5Str)
	m.writeStart = true
	m.writeEnd = make(chan bool)
	go func() {
		writeDatasToFile(&datas, filename)
		m.writeEnd <- true
		m.writeStart = false
	}()

	return md5Str, true, nil
}

func (m *Manager) SearchString(str, md5sum string) (result []string, busErr *dbus.Error) {
	m.service.DelayAutoQuit()

	var list []string
	if len(str) < 1 || len(md5sum) < 1 {
		return list, nil
	}

	list = searchString(str, md5sum)
	for _, v := range list {
		if !strIsInList(v, result) {
			result = append(result, v)
		}
	}

	return result, nil
}

func (m *Manager) SearchStartWithString(str, md5sum string) (result []string, busErr *dbus.Error) {
	m.service.DelayAutoQuit()

	var list []string
	if len(str) < 1 || len(md5sum) < 1 {
		return list, nil
	}

	list = searchStartWithString(str, md5sum)
	for _, v := range list {
		if !strIsInList(v, result) {
			result = append(result, v)
		}
	}

	return result, nil
}

func strIsInList(str string, list []string) bool {
	for _, l := range list {
		if str == l {
			return true
		}
	}

	return false
}
