// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub

import (
	"bufio"
	"bytes"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"errors"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"
)

const (
	grubTimeout = "GRUB_TIMEOUT"
	grubDefault = "GRUB_DEFAULT"

	GrubParamsFile = "/etc/default/grub"
)

type GrubIfc struct {
	content map[string]string
	path    string
	locker  sync.RWMutex
}

func LoadGrubParams() (*GrubIfc, error) {
	var gbInfo GrubIfc
	gbInfo.content = make(map[string]string)
	gbInfo.path = GrubParamsFile
	fr, err := os.Open(filepath.Clean(GrubParamsFile))
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := fr.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		line = strings.TrimSpace(line)
		if (line == "") || (line[0] == '#') || !strings.Contains(line, "=") {
			continue
		}
		list := strings.SplitN(line, "=", 2)
		gbInfo.content[list[0]] = list[1]
	}
	return &gbInfo, nil
}

func (info *GrubIfc) TimeOut() (uint, error) {
	v := info.content[grubTimeout]
	if len(v) == 0 {
		return 0, errors.New("failed get grub time out")
	}
	t, err := strconv.Atoi(v)
	if err != nil {
		return 0, err
	}
	return uint(t), nil
}

func (info *GrubIfc) SetTimeOut(time uint) error {
	v := info.content[grubTimeout]
	if len(v) == 0 {
		return errors.New("failed set grub time out")
	}
	s := strconv.Itoa(int(time))
	info.content[grubTimeout] = s
	return info.Save()
}

func (info *GrubIfc) GrubDefault() (string, error) {
	v := info.content[grubDefault]
	if len(v) == 0 {
		return "", errors.New("failed get grub default")
	}
	return v, nil
}

func (info *GrubIfc) SetGrubDefault(def string) error {
	v := info.content[grubDefault]
	if len(v) == 0 {
		return errors.New("failed set grub default")
	}
	s := "\"" + def + "\""
	info.content[grubDefault] = s
	return info.Save()
}

func getGrubParamsContent(params map[string]string) []byte {
	keys := make(sort.StringSlice, 0, len(params))
	for k := range params {
		keys = append(keys, k)
	}
	keys.Sort()

	// write buf
	var buf bytes.Buffer

	for _, k := range keys {
		buf.WriteString(k + "=" + params[k] + "\n")
	}
	// if you want let the update-grub exit with error code,
	// uncomment the next line.
	//buf.WriteString("=\n")
	return buf.Bytes()
}

func (info *GrubIfc) Save() error {
	info.locker.RLock()
	defer info.locker.RUnlock()

	tmpFile := info.path + "-" + util.MakeRandomString(util.MinRandomLen)
	content := getGrubParamsContent(info.content)

	err := ioutil.WriteFile(tmpFile, content, 0644)
	if err != nil {
		return err
	}
	_, err = util.Move(info.path, tmpFile, true)
	return err
}
