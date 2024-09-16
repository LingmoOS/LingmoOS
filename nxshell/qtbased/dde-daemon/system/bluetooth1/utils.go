// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"bytes"
	"encoding/json"
	"os"

	"github.com/linuxdeepin/go-lib/keyfile"
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

// readBoolFile 读取一个很小的文件，一般情况下内容只有 0 和 1。
func readBoolFile(filename string) (bool, error) {
	// #nosec G304
	f, err := os.Open(filename)
	if err != nil {
		return false, err
	}
	defer func() {
		_ = f.Close()
	}()
	var buf [4]byte
	n, err := f.Read(buf[:])
	if err != nil {
		return false, err
	}
	content := bytes.TrimSpace(buf[:n])
	// 文件内容应该是 0 或 1
	if bytes.Equal(content, []byte("0")) {
		return false, nil
	}
	return true, nil
}

const (
	bluetoothPrefixDir = "/var/lib/bluetooth"
	kfSectionGeneral   = "General"
	kfKeyTechnologies  = "SupportedTechnologies"
)

func doGetDeviceTechnologies(filename string) ([]string, error) {
	var kf = keyfile.NewKeyFile()
	err := kf.LoadFromFile(filename)
	if err != nil {
		return nil, err
	}
	return kf.GetStringList(kfSectionGeneral, kfKeyTechnologies)
}
