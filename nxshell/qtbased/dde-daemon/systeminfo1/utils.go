// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"fmt"
	"io/ioutil"
	"os/exec"
	"strconv"
	"strings"
)

const (
	memKeyTotal   = "MemTotal"
	memKeyDelim   = ":"
	lscpuKeyDelim = ":"
)

func getMemoryFromFile(file string) (uint64, error) {
	ret, err := parseInfoFile(file, memKeyDelim)
	if err != nil {
		return 0, err
	}

	value, ok := ret[memKeyTotal]
	if !ok {
		return 0, fmt.Errorf("Can not find the key '%s'", memKeyTotal)
	}

	cap, err := strconv.ParseUint(strings.Split(value, " ")[0], 10, 64)
	if err != nil {
		return 0, err
	}

	return cap * 1024, nil
}

// 执行命令：/usr/bin/getconf LONG_BIT 获取系统位数
func systemBit() string {
	output, err := exec.Command("/usr/bin/getconf", "LONG_BIT").Output()
	if err != nil {
		return "64"
	}

	v := strings.TrimRight(string(output), "\n")
	return v
}

func runLscpu() (map[string]string, error) {
	cmd := exec.Command("lscpu")
	cmd.Env = []string{"LC_ALL=C"}
	out, err := cmd.Output()
	if err != nil {
		return nil, err
	}

	lines := strings.Split(string(out), "\n")
	res := make(map[string]string, len(lines))
	for _, line := range lines {
		items := strings.SplitN(line, lscpuKeyDelim, 2)
		if len(items) != 2 {
			continue
		}

		res[items[0]] = strings.TrimSpace(items[1])
	}

	return res, nil
}

func parseInfoFile(file, delim string) (map[string]string, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var ret = make(map[string]string)
	lines := strings.Split(string(content), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}

		array := strings.Split(line, delim)
		if len(array) != 2 {
			continue
		}

		ret[strings.TrimSpace(array[0])] = strings.TrimSpace(array[1])
	}

	return ret, nil
}
