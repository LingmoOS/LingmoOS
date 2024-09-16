// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package systeminfo

import (
	"bufio"
	"io"
	"os"
	"strings"

	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("deepin-system-upgrade/version")

func ParseConfigFieldValue(path, field string) (string, error) {
	file, err := os.Open(path)
	if err != nil {
		logger.Warning("failed to open system version file:", err)
		return "", err
	}
	defer file.Close()
	br := bufio.NewReader(file)
	var value string
	for {
		s, _, err := br.ReadLine()
		if err == io.EOF {
			break
		}
		if strings.Contains(string(s), field) {
			value = strings.Split(string(s), "=")[1]
			break

		}
	}
	return value, nil
}
