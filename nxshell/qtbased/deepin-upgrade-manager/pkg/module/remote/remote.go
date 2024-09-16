// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package remote

import (
	"bytes"
	config "deepin-upgrade-manager/pkg/config/upgrader"
	"deepin-upgrade-manager/pkg/logger"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"path/filepath"
)

const (
	UPGRADE_MODE_FULL        = "full"
	UPGRADE_MODE_INCREMENTAL = "incremental"

	_DEFAULT_OFFSET = 0
	_DEFAULT_LIMIT  = 5
)

type VersionReq struct {
	Distribution string `json:"distribution"`
	Version      string `json:"version"`
	CreateAt     string `json:"create_at"`
	BuiltBy      string `json:"built_by"`
	Subject      string `json:"subject"`
	Annotation   string `json:"annotation"`
	Changelog    string `json:"changelog"`
}
type VersionReqList []*VersionReq

func (list VersionReqList) List() []string {
	var strv []string
	for _, v := range list {
		strv = append(strv, v.Version)
	}
	return strv
}

type UpgradeCheckReq struct {
	Distribution         string         `json:"distribution"`
	Total                int            `json:"total"`
	Offset               int            `json:"offset"`
	Limit                int            `json:"limit"`
	Version              string         `json:"version"`
	AvailableVersionList VersionReqList `json:"available_version_list"`
}

type UpgradeCreateReq struct {
	Distribution  string `json:"distribution"`
	Mode          string `json:"mode"`
	BaseVersion   string `json:"base_version"`
	TargetVersion string `json:"target_version"`
	UpgradeFile   string `json:"upgrade_file"`
	RetrySeconds  int    `json:"retry_seconds"`
}

type Request config.Server

func (req *UpgradeCreateReq) Filename(cacheDir string) string {
	switch req.Mode {
	case UPGRADE_MODE_FULL:
		return filepath.Join(cacheDir, req.TargetVersion+".tar.zst")
	case UPGRADE_MODE_INCREMENTAL:
		return filepath.Join(cacheDir, req.BaseVersion+"-"+req.TargetVersion+".tar.zst")
	}
	return ""
}

func IsModeValid(mode string) bool {
	if len(mode) == 0 {
		return false
	}
	switch mode {
	case UPGRADE_MODE_FULL, UPGRADE_MODE_INCREMENTAL:
		return true
	}
	return false
}

func (req *Request) UpgradeCheck(version string, offset, limit int) (*UpgradeCheckReq, error) {
	if offset < 0 {
		offset = _DEFAULT_OFFSET
	}
	if limit < 0 {
		limit = _DEFAULT_LIMIT
	}

	var info = UpgradeCheckReq{
		Distribution: req.Distribution,
		Version:      version,
		Offset:       offset,
		Limit:        limit,
	}
	data, status, err := sendRequest(http.MethodGet,
		req.Host+req.UpgradeRoute, &info)
	if err != nil {
		return nil, err
	}
	if status != http.StatusOK {
		return nil, fmt.Errorf("%s", string(data))
	}
	err = json.Unmarshal(data, &info)
	return &info, err
}

func (req *Request) UpgradeCreate(mode, baseVer, targetVer string) (*UpgradeCreateReq, error) {
	if !IsModeValid(mode) {
		return nil, fmt.Errorf("invalid mode: %q", mode)
	}

	if (mode == UPGRADE_MODE_INCREMENTAL && len(baseVer) == 0) || len(targetVer) == 0 {
		return nil, fmt.Errorf("invalid args: base(%q), target(%q)",
			baseVer, targetVer)
	}

	var info = UpgradeCreateReq{
		Distribution:  req.Distribution,
		Mode:          mode,
		BaseVersion:   baseVer,
		TargetVersion: targetVer,
	}
	data, status, err := sendRequest(http.MethodPost,
		req.Host+req.UpgradeRoute, &info)
	if err != nil {
		return nil, err
	}
	switch status {
	case http.StatusOK, http.StatusAccepted:
		err = json.Unmarshal(data, &info)
		if len(info.UpgradeFile) != 0 {
			// TODO(jouyouyun): file route
			info.UpgradeFile = req.Host + "/v0/file/" + info.UpgradeFile
		}
		return &info, err
	}
	return nil, fmt.Errorf("%s", string(data))
}

func (req *Request) VersionQuery(version string) (*VersionReq, error) {
	if len(version) == 0 {
		return nil, fmt.Errorf("invalid args: version(%q)", version)
	}

	var info = VersionReq{
		Distribution: req.Distribution,
		Version:      version,
	}
	data, status, err := sendRequest(http.MethodGet,
		req.Host+req.VersionRoute, &info)
	if err != nil {
		return nil, err
	}
	if status != http.StatusOK {
		return nil, fmt.Errorf("%s", string(data))
	}
	err = json.Unmarshal(data, &info)
	return &info, err
}

func sendRequest(method, url string, info interface{}) ([]byte, int, error) {
	data, err := json.Marshal(info)
	if err != nil {
		return nil, 0, err
	}

	req, err := http.NewRequest(method, url, bytes.NewReader(data))
	if err != nil {
		return nil, 0, err
	}

	// TODO(jouyouyun): authorization
	req.Header.Set("Content-Type", "application/json")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, 0, err
	}
	if res.Body != nil {
		data, err = ioutil.ReadAll(res.Body)
		if err := res.Body.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}

	return data, res.StatusCode, err
}
