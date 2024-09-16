// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package manifest

import (
	"deepin-upgrade-manager/pkg/module/signature"
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"time"
)

type Entry struct {
	Key  string `xml:"key"`
	Data string `xml:"data"`
}

type OSDesc struct {
	XMLName xml.Name `xml:"description"`
	Meta    struct {
		Version    string `xml:"version"`
		BuiltBy    string `xml:"builtBy"`
		CreateAt   string `xml:"createAt"`
		Subject    string `xml:"subject"`
		OSVersion  string `xml:"osVersion"`
		Annotation string `xml:"annotation"`
	} `xml:"meta"`
	Changelog struct {
		Changes []*Entry `xml:"subject"`
	} `xml:"changelog"`
}

type Manifest struct {
	XMLName xml.Name `xml:"manifest"`
	Meta    struct {
		Version     string `xml:"version"`
		BuiltBy     string `xml:"builtBy"`
		CreateAt    string `xml:"createAt"`
		SignVersion string `xml:"signVersion"`
	} `xml:"meta"`
	Upgrade struct {
		Mode          string `xml:"mode"`
		BaseVersion   string `xml:"base_version"`
		TargetVersion string `xml:"target_version"`
	} `xml:"upgrade"`
	Signature struct {
		Files []*Entry `xml:"file"`
	} `xml:"signature"`
}

const (
	OS_SQUASHFS_FILE = "os_data.squashfs"
	OS_DESC_FILE     = "os_desc.xml"
	OS_DIFF_FILE     = "os_file.diff"
	OS_MANIFEST_FILE = "manifest_sign.xml"
	OS_META_INF_DIR  = "META-INF"
)

func (info *Manifest) Sign(dir string, signer signature.Signature) error {
	for _, entry := range info.Signature.Files {
		filename := entry.Key
		if len(dir) != 0 {
			filename = filepath.Join(dir, entry.Key)
		}
		digest, err := signer.SignFile(filename)
		if err != nil {
			return err
		}
		entry.Key = filepath.Base(entry.Key)
		entry.Data = fmt.Sprintf("%x", digest)
	}
	return nil
}

func (info *Manifest) Verify(dir string, signer signature.Signature) error {
	for _, entry := range info.Signature.Files {
		filename := entry.Key
		if len(dir) != 0 {
			filename = filepath.Join(dir, entry.Key)
		}
		ok, err := signer.VerifyFile(filename, entry.Data)
		if err != nil {
			return err
		}
		if !ok {
			return fmt.Errorf("signature not match for '%s'", entry.Key)
		}
	}
	return nil
}

func LoadFile(info interface{}, filename string) error {
	content, err := ioutil.ReadFile(filepath.Clean(filename))
	if err != nil {
		return err
	}
	return xml.Unmarshal(content, info)
}

func GenerateManifest(mode, base, target string,
	fileList []string, signer signature.Signature) (*Manifest, error) {
	var info = Manifest{}
	info.Meta.Version = "1.0"
	info.Meta.BuiltBy = "deepin"
	info.Meta.CreateAt = time.Now().String()
	info.Meta.SignVersion = "1.0"
	info.Upgrade.Mode = mode
	info.Upgrade.BaseVersion = base
	info.Upgrade.TargetVersion = target

	info.Signature.Files = []*Entry{}
	for _, v := range fileList {
		info.Signature.Files = append(info.Signature.Files, &Entry{
			Key: v,
		})
	}

	err := info.Sign("", signer)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func Save(info interface{}, filename string) error {
	content, err := xml.Marshal(info)
	if err != nil {
		return err
	}
	err = ioutil.WriteFile(filename, content, 0600)
	if err != nil {
		return err
	}
	return nil
}
