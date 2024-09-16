// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package extractor

import (
	"deepin-upgrade-manager/pkg/module/archive"
	"deepin-upgrade-manager/pkg/module/manifest"
	"deepin-upgrade-manager/pkg/module/signature"
	"io/ioutil"
	"path/filepath"
)

type Extractor struct {
	compressor archive.Compressor
	signer     signature.Signature

	cacheDir string
}

func NewExtractor(comp, alg, cacheDir string) (*Extractor, error) {
	compressor, err := archive.NewCompressor(comp)
	if err != nil {
		return nil, err
	}
	signer, err := signature.NewSignature(alg)
	if err != nil {
		return nil, err
	}
	return &Extractor{
		compressor: compressor,
		signer:     signer,
		cacheDir:   cacheDir,
	}, nil
}

func (e *Extractor) Extract(filename string) (string, error) {
	dataDir, err := e.doExtract(filename)
	if err != nil {
		return "", err
	}
	err = e.doVerify(dataDir)
	if err != nil {
		return "", err
	}
	return dataDir, nil
}

func (e *Extractor) doExtract(filename string) (string, error) {
	dataDir, err := ioutil.TempDir(e.cacheDir, "data-")
	if err != nil {
		return "", err
	}

	err = e.compressor.Extract(filename, dataDir)
	if err != nil {
		return "", err
	}
	return dataDir, nil
}

func (e *Extractor) doVerify(dataDir string) error {
	var info manifest.Manifest

	err := manifest.LoadFile(&info,
		filepath.Join(dataDir, manifest.OS_MANIFEST_FILE))
	if err != nil {
		return err
	}
	return info.Verify(dataDir, e.signer)
}
