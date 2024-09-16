// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package zoneinfo

import (
	"fmt"
	"io/ioutil"
	"regexp"
	"strings"
	"sync"
)

type DSTInfo struct {
	// The timestamp of entering DST every year
	Enter int64
	// The timestamp of leaving DST every year
	Leave int64

	// The DST offset
	Offset int32
}

type ZoneInfo struct {
	// Timezone name, ex: "Asia/Shanghai"
	Name string
	// Timezone description, ex: "上海"
	Desc string

	// Timezone offset
	Offset int32

	DST DSTInfo
}

var (
	_zoneListMux sync.Mutex
	_zoneList    []string
	_zoneListMap map[string]struct{}

	// Error, invalid timezone
	ErrZoneInvalid = fmt.Errorf("Invalid time zone")

	defaultZoneTab = "/usr/share/zoneinfo/zone1970.tab"
	defaultZoneDir = "/usr/share/zoneinfo"
)

// Check timezone validity
func IsZoneValid(zone string) (ret bool, err error) {
	if len(zone) == 0 {
		ret = false
		return
	}

	_zoneListMux.Lock()
	defer _zoneListMux.Unlock()

	if _zoneList == nil {
		err = loadZoneListWithoutLock()
	}

	_, ret = _zoneListMap[zone]
	return
}

func loadZoneListWithoutLock() (err error) {

	_zoneList, err = getZoneListFromFile(defaultZoneTab)
	_zoneListMap = make(map[string]struct{})

	for _, zone := range _zoneList {
		_zoneListMap[zone] = struct{}{}
	}

	return
}

func GetAllZones() (ret []string, err error) {
	_zoneListMux.Lock()
	defer _zoneListMux.Unlock()
	if _zoneList == nil {
		err = loadZoneListWithoutLock()
	}
	return _zoneList, err
}

// Query timezone detail info by timezone
func GetZoneInfo(zone string) (*ZoneInfo, error) {
	ok, err := IsZoneValid(zone)
	if err != nil {
		return nil, err
	}
	if !ok {
		return nil, ErrZoneInvalid
	}

	info := newZoneInfo(zone)

	return info, nil
}

func getZoneListFromFile(file string) ([]string, error) {
	lines, err := getUncommentedZoneLines(file)
	if err != nil {
		return nil, err
	}

	var list []string
	for _, line := range lines {
		strv := strings.Split(line, "\t")
		list = append(list, strv[2])
	}

	return list, nil
}

// when error occurs, return nil,error
func getUncommentedZoneLines(file string) ([]string, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var (
		lines = strings.Split(string(content), "\n")
		match = regexp.MustCompile(`^#`)
		ret   []string
	)
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}

		if match.MatchString(line) {
			continue
		}

		ret = append(ret, line)
	}

	return ret, nil
}
