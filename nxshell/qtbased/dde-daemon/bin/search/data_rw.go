// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"bytes"
	"encoding/gob"
	"io/ioutil"
	"os"

	dutils "github.com/linuxdeepin/go-lib/utils"
)

func writeDatasToFile(datas interface{}, filename string) {
	if datas == nil {
		logger.Warning("writeDatasToFile args error")
		return
	}

	var w bytes.Buffer
	enc := gob.NewEncoder(&w)
	if err := enc.Encode(datas); err != nil {
		logger.Warning("Gob Encode Datas Failed:", err)
		return
	}

	fp, err := os.Create(filename)
	if err != nil {
		logger.Warningf("failed to open %q: %v", filename, err)
		return
	}
	defer fp.Close()

	_, _ = fp.WriteString(w.String())
	_ = fp.Sync()
}

func readDatasFromFile(datas interface{}, filename string) bool {
	if !dutils.IsFileExist(filename) || datas == nil {
		logger.Warning("readDatasFromFile args error")
		return false
	}

	contents, err := ioutil.ReadFile(filename)
	if err != nil {
		logger.Warningf("failed to read file %q: %v", filename, err)
		return false
	}

	r := bytes.NewBuffer(contents)
	dec := gob.NewDecoder(r)
	if err = dec.Decode(datas); err != nil {
		logger.Warning("Decode Datas Failed:", err)
		return false
	}

	return true
}
