// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package version

import "testing"

func TestGetSystemVersionInfo(t *testing.T) {
	info := getSystemVersionInfo()
	t.Error(info)
}
