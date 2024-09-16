// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package signature

import (
	"deepin-upgrade-manager/pkg/module/signature/sha256"
	"fmt"
)

type Signature interface {
	Sign(data []byte) ([]byte, error)
	SignFile(filename string) ([]byte, error)
	Verify(data []byte, signed string) (bool, error)
	VerifyFile(filename, signed string) (bool, error)
}

const (
	AlgSHA256 = "sha256"
)

func NewSignature(alg string) (Signature, error) {
	switch alg {
	case AlgSHA256:
		return &sha256.SHA256{}, nil
	}
	return nil, fmt.Errorf("unknown algorithm: %s", alg)
}
