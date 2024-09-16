// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"testing"

	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

func TestKey2Mode(t *testing.T) {
	mod, res := key2Mod("super_l")
	if mod != keysyms.ModMaskSuper || res != true {
		t.Fatalf("key2Mod on super_l failed")
	}

	mod, res = key2Mod("super_r")
	if mod != keysyms.ModMaskSuper || res != true {
		t.Fatalf("key2Mod on super_l failed")
	}

	mod, res = key2Mod("r")
	if mod != 0 || res != false {
		t.Fatalf("key2Mod on r failed")
	}
}

func BenchmarkKey2Mode(b *testing.B) {
	for i := 0; i < b.N; i++ {
		key2Mod("super_l")
	}
}
