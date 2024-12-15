// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package dsync

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type Connection struct {
	Type     string `json:"type"`
	Filename string `json:"filename"`
	Contents []byte `json:"contents"`
}
type ConnectionList []*Connection

type NetworkData struct {
	Version     string         `json:"version"`
	Connections ConnectionList `json:"connections"`
}

const (
	ConnTypeWIFI = "wifi"
)

var (
	ErrConnUnsupportedType = errors.New("unsupported connection type")
)

func (datas ConnectionList) Len() int {
	return len(datas)
}

func (datas ConnectionList) Swap(i, j int) {
	datas[i], datas[j] = datas[j], datas[i]
}

func (datas ConnectionList) Less(i, j int) bool {
	if datas[i].Type < datas[j].Type {
		return true
	} else if datas[i].Type > datas[j].Type {
		return false
	}

	return datas[i].Filename < datas[j].Filename
}

func (datas ConnectionList) Exists(data *Connection) bool {
	for _, v := range datas {
		if v.Equal(data) {
			return true
		}
	}
	return false
}

func (datas ConnectionList) Check() error {
	for _, data := range datas {
		if err := data.Check(); err != nil {
			return err
		}
	}
	return nil
}

func (datas ConnectionList) Get(ty, filename string) *Connection {
	for _, data := range datas {
		if data.Type == ty && data.Filename == filename {
			return data
		}
	}
	return nil
}

func (datas ConnectionList) Diff(list ConnectionList) ConnectionList {
	var ret ConnectionList
	for _, v := range list {
		if datas.Get(v.Type, v.Filename) != nil {
			continue
		}
		ret = append(ret, v)
	}
	return ret
}

func (data *Connection) Equal(info *Connection) bool {
	return data.Type == info.Type &&
		string(data.Contents) == string(info.Contents)
}

func (data *Connection) Check() error {
	if len(data.Type) == 0 || len(data.Filename) == 0 ||
		len(data.Contents) == 0 {
		return errors.New("empty type/filename/contents")
	}
	if data.Type != ConnTypeWIFI {
		return ErrConnUnsupportedType
	}
	return nil
}

func (data *Connection) WriteFile(dir string) error {
	err := os.MkdirAll(dir, 0755)
	if err != nil {
		return err
	}
	filename := filepath.Join(dir, data.Filename)
	absPath, err := filepath.Abs(filename)
	if err != nil {
		return err
	}

	if !strings.HasPrefix(absPath, dir) {
		return fmt.Errorf("%s is not in %s", absPath, dir)
	}
	return os.WriteFile(absPath, data.Contents, 0600)
}

func (data *Connection) RemoveFile(dir string) error {
	filename := filepath.Join(dir, data.Filename)
	return os.Remove(filename)
}
