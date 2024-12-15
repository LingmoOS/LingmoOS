// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"encoding/json"
)

type Info struct {
	SubmissionTime string `json:"SubmissionTime"`
	SystemVersion  string `json:"SystemVersion"`
	SubmissionType int    `json:"SubmissionType"`
	UUID           string `json:"UUID"`
	Note           string `json:"Note"`
}

type RecoredState int

const (
	_COMMIT_SYSTEM_ RecoredState = iota
	_COMMIT_USER_
	_COMMIT_INSTALL_
)

func (info Info) Time() string {
	return info.SubmissionTime
}

func (info Info) IsIntall() bool {
	return info.SubmissionType == int(_COMMIT_INSTALL_)
}

func LoadSubject(subject string) (Info, error) {
	var info Info

	err := json.Unmarshal([]byte(subject), &info)
	if err != nil {
		return info, nil
	}
	return info, nil
}
