// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

var str = []string{"/bin/sh", "/bin/bash",
	"/bin/zsh", "/usr/bin/zsh",
	"/usr/bin/fish",
}

func TestIsStringInArray(t *testing.T) {
	ret := isStringInArray("testdata/shells", str)
	assert.False(t, ret)
	ret = isStringInArray("/bin/sh", str)
	assert.True(t, ret)
}

func TestMarshalJSON(t *testing.T) {
	str1 := make(map[string]string)
	str1["name"] = "uniontech"
	str1["addr"] = "wuhan"
	ret := marshalJSON(str1)
	assert.Equal(t, ret, `{"addr":"wuhan","name":"uniontech"}`)
}
