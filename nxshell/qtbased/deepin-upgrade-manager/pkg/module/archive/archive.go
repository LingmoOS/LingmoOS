// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package archive

import (
	"deepin-upgrade-manager/pkg/module/archive/zstd"
	"fmt"
)

type Compressor interface {
	Compress(files []string, filename string) error
	Extract(filename, dstDir string) error
}

const (
	CompZSTD = "zstd"
)

func NewCompressor(comp string) (Compressor, error) {
	switch comp {
	case CompZSTD:
		return &zstd.ZSTD{}, nil
	}
	return nil, fmt.Errorf("unknown compression: %s", comp)
}
