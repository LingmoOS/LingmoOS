// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package records

import (
	"deepin-upgrade-manager/pkg/module/util"
	"errors"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
)

const (
	SelfRecordResultPath = "/etc/deepin-upgrade-manager/result.records"
)

func ReadResult() (res int, cmd string, err error) {
	res = -1
	if !util.IsExists(SelfRecordResultPath) {
		err = errors.New("file isn't exist")
		return 
	}

	file, err := os.Open(SelfRecordResultPath)
	if err != nil {
		return 
	}
	defer func(){
		if closeErr := file.Close(); err != nil{
			err = closeErr
		}
	}()
	content, err := ioutil.ReadAll(file)
	if err != nil {
		return 
	}
	line := strings.Split(string(content), ",")
	if len(line) == 0 {
		err = errors.New("file isn't exist")
		return 
	}
	result, err := strconv.Atoi(line[0])
	if err != nil {
		return 
	}
	if len(line) == 2 {
		cmd = line[1]
	}
	if RecoredState(result) == _ROLLBACK_SUCCESSED {
		res = 1
	}
	if RecoredState(result) == _ROLLBACK_FAILED {
		res = 0
	}
	return 
}

func RemoveResult() bool {
	if util.IsExists(SelfRecordResultPath) {
		os.RemoveAll(SelfRecordResultPath)
		return true
	} else {
		return false
	}
}
