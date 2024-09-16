// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package calltrace

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/linuxdeepin/go-lib/strv"
)

func getMemoryUsage() (int64, error) {
	filename := fmt.Sprintf("/proc/%d/status", os.Getpid())
	return sumMemByFile(filename)
}

func sumMemByFile(filename string) (int64, error) {
	fr, err := os.Open(filename)
	if err != nil {
		return 0, err
	}
	defer fr.Close()

	var count = 0
	var memSize int64
	var scanner = bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if line == "" {
			continue
		}

		if !strings.Contains(line, "RssAnon:") &&
			!strings.Contains(line, "VmPTE:") &&
			!strings.Contains(line, "VmPMD:") {
			continue
		}

		v, err := getInterge(line)
		if err != nil {
			return 0, err
		}
		memSize += v

		count++
		if count == 3 {
			break
		}
	}

	return memSize, nil
}

func getInterge(line string) (int64, error) {
	list := strings.Split(line, " ")
	list = strv.Strv(list).FilterEmpty()
	if len(list) != 3 {
		return 0, fmt.Errorf("Bad format: %s", line)
	}
	return strconv.ParseInt(list[1], 10, 64)
}
