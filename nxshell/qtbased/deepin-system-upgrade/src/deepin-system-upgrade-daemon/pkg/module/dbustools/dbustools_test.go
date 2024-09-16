// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package dbustools

import (
	"fmt"
	"strconv"
	"testing"
)

func TestGetProperty(t *testing.T) {
	reply, err := GetProperty("com.deepin.license", "com.deepin.license.Info", "AuthorizationState", "/com/deepin/license/Info")
	if err != nil {
		t.Error(err)
	}
	fmt.Printf("%v\n", strconv.Itoa(int(reply.Value().(int32))))
}
