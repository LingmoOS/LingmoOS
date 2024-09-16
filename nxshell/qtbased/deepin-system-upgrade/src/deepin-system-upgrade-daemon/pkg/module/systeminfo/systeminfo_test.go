// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package systeminfo

import (
	"fmt"
	"testing"
)

func TestParseConfigFieldValue(t *testing.T) {
	value, err := ParseConfigFieldValue("/etc/os-version", "EditionName[zh_CN]")
	if err != nil {
		t.Error(err)
	}
	fmt.Printf("%v\n", value)
}
