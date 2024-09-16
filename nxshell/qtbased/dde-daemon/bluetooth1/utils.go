// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"encoding/json"
	"io/ioutil"
	"strconv"

	"github.com/linuxdeepin/go-lib/procfs"
)

func isStringInArray(str string, list []string) bool {
	for _, tmp := range list {
		if tmp == str {
			return true
		}
	}
	return false
}

func marshalJSON(v interface{}) (strJSON string) {
	byteJSON, err := json.Marshal(v)
	if err != nil {
		logger.Error(err)
		return
	}
	strJSON = string(byteJSON)
	return
}

// find process
func checkProcessExists(processName string) bool {
	files, err := ioutil.ReadDir("/proc")
	if err != nil {
		logger.Warningf("read proc failed,err:%v", err)
	}

	for _, f := range files {
		if !f.IsDir() {
			continue
		}

		pid, err := strconv.ParseUint(f.Name(), 10, 32)
		if err != nil {
			continue
		}

		process := procfs.Process(pid)
		executablePath, err := process.Exe()
		if err != nil {
			//fmt.Println(err)
			continue
		}
		//if !fullpath {
		//	executablePath = filepath.Base(executablePath)
		//}
		if executablePath == processName {
			return true
		}
	}

	return false
}
