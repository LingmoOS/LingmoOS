// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mime

import (
	"encoding/json"
	"io/ioutil"
)

type defaultAppTable struct {
	Apps defaultAppInfos `json:"DefaultApps"`
}

type defaultAppInfo struct {
	AppId   []string `json:"AppId"`
	AppType string   `json:"AppType"`
	Types   []string `json:"SupportedType"`
}
type defaultAppInfos []*defaultAppInfo

func unmarshal(file string) (*defaultAppTable, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var table defaultAppTable
	err = json.Unmarshal(content, &table)
	if err != nil {
		return nil, err
	}

	return &table, nil
}

func toJSON(v interface{}) (string, error) {
	content, err := json.Marshal(v)
	if err != nil {
		return "", err
	}
	return string(content), nil
}
